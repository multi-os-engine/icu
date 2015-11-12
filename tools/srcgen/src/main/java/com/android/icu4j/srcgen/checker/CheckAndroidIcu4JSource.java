/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.icu4j.srcgen.checker;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import com.google.currysrc.Main;
import com.google.currysrc.api.Rules;
import com.google.currysrc.api.input.InputFileGenerator;
import com.google.currysrc.api.output.NullOutputSourceFileGenerator;
import com.google.currysrc.api.output.OutputSourceFileGenerator;
import com.google.currysrc.api.process.Context;
import com.google.currysrc.api.process.JavadocUtils;
import com.google.currysrc.api.process.Reporter;
import com.google.currysrc.api.process.Rule;
import com.google.currysrc.api.process.Processor;
import com.google.currysrc.api.process.ast.BodyDeclarationLocators;
import com.google.currysrc.api.process.ast.TypeLocator;
import com.google.currysrc.processors.BaseModifyCommentScanner;

import com.android.icu4j.srcgen.Icu4jTransformRules;
import com.android.icu4j.srcgen.TranslateJcite;

import org.eclipse.jdt.core.dom.ASTNode;
import org.eclipse.jdt.core.dom.ASTVisitor;
import org.eclipse.jdt.core.dom.AbstractTypeDeclaration;
import org.eclipse.jdt.core.dom.AnnotationTypeDeclaration;
import org.eclipse.jdt.core.dom.AnnotationTypeMemberDeclaration;
import org.eclipse.jdt.core.dom.BodyDeclaration;
import org.eclipse.jdt.core.dom.Comment;
import org.eclipse.jdt.core.dom.CompilationUnit;
import org.eclipse.jdt.core.dom.EnumConstantDeclaration;
import org.eclipse.jdt.core.dom.EnumDeclaration;
import org.eclipse.jdt.core.dom.FieldDeclaration;
import org.eclipse.jdt.core.dom.Javadoc;
import org.eclipse.jdt.core.dom.MethodDeclaration;
import org.eclipse.jdt.core.dom.Modifier;
import org.eclipse.jdt.core.dom.TagElement;
import org.eclipse.jdt.core.dom.TypeDeclaration;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import static com.android.icu4j.srcgen.Icu4jTransformRules.createOptionalRule;

/**
 * Checks that the source in android_icu4j doesn't have obvious problems.
 */
public class CheckAndroidIcu4JSource {

  private static final boolean DEBUG = false;

  private CheckAndroidIcu4JSource() {
  }

  /**
   * Usage:
   * java com.android.icu4j.srcgen.CheckAndroidIcu4JSource {android_icu4j src directories}
   *   {report output file path}
   */
  public static void main(String[] args) throws Exception {
    if (args.length < 2) {
      throw new IllegalArgumentException("At least 2 argument required.");
    }

    Main main = new Main(DEBUG);

    // We assume we only need to look at ICU4J code for this for both passes.
    String[] inputDirs = new String[args.length - 1];
    System.arraycopy(args, 0, inputDirs, 0, inputDirs.length);
    InputFileGenerator inputFileGenerator = Icu4jTransformRules.createInputFileGenerator(inputDirs);

    // Pass 1: Establish the set of classes and members that are public in the Android API.
    System.out.println("Establishing Android public ICU4J API");
    RecordPublicApiRules recordPublicApiRulesRules = new RecordPublicApiRules(inputFileGenerator);
    main.execute(recordPublicApiRulesRules);
    List<String> publicMemberLocaterStrings = recordPublicApiRulesRules.publicMembers();
    System.out.println("Public API is:");
    for (String publicMemberLocaterString : publicMemberLocaterStrings) {
      System.out.println(publicMemberLocaterString);
    }

    // Pass 2: Check for issues.
    System.out.println("Checking for issues");
    Set<String> publicMembersSet = Sets.newHashSet(publicMemberLocaterStrings);
    File outputReportFile = new File(args[args.length - 1]);
    FileWriter out = new FileWriter(outputReportFile, false /* append */);
    try (BufferedWriter reportWriter = new BufferedWriter(out)) {
      reportWriter.append("Beginning of report:\n");
      CheckAndroidIcu4SourceRules reportRules =
          new CheckAndroidIcu4SourceRules(inputFileGenerator, publicMembersSet);
      main.execute(reportRules, reportWriter);
      reportWriter.append("End of report\n");
    }

    System.out.println("Report file: " + outputReportFile);
  }

  private static class RecordPublicApiRules implements Rules {

    private final InputFileGenerator inputFileGenerator;
    private final RecordPublicApi recordPublicApi;

    public RecordPublicApiRules(InputFileGenerator inputFileGenerator) {
      this.inputFileGenerator = inputFileGenerator;
      recordPublicApi = new RecordPublicApi();
    }

    @Override public InputFileGenerator getInputFileGenerator() {
      return inputFileGenerator;
    }

    @Override public List<Rule> getRuleList(File file) {
      return Collections.<Rule>singletonList(createOptionalRule(recordPublicApi));
    }

    @Override public OutputSourceFileGenerator getOutputSourceFileGenerator() {
      return NullOutputSourceFileGenerator.INSTANCE;
    }

    public List<String> publicMembers() {
      return recordPublicApi.publicMembers();
    }
  }

  private static class CheckAndroidIcu4SourceRules implements Rules {

    private final InputFileGenerator inputFileGenerator;

    private final List<Rule> rules;

    public CheckAndroidIcu4SourceRules(InputFileGenerator inputFileGenerator,
        Set<String> publicMembers) {
      this.inputFileGenerator = inputFileGenerator;
      // Add more as we find issues.
      rules = Lists.<Rule>newArrayList(
          createOptionalRule(new CheckForBrokenJciteTag(publicMembers))
          );
    }

    @Override
    public List<Rule> getRuleList(File file) {
      return rules;
    }

    @Override
    public InputFileGenerator getInputFileGenerator() {
      return inputFileGenerator;
    }

    @Override
    public OutputSourceFileGenerator getOutputSourceFileGenerator() {
      return NullOutputSourceFileGenerator.INSTANCE;
    }

    /**
     * Checks for escaped (rather than transformed) jcite tags and warns for any found in the
     * public APIs.
     */
    private static class CheckForBrokenJciteTag extends BaseModifyCommentScanner {

      private final Set<String> publicMembers;

      public CheckForBrokenJciteTag(Set<String> publicMembers) {
        this.publicMembers = publicMembers;
      }

      @Override
      protected String processComment(Reporter reporter, Comment commentNode, String commentText) {
        if (commentNode instanceof Javadoc
            && commentText.contains(TranslateJcite.ESCAPED_JCITE_TAG)) {
          BodyDeclaration declaration = BodyDeclarationLocators.findDeclarationNode(commentNode);
          if (JavadocUtils.isNormallyDocumented(declaration)) {
            List<String> locatorStrings = BodyDeclarationLocators.toLocatorStringForms(declaration);
            for (String locatorString : locatorStrings) {
              if (publicMembers.contains(locatorString)) {
                reporter.info(
                    commentNode, "Escaped JCITE tag found in public API docs:" + commentText);
              } else {
                reporter.info(
                    commentNode, "Escaped JCITE tag found in non-public API docs (this is fine)");
              }
            }
          }
        }
        return null;
      }
    }
  }

  /**
   * Generates a list of all public members.
   */
  private static class RecordPublicApi implements Processor {
    private final List<String> publicMembers = Lists.newArrayList();

    @Override public void process(Context context, CompilationUnit cu) {
      cu.accept(new ASTVisitor() {
        @Override public boolean visit(AnnotationTypeDeclaration node) {
          throw new AssertionError("Not supported");
        }

        @Override public boolean visit(AnnotationTypeMemberDeclaration node) {
          throw new AssertionError("Not supported");
        }

        @Override public boolean visit(EnumConstantDeclaration node) {
          return handleMemberDeclarationNode(node);
        }

        @Override public boolean visit(EnumDeclaration node) {
          return handleTypeDeclarationNode(node);
        }

        @Override public boolean visit(FieldDeclaration node) {
          return handleMemberDeclarationNode(node);
        }

        @Override public boolean visit(MethodDeclaration node) {
          return handleMemberDeclarationNode(node);
        }

        @Override public boolean visit(TypeDeclaration node) {
          return handleTypeDeclarationNode(node);
        }
      });
    }

    private boolean handleTypeDeclarationNode(AbstractTypeDeclaration node) {
      handleDeclarationNode(node);
      // Continue processing for nested types / methods.
      return true;
    }

    private boolean handleMemberDeclarationNode(BodyDeclaration node) {
      handleDeclarationNode(node);
      // Leaf declaration (i.e. a method, fields, enum constant).
      return false;
    }

    private void handleDeclarationNode(BodyDeclaration node) {
      if (isExplicitlyHidden(node)) {
        return;
      }

      AbstractTypeDeclaration typeDeclaration = TypeLocator.findTypeDeclarationNode(node);
      if (typeDeclaration == null) {
        // Not unusual: methods / fields defined on anonymous types are like this. The parent
        // is a constructor expression, not a declaration.
        return;
      }

      boolean isNonTypeDeclaration = typeDeclaration != node;
      if (isNonTypeDeclaration) {
        if (isExplicitlyHidden(node) || !isMemberPublicApiEligible(typeDeclaration, node)) {
          return;
        }
      }
      while (typeDeclaration != null) {
        if (isExplicitlyHidden(typeDeclaration) || !isTypePublicApiEligible(typeDeclaration)) {
          return;
        }
        typeDeclaration = TypeLocator.findEnclosingTypeDeclaration(typeDeclaration);
      }
      // The node is appropriately public and is not hidden.
      publicMembers.addAll(BodyDeclarationLocators.toLocatorStringForms(node));
    }

    private boolean isTypePublicApiEligible(AbstractTypeDeclaration typeDeclaration) {
      int typeModifiers = typeDeclaration.getModifiers();
      return ((typeModifiers & Modifier.PUBLIC) != 0);
    }

    public boolean isMemberPublicApiEligible(AbstractTypeDeclaration type, BodyDeclaration node) {
      if (node.getNodeType() == ASTNode.ENUM_DECLARATION
          || node.getNodeType() == ASTNode.TYPE_DECLARATION) {
        throw new AssertionError("Unsupported node type: " + node);
      }

      if (type instanceof TypeDeclaration) {
        TypeDeclaration typeDeclaration = (TypeDeclaration) type;
        // All methods are public on interfaces. Not sure if true on Java 8.
        if (typeDeclaration.isInterface()) {
          return true;
        }
      }
      int memberModifiers = node.getModifiers();
      return ((memberModifiers & (Modifier.PUBLIC | Modifier.PROTECTED)) != 0);
    }

    private boolean isExplicitlyHidden(BodyDeclaration node) {
      Javadoc javadoc = node.getJavadoc();
      if (javadoc == null) {
        return false;
      }
      final Boolean[] isHidden = new Boolean[] { false };
      javadoc.accept(new ASTVisitor(true /* visitDocNodes */) {
        @Override public boolean visit(TagElement node) {
          String tagName = node.getTagName();
          if (tagName == null) {
            return true;
          }
          if (tagName.equals("@hide")) {
            isHidden[0] = true;
            return false;
          }
          return true;
        }
      });
      return isHidden[0];
    }

    public List<String> publicMembers() {
      return publicMembers;
    }
  }
}

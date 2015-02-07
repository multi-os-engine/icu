#
# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

# User-supplied locale service providers (using the java.text.spi or
# java.util.spi mechanisms) are not supported in Android:
#
# http://developer.android.com/reference/java/util/Locale.html

icu4j_src_files := \
    $(filter-out main/classes/localespi/%, \
    $(call all-java-files-under,main/classes))

icu4j_test_src_files := \
    $(filter-out main/tests/localespi/%, \
    $(call all-java-files-under,main/tests))

# Not all src dirs contain resources, some instead contain other random files
# that should not be included as resources. The ones that should be included
# can be identifed by the fact that they contain particular subdir trees.

define all-subdir-with-subdir
$(patsubst $(LOCAL_PATH)/%/$(2),%,$(wildcard $(LOCAL_PATH)/$(1)/$(2)))
endef

icu4j_resource_dirs := \
    $(filter-out main/classes/localespi/%, \
    $(call all-subdir-with-subdir,main/classes/*/src,com/ibm/icu))

icu4j_test_resource_dirs := \
    $(filter-out main/tests/localespi/%, \
    $(call all-subdir-with-subdir,main/tests/*/src,com/ibm/icu/dev))

# ICU4J depends on being able to use deprecated APIs and doing unchecked
# conversions, so these otherwise noisy lint warnings must be turned off.

icu4j_javac_flags := -encoding UTF-8 -Xlint:-deprecation,-unchecked
icu4j_test_javac_flags := $(icu4j_javac_flags)

# TODO: Replace use of ICU4J data JAR files with accessing ICU4C data (ie. by
# setting the com.ibm.icu.impl.ICUBinary.dataPath property), after everything
# else works and ICU4C in Android has been updated to ICU 54.

# For each data *.jar file, define a corresponding icu4j-* target.
icu4j_data_jars := \
    $(shell find $(LOCAL_PATH)/main/shared/data -name "*.jar" \
    | sed "s,^$(LOCAL_PATH)/\(.*/\(.*\)\.jar\)$$,icu4j-\2:\1,")

# A reimplementation in Bash of the @full-locale-names target rule from the
# main/shared/build/common-targets.xml Ant build file. This generates index
# files from the ICU4J data JAR and adds these index files to the library JAR.

ICUDATA_TMP := $(call intermediates-dir-for,JAVA_LIBRARIES,icudata,HOST,COMMON)
BUILD_FULL_LOCALE_NAMES := $(ICUDATA_TMP)/fullLocaleNames.mk
$(BUILD_FULL_LOCALE_NAMES): $(LOCAL_PATH)/main/shared/data/icudata.jar
	$(hide) mkdir -p $(dir $@)
	$(hide) \
	set -e ;\
	declare -A map ;\
	for line in $$( \
	    jar tf $< |\
	    grep -E '/icudt[^/]+/(.+/)?(.{2,3}(_.*)?|root)\.res$$' |\
	    grep -v '/res_index\.res$$' \
	) ; do \
	    file=$${line##*/} ; map[$${line%/*}]+=$$IFS$${file%%.res} ;\
	done ;\
	for dir in $${!map[@]} ; do \
	    mkdir -p $(ICUDATA_TMP)/$$dir ;\
	    for res in $${map[$$dir]} ; do \
	        echo $$res ;\
	    done |\
	    LC_COLLATE=C sort > $(ICUDATA_TMP)/$$dir/fullLocaleNames.lst ;\
	done ;\
	( \
	    echo '$$(LOCAL_INTERMEDIATE_TARGETS): $@' ;\
	    echo '$$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_EXTRA_JAR_ARGS+=\' ;\
	    for dir in $${!map[@]} ; do \
	        echo " -C \"$(ICUDATA_TMP)\" \"$$dir/fullLocaleNames.lst\"\\" ;\
	    done ;\
	    echo \
	) > $@

include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := $(icu4j_data_jars)
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_IS_HOST_MODULE := true
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := $(subst :,-host:,$(icu4j_data_jars))
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(icu4j_src_files)
LOCAL_JAVA_RESOURCE_DIRS := $(icu4j_resource_dirs)
LOCAL_STATIC_JAVA_LIBRARIES := icu4j-icudata icu4j-icutzdata
LOCAL_DONT_DELETE_JAR_DIRS := true
LOCAL_JAVACFLAGS := $(icu4j_javac_flags)
LOCAL_MODULE := icu4j
include $(BUILD_STATIC_JAVA_LIBRARY)
include $(BUILD_FULL_LOCALE_NAMES)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(icu4j_src_files)
LOCAL_JAVA_RESOURCE_DIRS := $(icu4j_resource_dirs)
LOCAL_STATIC_JAVA_LIBRARIES := icu4j-icudata-host icu4j-icutzdata-host
LOCAL_DONT_DELETE_JAR_DIRS := true
LOCAL_JAVACFLAGS := $(icu4j_javac_flags)
LOCAL_MODULE := icu4j-host
include $(BUILD_HOST_JAVA_LIBRARY)
include $(BUILD_FULL_LOCALE_NAMES)

ifeq ($(HOST_OS),linux)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(icu4j_src_files)
LOCAL_JAVA_RESOURCE_DIRS := $(icu4j_resource_dirs)
LOCAL_STATIC_JAVA_LIBRARIES := icu4j-icudata-host icu4j-icutzdata-host
LOCAL_DONT_DELETE_JAR_DIRS := true
LOCAL_JAVACFLAGS := $(icu4j_javac_flags)
LOCAL_MODULE := icu4j-hostdex
include $(BUILD_HOST_DALVIK_JAVA_LIBRARY)
include $(BUILD_FULL_LOCALE_NAMES)
endif  # HOST_OS == linux

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(icu4j_test_src_files)
LOCAL_JAVA_RESOURCE_DIRS := $(icu4j_test_resource_dirs)
LOCAL_STATIC_JAVA_LIBRARIES := icu4j-testdata
LOCAL_DONT_DELETE_JAR_DIRS := true
LOCAL_JAVA_LIBRARIES := icu4j
LOCAL_JAVACFLAGS := $(icu4j_test_javac_flags)
LOCAL_MODULE := icu4j-tests
include $(BUILD_STATIC_JAVA_LIBRARY)

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_EXTRA_JAR_ARGS += \
    -C "$(LOCAL_PATH)/main/tests/core/src" \
    "com/ibm/icu/dev/test/serializable/data"

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(icu4j_test_src_files)
LOCAL_JAVA_RESOURCE_DIRS := $(icu4j_test_resource_dirs)
LOCAL_STATIC_JAVA_LIBRARIES := icu4j-testdata-host
LOCAL_DONT_DELETE_JAR_DIRS := true
LOCAL_JAVA_LIBRARIES := icu4j-host
LOCAL_JAVACFLAGS := $(icu4j_test_javac_flags)
LOCAL_MODULE := icu4j-tests-host
include $(BUILD_HOST_JAVA_LIBRARY)

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_EXTRA_JAR_ARGS += \
    -C "$(LOCAL_PATH)/main/tests/core/src" \
    "com/ibm/icu/dev/test/serializable/data"

ifeq ($(HOST_OS),linux)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(icu4j_test_src_files)
LOCAL_JAVA_RESOURCE_DIRS := $(icu4j_test_resource_dirs)
LOCAL_STATIC_JAVA_LIBRARIES := icu4j-testdata-host
LOCAL_DONT_DELETE_JAR_DIRS := true
LOCAL_JAVA_LIBRARIES := icu4j-hostdex
LOCAL_JAVACFLAGS := $(icu4j_test_javac_flags)
LOCAL_MODULE := icu4j-tests-hostdex
include $(BUILD_HOST_DALVIK_JAVA_LIBRARY)

$(LOCAL_INTERMEDIATE_TARGETS): PRIVATE_EXTRA_JAR_ARGS += \
    -C "$(LOCAL_PATH)/main/tests/core/src" \
    "com/ibm/icu/dev/test/serializable/data"

endif  # HOST_OS == linux

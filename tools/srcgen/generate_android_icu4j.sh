# Copyright (C) 2015 The Android Open Source Project
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

# A script for generating the source code of the subset of ICU used by Android in libcore.

if [[ -z ${ANDROID_BUILD_TOP} ]]; then
  echo ANDROID_BUILD_TOP not set
  return
fi

# Build the srcgen tools.
cd ${ANDROID_BUILD_TOP}/external/icu/tools/srcgen/currysrc
mm -j32
cd ${ANDROID_BUILD_TOP}/external/icu/tools/srcgen
mm -j32

ANDROID_ICU4J_DIR=${ANDROID_BUILD_TOP}/external/icu/android_icu4j

# Clean out previous generated code / resources.
DEST_SRC_DIR=${ANDROID_ICU4J_DIR}/src/main/java
rm -rf ${DEST_SRC_DIR}
mkdir -p ${DEST_SRC_DIR}

DEST_RESOURCE_DIR=${ANDROID_ICU4J_DIR}/resources
rm -rf ${DEST_RESOURCE_DIR}
mkdir -p ${DEST_RESOURCE_DIR}

# Generate the source code needed by Android.
CLASSPATH=${ANDROID_BUILD_TOP}/out/host/common/obj/JAVA_LIBRARIES/currysrc_intermediates/javalib.jar:${ANDROID_BUILD_TOP}/out/host/common/obj/JAVA_LIBRARIES/android_icu4j_srcgen_intermediates/javalib.jar
INPUT_DIRS="\
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/collate/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/core/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/regiondata/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/translit/src \
    "
java -cp ${CLASSPATH} com.android.icu4j.srcgen.Icu4jTransform ${INPUT_DIRS} ${DEST_SRC_DIR}

# Copy / transform the resources needed by the code.
for INPUT_DIR in ${INPUT_DIRS}; do
  RESOURCES=$(find ${INPUT_DIR} -type f | egrep -v '.*(\.java|\/package\.html|\/ICUConfig.properties)$')
  for RESOURCE in ${RESOURCES}; do
    SOURCE_DIR=$(dirname ${RESOURCE})
    RELATIVE_SOURCE_DIR=$(echo ${SOURCE_DIR} | sed "s,${INPUT_DIR}/,,")
    RELATIVE_DEST_DIR=$(echo ${RELATIVE_SOURCE_DIR} | sed "s,com/ibm/icu,android/icu,")
    DEST_DIR=${DEST_RESOURCE_DIR}/${RELATIVE_DEST_DIR}
    mkdir -p ${DEST_DIR}
    cp $RESOURCE ${DEST_DIR}
  done
done

# Create the ICUConfig.properties for Android.
mkdir -p ${ANDROID_ICU4J_DIR}/resources/android/icu
sed 's,com.ibm.icu,android.icu,' ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/core/src/com/ibm/icu/ICUConfig.properties > ${ANDROID_ICU4J_DIR}/resources/android/icu/ICUConfig.properties

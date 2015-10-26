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

# Generate the source code needed by Android.
CLASSPATH=${ANDROID_BUILD_TOP}/out/host/common/obj/JAVA_LIBRARIES/currysrc_intermediates/javalib.jar:${ANDROID_BUILD_TOP}/out/host/common/obj/JAVA_LIBRARIES/android_icu4j_srcgen_intermediates/javalib.jar
INPUT_DIRS="\
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/collate/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/core/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/currdata/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/langdata/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/regiondata/src \
    ${ANDROID_BUILD_TOP}/external/icu/icu4j/main/classes/translit/src \
    "
java -agentlib:jdwp=transport=dt_socket,server=y,suspend=y,address=8100 -cp ${CLASSPATH} com.android.icu4j.srcgen.CaptureDeprecatedElements ${INPUT_DIRS}

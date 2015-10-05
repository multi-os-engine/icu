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

LOCAL_PATH := $(call my-dir)

# Create a list of prebuilt jars relative to $(LOCAL_PATH). i.e. lib/xxx.jar
prebuilt_jar_paths := $(shell find $(LOCAL_PATH)/libs -name "*.jar" | grep -v ".source_" | sed "s,^$(LOCAL_PATH)/,,")

# Create the list of target names each prebuilt will map to. i.e. currysrc-prebuilt-xxx
# prebuilt_target_names := $(shell $(find_prebuilts_command) | sed "s,^libs/\(.*\)\.jar$$,currysrc-prebuilt-\1,")
prebuilt_target_names := \
    currysrc-prebuilt-org.eclipse.text_3.5.400.v20150505-1044 \
    currysrc-prebuilt-org.eclipse.jdt.core_3.11.0.v20150602-1242 \
    currysrc-prebuilt-org.eclipse.osgi_3.10.100.v20150529-1857 \
    currysrc-prebuilt-org.eclipse.core.runtime_3.11.0.v20150405-1723 \
    currysrc-prebuilt-org.eclipse.core.contenttype_3.5.0.v20150421-2214 \
    currysrc-prebuilt-org.eclipse.equinox.common_3.7.0.v20150402-1709 \
    currysrc-prebuilt-org.eclipse.equinox.preferences_3.5.300.v20150408-1437 \
    currysrc-prebuilt-org.eclipse.core.resources_3.10.0.v20150423-0755 \
    currysrc-prebuilt-org.eclipse.core.jobs_3.7.0.v20150330-2103

# For each data *.jar file, define a corresponding currysrc-prebuilt-* target.
#prebuilt_jar_mapping := \
#    $(shell echo $(prebuilt_jar_paths) \
#    | sed "s,^\(libs/\(.*\)\.jar\)$$,currysrc-prebuilt-\2:\1,")
prebuilt_jar_mapping := \
    currysrc-prebuilt-org.eclipse.text_3.5.400.v20150505-1044:libs/org.eclipse.text_3.5.400.v20150505-1044.jar \
    currysrc-prebuilt-org.eclipse.jdt.core_3.11.0.v20150602-1242:libs/org.eclipse.jdt.core_3.11.0.v20150602-1242.jar \
    currysrc-prebuilt-org.eclipse.osgi_3.10.100.v20150529-1857:libs/org.eclipse.osgi_3.10.100.v20150529-1857.jar \
    currysrc-prebuilt-org.eclipse.core.runtime_3.11.0.v20150405-1723:libs/org.eclipse.core.runtime_3.11.0.v20150405-1723.jar \
    currysrc-prebuilt-org.eclipse.core.contenttype_3.5.0.v20150421-2214:libs/org.eclipse.core.contenttype_3.5.0.v20150421-2214.jar \
    currysrc-prebuilt-org.eclipse.equinox.common_3.7.0.v20150402-1709:libs/org.eclipse.equinox.common_3.7.0.v20150402-1709.jar \
    currysrc-prebuilt-org.eclipse.equinox.preferences_3.5.300.v20150408-1437:libs/org.eclipse.equinox.preferences_3.5.300.v20150408-1437.jar \
    currysrc-prebuilt-org.eclipse.core.resources_3.10.0.v20150423-0755:libs/org.eclipse.core.resources_3.10.0.v20150423-0755.jar \
    currysrc-prebuilt-org.eclipse.core.jobs_3.7.0.v20150330-2103:libs/org.eclipse.core.jobs_3.7.0.v20150330-2103.jar

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := $(prebuilt_jar_mapping)
include $(BUILD_HOST_PREBUILT)

# build currysrc jar
# ============================================================

include $(CLEAR_VARS)
LOCAL_IS_HOST_MODULE := true
LOCAL_MODULE := currysrc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_STATIC_JAVA_LIBRARIES := $(prebuilt_target_names) guavalib
LOCAL_SRC_FILES := $(call all-java-files-under, src/)
include $(BUILD_HOST_JAVA_LIBRARY)


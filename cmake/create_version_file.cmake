
# Copyright 2019 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

string(TIMESTAMP _date "%Y.%m.%d %H:%M:%S")
file(WRITE ${VERSION_FILE_NAME} "

#include \"version.h\"

/* generated on ${_date} */

#define GIT_REVISION \"${GIT_BRANCH}/${GIT_COMMIT_HASH} rev: ${GIT_COMMIT_COUNT}\"
#define TCAM_VERSION \"${TCAM_VERSION}\"
#define TCAM_ARAVIS_GIT_COMMIT_HASH \"${ARAVIS_GIT_COMMIT_HASH}\"
#define TCAM_ARAVIS_VERSION \"${ARAVIS_VERSION}\"

const char* get_commit_id ()
{
    return \"${GIT_BRANCH}/${GIT_COMMIT_HASH}_rev_${GIT_COMMIT_COUNT}\";
}

const char* get_version_number ()
{
    return \"${TCAM_VERSION}\";
}

const char* get_version_major ()
{
    return \"${TCAM_VERSION_MAJOR}\";
}

const char* get_version_minor ()
{
    return \"${TCAM_VERSION_MINOR}\";
}

const char* get_version_patch ()
{
    return \"${TCAM_VERSION_PATCH}\";
}

const char* get_version_modifier ()
{
    return \"${TCAM_VERSION_MODIFIER}\";
}

const char* get_version ()
{
    return \"${TCAM_VERSION}_${GIT_BRANCH}/${GIT_COMMIT_HASH}_rev_${GIT_COMMIT_COUNT}\";
}

const char* get_aravis_commit_id ()
{
    return \"${ARAVIS_GIT_COMMIT_HASH}\";
}

const char* get_aravis_version_number ()
{
    return \"${ARAVIS_VERSION}\";
}

const char* get_aravis_version ()
{
    return \"${ARAVIS_VERSION}_version_${ARAVIS_GIT_COMMIT_HASH}\";
}

const char* get_enabled_modules ()
{
    return \"${ENABLED_MODULES}\";
}

")

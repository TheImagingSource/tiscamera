
#ifndef TCAM_VERSION_H
#define TCAM_VERSION_H

#include "compiler_defines.h"

VISIBILITY_DEFAULT

// commit hash
const char* get_commit_id();

// x.y.z
const char* get_version_number();

// x
const char* get_version_major();

// y
const char* get_version_minor();

// z
const char* get_version_patch();

// version modifier like pre-release descriptions
// may return empty string
const char* get_version_modifier();

// complete version
// x.y.z_branch/<commit-hash>_rev_<commit-count>
const char* get_version();

const char* get_aravis_commit_id();

const char* get_aravis_version_number();

const char* get_aravis_version();

const char* get_enabled_modules();

VISIBILITY_POP

#endif /* TCAM_VERSION_H */

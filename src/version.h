
#ifndef TCAM_VERSION_H
#define TCAM_VERSION_H

// commit hash
const char* get_commit_id ();

// x.y.z
const char* get_version_number ();

// x
const char* get_version_major ();

// y
const char* get_version_minor ();

// z
const char* get_version_patch ();

// complete version
// x.y.z_branch/<commit-hash>_rev_<commit-count>
const char* get_version ();

const char* get_aravis_commit_id ();

const char* get_aravis_version_number ();

const char* get_aravis_version ();

#endif /* TCAM_VERSION_H */

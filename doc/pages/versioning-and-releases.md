# Versioning and Releases

## Versioning

The Tiscamera project follows semantic versioning as described here: [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

The version string you will encounter will look like this:

MAJOR.MINOR.PATCH(~additional_info)

Example:

`2.10.3~exposure-fixes(54d1gs)`

This can be read as:

- API is version 2
- 10 Features have been added since introduction of the API
- 3 bugs have been fixed since the last feature release
- ~\<description\> is an optional string that can be encountered on feature
  branches to further distinguish them from the master branch. They cannot be
  encountered on the master branch and any kind of official releases.
- (hash) is an optional git commit hash identifier

## Releases

Every release that is at least on the minor level will be tagged according to
the scheme:

v-tiscamera-MAJOR.MINOR.PATCH(~additional_info)

### Pre-Release steps

Steps that are taken for every release:

- Update changelog.txt
- Execute versioning scripts/versioning.sh
- Ensure a proper git tag is introduced

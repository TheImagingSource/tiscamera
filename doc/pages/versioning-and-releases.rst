
.. _versioning_and_release:

#######################
Versioning and Releases
#######################

==========
Versioning
==========

The Tiscamera project follows semantic versioning as described here: `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_.

The version string will look like this:

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
  
========
Releases
========

Every release that is at least on the minor level will be tagged according to
the scheme:

v-tiscamera-MAJOR.MINOR.PATCH(~additional_info)

Release Lifetime
================

Their dependencies are tailored to the current reference system.

see: https://ubuntu.com/about/release-cycle

Ubuntu 16.04 LTS will be supported until December 2019
Ubuntu 18.04 LTS will be supported until December 2021
Ubuntu 20.04 LTS will be supported until December 2023


Pre-Release steps
=================

Steps that are taken for every release:

- Update CHANGELOG.md
- Execute versioning `scripts/create-release`
- Ensure a proper git tag is introduced
- Ensure all unit tests run without error

Legacy Releases
===============


To checkout our previous release, follow these steps:

.. code-block:: sh

   git clone --recursive https://github.com/TheImagingSource/tiscamera

   cd tiscamera

   git checkout v-tiscamera-<RELEASE>

After this, simply follow the build and install instructions in the README.

All files required are included in the repository.

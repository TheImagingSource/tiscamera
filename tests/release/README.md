# Release Tests

## build-configurations.py

This script iterates all possible configurations and build them in separate directories.
Each directory contains a build.log. These can be manually checked.

If the console output does not contain any information about unusual cmake/make
return values everything can be considered ok.

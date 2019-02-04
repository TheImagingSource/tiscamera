# Tests {#tests}

Tests are divided in unit tests and integration tests.

The test coverage is currently incomplete. More tests will be added in the future.

## unit tests

Unit tests are implemented with the help of the catch2 framework.

To execute unit tests build the project and call `make test`.

If you want verbose output without calling a single test executable, set the
environment variable `CTEST_OUTPUT_ON_FAILURE=1`

## integration tests

Integration tests are tests that verify the proper interaction of multiple elements.
This e.g. includes the execution of complete gstreamer pipelines.

They are not executed automatically. 

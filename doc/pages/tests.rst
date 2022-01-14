.. _tests:

#####
Tests
#####

Tests are divided in unit tests and integration tests.

The test coverage is currently incomplete. More tests will be added in the future.

.. _unit-tests:

Unit Tests
==========

.. note::

   The GStreamer unit tests will require a camera for indexing.
   It is also advised to have the gige-daemon running to speed up test runs.

Unit tests are implemented with the help of the catch2 framework.

To execute unit tests, build the project and call `make test`.

To retrieve verbose output without calling a single test executable, set the
environment variable `CTEST_OUTPUT_ON_FAILURE=1`

.. code-block:: sh

   export CTEST_OUTPUT_ON_FAILURE=1

.. _integration-tests:

Integration Tests
=================

Integration tests are tests that verify the proper interaction of multiple elements.
This would, for example, include the execution of complete GStreamer pipelines.

They are not executed automatically.

Release Tests
=============

Release Tests are tests that are run to ensure no problems arise when deploying tiscamera.
This includes the building of different tiscamera configurations.
They are not executed automatically.

Manual Tests
============

The following tests are executed by a tester before publication of a new release.
The tests are executed on the current reference system.

For most tests, the following configuration is assumed:
``cmake -DTCAM_BUILD_ARAVIS=ON -DTCAM_BUILD_USB=ON -DTCAM_BUILD_LIBUSB=ON -DTCAM_BUILD_TESTS=ON -DTCAM_BUILD_TOOLS=ON ..``

- [ ] Building/Installation

  - [ ] The README instructions are clear and correct.
  - [ ] scripts/install-dependencies.sh installs all required dependencies for compilation/run time
  - [ ] `tests/release/build-configurations.py` runs without error.
  - [ ] ``make test`` executes without errors.
    Requires installation or sourcing of env.sh.
  - [ ] ``make package`` creates a Debian package that is installable and executable.
    See :ref:`package_testing`.

  - [ ] Sourcing env.sh sets all paths correctly

  - [ ] ``tcam-ctrl -l`` lists all devices.

    - [ ] ``tcam-capture`` can be started and works as expected.
    - [ ] ``camera-ip-conf -l`` works

  - [ ] installation works

    - [ ] ``sudo make install`` runs without warnings/error
    - [ ] ``sudo systemctl start gige-daemon.service``

- [ ] tcam-capture

  - [ ] Starting with ``--fullscreen`` opens in fullscreen mode
  - [ ] Starting with ``--serial`` opens the device with the given serial.
  - [ ] Starting with ``--format <caps string>`` opens the last device with the described format.
  - [ ] Pressing the fit-to-view button resizes the display area fit the current window size.
  - [ ] ROI display

    - [ ] ROIs can be adjusted via mouse
    - [ ] ROIs can be moved via mouse

  - [ ] hotkeys

    - [ ] trigger button works
      Active `trigger mode` and press the `trigger button` to receive a new image (default: spacebar).
    - [ ] reopen the device dialog
      Reopen the device dialog to select a new device (default: ctrl-o).
    - [ ] Pressing F11 or f toggles fullscreen mode

- [ ] evironment

  - [ ] GST_DEBUG output

    - [ ] Setting GST_DEBUG=tcam*:5 enables debug output.
    - [ ] Setting GST_DEBUG=0 disables any output log.

  - [ ] TCAM_GIGE_PACKET_SIZE works

    - | [ ] ``export TCAM_GIGE_PACKET_SIZE=XXXX``
      | use ``TCAM_LOG=DEBUG gst-launch-1.0 tcambin serial=<serial> ! videoconvert ! ximagesink``
      | to verify the size.

- [ ] GStreamer Elements

  - [ ] tcamsrc

    - [ ] tcamsrc property `drop-incomplete-frames=false` delivers incomplete frames.
      Testable by changing the mtu.
    - [ ] delivers correct GstMeta data

  - [ ] tcamautoexposure

    - [ ] auto exposure changes the exposure value

    - [ ] increasing auto exposure min increases the lowest possible exposure value the algorithm chooses.
      - [ ] auto exposure max is correctly limited by the used framerate.
      - [ ] lowering auto exposure max causes adjustments by the algorithms when high exposure values are set.

    - [ ] auto gain changes the gain value.

      - [ ] increasing auto gain min increases the lowest possible gain value the algorithm chooses.
      - [ ] lowering auto gain max causes adjustments by the algorithms when high gain values are set.

    - [ ] auto iris changes the iris value

      - [ ] iris is on maximum opening
        when exposure/gain adjustments are sufficient
      - [ ] iris closes when image becomes the bright
        and exposure/gain are already at their minim values.

- [ ] USB

  - [ ] extension units are correctly loaded when

    - [ ] USB 2
    - [ ] USB 23
    - [ ] USB 33/37

  - [ ] UDEV

    - [ ] extension units are correctly loaded when a camera is attached
    - [ ] libusb cameras like the afu050 can be opened


.. _package_testing:

Package Testing
---------------

The following steps are to be taken to ensure proper package integrity.
These steps should be executed on a vanilla reference system.

- [ ] ``sudo apt install tiscamera-*.deb`` installs the package without warnings etc.

- [ ] The gige-daemon is running.

  - [ ] The gige-daemon is running after a reboot.

- [ ] ``tcam-ctrl -l`` lists all expected devices and has no waiting period.

- [ ] ``tcam-ctrl -p <serial>`` lists all properties for a UVC camera (this verifies tcam-uvc-extension-loader).

- [ ] ``gst-launch-1.0 tcambin ! videoconvert ! ximagesink`` opens the first device and displays an image.

- [ ] ``tcam-capture`` correctly interacts with cameras

  - [ ] camera images and `The Imaging Source` icon are correctly displayed.

- [ ] The documenation can be opened and used. Default path: /usr/share/theimagingsource/tiscamera/documentation/index.html

- [ ] ``sudo apt remove tiscamera`` removes the package without warnings, etc.

####################################
The Imaging Source Gstreamer Plugins
####################################

The following plugins are offered by `The Imaging Source`.

.. _tcammainsrc:

tcammainsrc
###########

Source element that retrieves images from a device.  
The tcammainsrc existed as `tcamsrc` prior to tiscamera 0.13.0.  
It is used for v4l2, aravis and libusb devices.

Always use tcamsrc. tcammainsrc is considered an internal element.

.. list-table:: tcammainsrc properties
   :header-rows: 1
   :widths: 15 10 55 10 10

   * - fieldname
     - type
     - description
     - set available
     - get available
   * - serial
     - string
     - Serial number of the device that shall be used
     - `GST_STATE_NULL`
     - always
   * - type
     - string
     - Backend the camera shall use. Available options: v4l2, aravis, libusb, unknown
     - `GST_STATE_NULL`
     - always
   * - camera-buffers
     - int
     - Number of internal buffers the backend can use.
     - `< GST_STATE_PAUSED`
     - always
   * - num-buffers
     - int
     - Only send the specified number of images.
       With 'num-buffers=200' tcamsrc will automatically send an EOS and stop the device after 200 buffers have been sent to the pipeline.
     - `< GST_STATE_PAUSED`
     - always
   * - drop-incomplete-buffer
     - bool
     - When incomplete buffers should be delivered this has to be set to `false`.
     - always
     - always
   * - :ref:`tcam-properties<tcam-properties>`
     - GstStructure
     - Property that can be used to set/get the current TcamPropertyProvider properties.
       This can be used like: `gst-launch-1.0 tcammainsrc tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...`
     - always
     - always
   * - io-mode
     - integer
     - For a description of possible values, see :ref:`TcamMainSrc_io_mode`
     - `< GST_STATE_PAUSED`
     - always

.. _TcamMainSrc_io_mode:

.. list-table:: tcammainsrc io-mode
   :header-rows: 1

   * - int
     - name
     - desccription
   * - 0
     - auto
     - Automatically select the io-mode to use.
       Typically this will result in mmap for v4l2 and userptr for aravis/libusb.
   * - 1
     - mmap
     - Use memory allocated by the kernel driver
   * - 2
     - userptr
     - Use memory allocated in user space   
       
TcamMainSrc Signals
-------------------

tcammainsrc will emit the following signals:

.. list-table:: tcammainsrc signals
   :header-rows: 1

   * - signal
     - description
     - callback function
   * - device-open
     - Signal will be emitted when a device is opened.
       After this signal property interactions will be possible.
     - void user_function (GstElement* object, gpointer user_data);
   * - device-close
     - Signal will be emitted when the device is closed.
       All further device interactions through properties, etc will fail.
     - void user_function (GstElement* object, gpointer user_data);

       
.. _tcammainsrc_caps_auto_selection:
       
TcamMainsSrc Caps Auto Selection
--------------------------------

tcammainsrc will prefer certain GStreamer Caps over others when no explicit caps are given.

**format**:
    | prefer bayer 8-bit over everything else
    | if bayer 8-bit does not exist order according to the following list:
    | - color formats like BGR
    | - color formats like YUV
    | - formats like MJPEG
    | - GRAY16
    | - GRAY8
    | - pwl bayer
    | - bayer12/16
    | - polarized bayer
    | - polarized mono
**width**:
    | Will default to the highest value for the available range for the selected format.
    | If no format has been selected, one will be selected before a width is selected.
**height**:
    | Will default to the highest value for the available range for the selected format.
    | If no format has been selected, one will be selected before a height is selected.
**framerate**:
    | Will default to the highest framerate for the selected resolution.
    | If no resolution has been given it will be selected before a framerate is selected.
**binning**:
    Will default to '1x1' and omitted.
**skipping**:
    Will default to '1x1' and omitted.

MetaData
--------

Each image buffer the tcamsrc send has associated meta data that contains multiple information concerning the buffer.

The meta object contains a GstStructure which contains all information. This is to ensure extensibility without interfering with user applications.

The following fields are available:
                        
.. list-table:: GstTcamMeta fields
   :header-rows: 1
   :widths: 20 10 70
                                                               
   * - fieldname
     - type
     - description
   * - frame_count
     - uint64
     - number of frames delivered. Starts at 0 with every stream start.
   * - frames_dropped
     - uint64
     - number of frames dropped by backend
   * - capture_time_ns
     - uint64
     - Timestamp in Nanoseconds when the backend received the image
   * - camera_time_ns
     - uint64
     - Timestamp when the device itself captured the image. Only useful for GigE.
   * - is_damaged
     - bool
     - Flag noting if the buffer is damaged in any way. Only useful when drop-incomplete-buffer=false.
       
For timestamp point of reference values look :any:`timestamps`.
Please be aware that not all GStreamer elements correctly pass GstMeta information through.  
Elements like `bayer2rgb` to not copy the meta information.  
This may affect your usage of elements like `tcambin` as they can use such elements internally.

Messages
--------

The tcammainsrc element can send multiple possible messages to the GstBus.
It is generally recommended to listen for error messages as these will be considered lethal to the video stream and cause a stream stop.

Device lost
^^^^^^^^^^^

An error message containing the string "Device lost" will always be sent when the device does not respond or is not reachable.

The received message will be in the format "Device lost (<SERIAL>)".
For an example of message handling, see the example `09-device-lost`.

.. note:: The following requires GStreamer >= 1.10

To simplify error handling the tcamsrc sends an additional "Device lost" message
with a GstStructure attached. This structure contains the string field "serial".
This implies tiscamera was compiled with gstreamer >= 1.10.

.. code-block:: c

   /* This code only works when using gstreamer version 1.10 or higher */
   GstStructure* struc = gst_message_parse_error_details(message);
   const char* lost_serial = gst_structure_get_string(struc, "serial");

.. _tcampimipisrc:

tcampimipisrc
#############

Closed source GStreamer Source for FPD/MiPi Cameras on RaspberryPi.

You can find a Debian package `in our download section <https://www.theimagingsource.com/support/downloads-for-linux/>`__.

Further information can be found `in the online documentation <https://www.theimagingsource.com/documentation/tcampimipisrc/>`__.

.. _tcamtegrasrc:

tcamtegrasrc
############

Closed source GStreamer Source for FPD/MiPi Cameras on NVidia Jetson systems.

You can find a Debian package `in our download section <https://www.theimagingsource.com/support/downloads-for-linux/>`__.

Further information can be found `in the online documentation <https://www.theimagingsource.com/documentation/tcamtegrasrc/>`__.
                
.. _tcamsrc:
   
tcamsrc
#######

The tcamsrc is a source bin that allows access to all source elements supported by tiscamera.
It is a convenience wrapper and offers no additional properties.

| As of tiscamera 0.13.0 the supported source elements include tcammainsrc and tcampimipisrc.
| tiscamera 0.14.0 added support for tcamtegrasrc.
   

.. list-table:: TcamSrc properties
   :header-rows: 1
   :widths: 15 10 55 10 10

   * - fieldname
     - type
     - description
     - set available
     - get available

   * - serial
     - string
     - Serial number of the device that shall be used
     - `< GST_STATE_READY`
     - always
   * - type
     - string
     - Backend the camera shall use. Available options: v4l2, aravis, libusb, pimipi, unknown
     - `< GST_STATE_READY`
     - always
   * - :ref:`tcam-device<tcam-device>`
     - GstDevice
     - Assigns a GstDevice to open when transitioning from `GST_STATE_NULL` to `GST_STATE_READY`.
     - `< GST_STATE_READY`
     - never
   * - :ref:`tcam-properties<tcam-properties>`
     - GstStructure
     - Property that can be used to set/get the current TcamPropertyProvider properties. This can be used like: `gst-launch-1.0 tcambin tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...`
     - always
     - always
   * - :ref:`tcam-properties-json<tcam-properties-json>`
     - string
     - Property that can be used to set/get the current TcamPropertyProvider properties. This works the same way `tcam-properties` works, but uses a json string to provide the property names and values.
     - always
     - always
   * - camera-buffers
     - int
     - Number of internal buffers the backend can use. Forwarded to the actual device opened in `GST_STATE_READY`.
     - always
     - `>= GST_STATE_READY`
   * - num-buffers
     - int
     - Only send the specified number of images.
       With 'num-buffers=200' tcamsrc will automatically send an EOS and stop the device after 200 buffers have been sent to the pipeline.  Forwarded to the actual device opened in `GST_STATE_READY`.
     - always
     - `>= GST_STATE_READY`
   * - drop-incomplete-buffer
     - bool
     - When incomplete buffers should be delivered this has to be set to `false`. Forwarded to the actual device opened in `GST_STATE_READY`.
     - always
     - `>= GST_STATE_READY`
   * - do-timestamp 
     - bool
     - Sets the `do-timestamp` property. Forwarded to the actual device opened in `GST_STATE_READY`.
     - always
     - `>= GST_STATE_READY`

.. _tcamsrc_caps_auto_selection:
       
TcamSrc Caps Auto Selection
---------------------------

The caps auto selection is dependent on the internally used source element.

If your device uses the tcammainsrc (v4l2, aravis, libusb), see :ref:`here<tcammainsrc_caps_auto_selection>`.

.. _tcamdutils:

tcamdutils
##########

Closed source optional transformation and interpretation filter.
Allows the transformation of bayer 12-bit and 16-bit formats to BGRx 64-Bit.
Implements features like HDR.
For more information read `the documentation <https://www.theimagingsource.com/documentation/tiscameradutils/>`_

.. note::
   When using tiscamera-dutils with tcambin a version check is undertaken.
   tiscamera and tiscamera-dutils are version locked, meaning their major.minor version have to match.
   If a mismatch is detected, tcambin will disable the usage of the tcamdutils element and
   notify you with a GStreamer warning log message and a GstBus message.
   This can be overwritten by manually setting the tcambin property `conversion-element` to `tcamdutils`.

.. _tcamdutils_cuda:

tcamdutils-cuda
###############

Closed source optional transformation and interpretation filter.
Allows the transformation of bayer 12-bit and 16-bit formats to BGRx 64-Bit.
Optimized for NVidia Jetson platforms. Part of the package `tcamdutils-cuda`.

.. note::
   When using tcamdutils-cuda with tcambin a version check is undertaken.
   tiscamera and tcamdutils-cuda are version locked, meaning their major.minor version have to match.
   If a mismatch is detected, tcambin will disable the usage of the tcamdutils element and
   notify you with a GStreamer warning log message and a GstBus message.
   This can be overwritten by manually setting the tcambin property `conversion-element` to `tcamdutils-cuda`.
   
   
.. _tcambin:

tcambin
#######

Wrapper around all the previous elements, allowing for an easy all-in-one handling.
The tcambin will prefer bayer 8-bit over bayer 12/16-bit. Currently tcamdutils are required
for a correct conversion of these formats. Since tcamdutils are an optional module its existence
can not be expected. To ensure identical behavior whether or not tcamdutils are installed, bayer 8-bit will be preferred unless the user explicitly specifies bayer 12/16-bit for the source through the property 'device-caps'. The selected caps for the internal tcamscr will be propagated as a gstbus message with the prefix "Working with src caps: ".
The offered caps are the sum of unfiltered camera caps and caps that will be available through conversion elements like `bayer2rgb`.

The format that can always be expected to work is `BGRx`. All other formats depend on the used device.

.. note::
   When using tcamdutils or tcamdutils-cuda with tcambin a version check is undertaken.
   tiscamera and tcamdutils/tcamdutils-cuda are version locked, meaning their major.minor version have to match.
   If a mismatch is detected, tcambin will disable the usage of the tcamdutils/tcamdutils-cuda element and
   notify you with a GStreamer warning log message and a GstBus message.
   This can be overwritten by manually setting `conversion-element` to the concerning element name.

.. _tcambin_properties:
   
.. list-table:: TcamBin properties
   :header-rows: 1
   :widths: 15 10 55 10 10

   * - fieldname
     - type
     - description
     - set available
     - get available

   * - serial
     - string
     - Serial number of the device that shall be used
     - `GST_STATE_NULL`
     - always
   * - type
     - string
     - Backend the camera shall use. Available options: v4l2, aravis, libusb, pimipi, unknown
     - `GST_STATE_NULL`
     - always
   * - :ref:`tcam-device<tcam-device>`
     - GstDevice
     - Assigns a GstDevice to open when transitioning from `GST_STATE_NULL` to `GST_STATE_READY`.
     - `GST_STATE_NULL`
     - never
   * - available-caps
     - string
     - String description of the GstCaps that can be used in `device-caps`. Will be equal to or a subsection of the GstCaps offered by tcamsrc.
     - never
     - `>= GST_STATE_READY`
   * - device-caps
     - string
     - String that overwrites the auto-detection of the gstreamer caps that will be set for the internal tcamsrc
     - `< GST_STATE_PAUSED`
     - always
   * - :ref:`tcam-properties<tcam-properties>`
     - GstStructure
     - Property that can be used to set/get the current TcamPropertyProvider properties. This can be used like: `gst-launch-1.0 tcambin tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...`
     - always
     - always
   * - :ref:`tcam-properties-json<tcam-properties-json>`
     - string
     - Property that can be used to set/get the current TcamPropertyProvider properties. This works the same way `tcam-properties` works, but uses a json string to provide the property names and values.
     - always
     - always
   * - conversion-element
     - enum
     - Select the transformation element to use.
       Assuming all elements are available the selection is as follows:
       
       tcamdutils-cuda > tcamdutils > tcamconvert

       Both tcamdutils and tcamdutils-cuda are available as separate packages in our download section.
       This property has to be set while in state `GST_STATE_NULL`.
       
       Possible values: `auto`, `tcamconvert`, `tcamdutils`, `tcamdutils-cuda`
       Default: `auto`
     - `GST_STATE_NULL`
     - always

Internal pipelines will always be created when the element state is set to READY.

    tcamsrc -> capsfilter -> tcamconvert

    tcamsrc -> capsfilter -> tcamdutils
    
    tcamsrc -> capsfilter -> tcamdutils-cuda

    tcamsrc -> capsfilter -> jpegdec

    tcamsrc -> capsfilter


GObject properties
##################

.. _tcam-device:

GObject property `tcam-device`
--------------------------------------

This write-only property allows to open a specific device by passing a `GstDevice`.

`tcam-device` is only writeable in `GST_STATE_NULL`.

In the transition from `GST_STATE_NULL` to `GST_STATE_READY`, if this property was set, the tcamsrc calls `gst_device_create_element` with the assigned `GstDevice`.

If this property is not set, the default opening procedure uses `serial` and `type` to find a suitable device via `GstDeviceMonitor`.

E.g:

.. code-block:: cpp

    GstElement* src = ...;
    GstDevice* dev = fetch_first_device_from_monitor();

    g_object_set( G_OBJECT( src ), "tcam-device", dev );


.. _tcam-properties:

GObject property `tcam-properties`
--------------------------------------

In ``state == GST_STATE_NULL``:

* Set on `tcam-properties` copies the passed in structure. This structure gets applied to the device when transitioning to `GST_STATE_READY`.
* Get on `tcam-properties` returns either the previously passed in structure or if nothing was set, an empty structure.

.. note::  The property values set in `GST_STATE_NULL` are only used by the next state transition to `GST_STATE_READY` and are discarded after that.

In ``state >= GST_STATE_READY``:

* Set on `tcam-properties` applies the passed in GstStructure to the currently open device.
* Get on `tcam-properties` returns the property values of the currently open device.

One usage is using this to specify the startup properties of the device in a command line. 

E.g.:

.. code-block:: sh

    gst-launch-1.0 tcammainsrc tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...

Property names and types are the ones of the `TcamPropertyBase` objects exposed by the `TcamPropertyProvider` interface.

If a locked property is encountered it will by tried again after all other properties have been set.



.. _tcam-properties-json:

GObject property `tcam-properties-json`
-------------------------------------------------

In ``state ==  GST_STATE_NULL``:

* Set on `tcam-properties-json` copies the passed in string. This data gets applied to the device when transitioning to `GST_STATE_READY`.
* Get on `tcam-properties-json` returns either the previously passed in string or if nothing was set, an empty string.

.. note::  The property values set in `GST_STATE_NULL` are only used by the next state transition to `GST_STATE_READY` and are discarded after that.

In ``state >= GST_STATE_READY``:

* Set on `tcam-properties` applies the passed in json data to the currently open device.
* Get on `tcam-properties` returns the property values of the currently open device.

This can be used to get a json formatted snap shot of the current property values or to set saved property values in the device.

Property names and types are the ones of the `TcamPropertyBase` objects exposed by the `TcamPropertyProvider` interface.

If a locked property is encountered it will by tried again after all other properties have been set.

This example would dump the device property settings as a json string to the command line:

.. code-block:: sh

    tcam-ctrl --save <serial>

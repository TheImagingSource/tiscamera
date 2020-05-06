####################################
The Imaging Source Gstreamer Plugins
####################################

The following plugins are offered by us.
When an element property is also available through the property interface its name in said interface will be explicitly listed as "PI: property-name".

.. _tcamsrc:

tcamsrc
#######

Source elements that retrieves images from a device.

.. list-table:: tcamsrc properties
   :header-rows: 1
   :widths: 25 10 65

   * - fieldname
     - type
     - description
   * - serial
     - string
     - Serial number of the device that shall be used
   * - type
     - string
     - Backend the camera shall use. Available options: v4l2, aravis, libusb, unknown
   * - camera-buffers
     - int
     - Number of internal buffers the backend can use.
   * - num-buffers
     - int
     - Only send the specified number of images.
       With 'num-buffers=200' tcamsrc will automatically send an EOS and stop the device after 200 buffers have been sent to the pipeline.
   * - drop-incomplete-buffer
     - bool
     - When incomplete buffers should be delivered this has to be set to `false`.
   * - property-state
     - string
     - JSON string describing the state of all device properties. See :any:`state`.

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
   * - framerate
     - double
     - framerate the backend has.
   * - is_damaged
     - bool
     - Flag noting if the buffer is damaged in any way. Only useful when drop-incomplete-buffer=false.
       
For timestamp point of reference values look :any:`timestamps`.

Messages
--------

The tcamsrc element can send multiple possible messages to the GstBus.
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

tcamautoexposure
################

Automatically adjust exposure and gain to reach the wished image brightness.

.. list-table:: tcamautoexposure properties
   :header-rows: 1
   :widths: 20 50 10 20

   * - Name
     - Description
     - Default
     - Property Interface
   * - auto-exposure
     - Bool stating if the exposure value will be changed or not.
     - True
     - Exposure Auto
   * - auto-gain
     - Bool stating if the gain value will be changed or not.
     - True
     - Gain Auto
   * - auto-iris
     - Bool stating if the iris value will be changed or not.
     - True
     - Iris Auto
   * - brightness-reference
     - Image brightness that shall be considered ideal.
       The auto algorithm will try to get within a certain range of this value and adjust exposure/gain to remain within the range. The range is +/- 5 of the brightness reference value.
       Minimum: 0
       Maximum: 255
     - 128
     - Brightness Reference
   * - exposure-min
     - Minimum Exposure Value the element is allowed to set.
       If the user defined value is lower than the camera internal value the camera minimum will be used.
       If the user defined value is not a possible value it will be rejected.
     - Minimum of the camera
     - Exposure Auto Min
   * - exposure-max
     - Maximum Exposure Value the element is allowed to set.
       If the user defined value is higher than the camera internal maximum the camera maximum will be used.
       If the user defined value is higher than the value that is possible with the current framerate the value will be set. **This can cause problems.**
       Default: Highest value the framerate allows i.e. 30 fps => 1000000 / (30 / 1) = 33333.3
       This is a maximum exposure time of 33333 micro seconds.
     - Maximum of the camera / Exposure time the format allows
     - Exposure Auto Max
   * - gain-min
     - Minimum Gain Value the element is allowed to set.
       If the user defined value is lower than the camera internal value the camera minimum will be used.
       If the user defined value is not a possible value it will be rejected.
     - Minimum of the camera
     - Gain Auto Min
   * - gain-max
     - Maximum Gain Value the element is allowed to set.
     - Maximum of the camera
     - Gain Auto Max
   * - iris-min
     - Minimum iris value the element is allowed to set.
       Some cameras suggest a minimum value for auto algorithms that is higher than the actual minimal value. This value will be used when available.
     - Minimum value of the camera.
     - Iris Auto Min
   * - iris-max
     - Maximum iris value the element is to set.
     - Maximum of the camera.
     - Iris Auto Max
     
The following properties are related to the region of interest.
The region of interest is a section of the entire image that shall be used by the element for its auto algorithm.
Per default the region equals the entire image unless the user defines these values.

.. list-table:: tcamautoexposure roi properties
   :header-rows: 1
   :widths: 10 30 20 40
            
   * - Name
     - Values
     - TcamProp
     - Description
   * - left
     - Default: 0
     - Exposure ROI Left
     - X coordinate of the upper left corner. Values are in image pixel.
   * - top
     - Default: 0
     - Exposure ROI Top
     - Y coordinate of the upper left corner. Values are in image pixel.
   * - width
     - | Default: image width
       | Minimum: 8
       | Maximum: image width - exposure roi left
     - Exposure ROI Width
     - Width the ROI shall have.
   * - height
     - | Default: image height
       | Minimum: 8
       | Maximum: image height - exposure roi top
     - Exposure ROI Height
     - Height the ROI shall have.
       
tcamwhitebalance
################

Color correction for bayer images.

.. list-table:: GstTcamMeta fields
   :header-rows: 1
   :widths: 25 10 65

   * - fieldname
     - type
     - description
   * - red
     - int
     - Red Channel
   * - green
     - int
     - Green Channel
   * - blue
     - int
     - Blue Channel
   * - auto
     - bool
     - Bool stating if the module should automatically adjust the rgb values or if static values should be used to allow user defined whitebalance.
       _Default_: True
       PI: Whitebalance Auto
   * - module-enabled
     - bool
     - Bool stating if whitebalance values will be applied or note
       *Default* : True
       PI: Exposure Auto
   * - camera-whitebalance
     - bool
     - Bool stating if the whitebalance values shall be applied via software or in the device. Currently only the 72 USB cameras support this.
       Default: False
       PI: Camera Whitebalance
       
.. _tcamautofocus:
       
tcamautofocus
#############

Allows for cameras with focus elements to automatically adjust focus.

- auto - Activate an auto focus run by setting this property to true. It will be set to false once the run is finished.
  Default: False
  PI: Focus Auto

The following properties are related to the region of interest.
The region of interest is a section of the entire image that shall be used by the element for its
auto algorithm.
Per default the region equals the entire image unless the user defines these values.

.. list-table:: tcamautoexposure properties
   :header-rows: 1
   :widths: 10 30 20 40
   
   * - Name
     - Values
     - TcamProp
     - Description
   * - left
     - Default: 0
     - Focus ROI Left
     - X coordinate of the upper left corner. Values are in image pixel.
   * - top
     - Default: 0
     - Focus ROI Top
     - Y coordinate of the upper left corner. Values are in image pixel.
   * - width
     - | Default: image width
       | Minimum: 8
       | Maximum: image width - focus roi left
     - Focus ROI Width
     - Width the ROI shall have.
   * - height
     - | Default: image height
       | Minimum: 8
       | Maximum: image height - focus roi top
     - Focus ROI Height
     - Height the ROI shall have.


tcamdutils
##########

Closed source optional transformation and interpretation filter.
Allows the transformation of bayer 12-bit and 16-bit formats to BGRx 64-Bit.
Implements features like HDR.
Optimized for x64 platforms.

tcambiteater
############

Converts BGRx 64-bit to BGRx 32-Bit. Only required when using tcamdutils.

tcambin
#######

Wrapper around all the previous elements, allowing for an easy all-in-one handling.
The tcambin will prefer bayer 8-bit over bayer 12/16-bit. Currently tcamdutils are required
for a correct conversion of these formats. Since tcamdutils are an optional module its existence
can not be expected. To ensure identical behavior whether or not tcamdutils are installed, bayer 8-bit will be preferred unless the user explicitly specifies bayer 12/16-bit for the source through the property 'device-caps'. The selected caps for the internal tcamscr will be propagated as a gstbus message with the prefix "Working with src caps: ".
The offered caps are the sum of unfiltered camera caps and caps that will be available through conversion elements like `bayer2rgb`.

.. list-table:: TcamSrc properties
   :header-rows: 1
   :widths: 25 10 65

   * - fieldname
     - type
     - description
     
   * - serial
     - string
     - Serial number of the device that shall be used
   * - type
     - string
     - Backend the camera shall use. Available options: v4l2, aravis, libusb, unknown
   * - device-caps
     - string
     - String that overwrites the auto-detection of the gstreamer caps that will be set for the internal tcamsrc
   * - use-dutils
     - bool
     - Use the tcamdutils element, if present.
       Default: True
   * - property-state
     - string
     - JSON string describing the state of all device properties. See :any:`state`.
       
Internal pipelines will always be created when the element state is set to PAUSED.



    tcamsrc -> capsfilter -> tcamautoexposure -> tcamwhitebalance -> bayer2rgb

    tcamsrc -> capsfilter -> tcamdutils

    tcamsrc -> capsfilter -> jpegdec

    tcamsrc -> capsfilter

Should the selected camera offer focus properties the element :any:`tcamautofocus` will also be included.

Elements that offer auto algorithms (auto exposure/focus) will only be included when the camera itself does not offer these functions.

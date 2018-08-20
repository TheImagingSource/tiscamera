# Gstreamer {#gstreamer}

Gstreamer is a general purpose multimedia framework. It is the recommended way
to interact with your device.

All information concerning a plugin can be queried by executing 'gst-inspect-1.0 elementname'

## The Imaging Source Gstreamer Plugins

The following plugins are offered by us.
When an element property is also available through the property interface its name in said interface will be explicitly listed as "PI: property-name".

### tcamsrc

Source elements that retrieves images from you device.

- __serial__ - Serial number of the device you want to use
- __num-buffers__ - Only send the specified number of images.
                  With 'num-buffers=200' tcamsrc will automatically send an EOS and stop the device after 200 buffers have been sent to the pipeline.

### tcamautoexposure

Automatically adjust exposure and gain to reach the wished image brightness.

- __auto-exposure__ - Bool stating if the exposure value will be changed or not.  
 _Default_: True  
PI: Exposure Auto

- __auto-gain__ - Bool stating if the gain value will be changed or not.  
 _Default_: True  
PI: Gain Auto

- __brightness-reference__ - Image brightness that shall be considered ideal.   
The auto algorithm will try to get within a certain range of this value and adjust exposure/gain to remain within the range. The range is +/- 5 of the brightness reference value.  
 _Default_ : 128  
 _Minimum_ : 0  
 _Maximum_ : 255  
PI: Brightness Reference

- __exposure-min__ - Minimum Exposure Value the element is allowed to set.
If the user defined value is lower than the camera internal value the camera minimum will be used.
If the user defined value is not a possible value it will be rejected.  
 _Default_ : 0  
 PI: Exposure Auto Min

- __exposure-max__ - Maximum Exposure Value the element is allowed to set.
If the user defined value is higher than the camera internal maximum the camera maximum will be used.
If the user defined value is higher than the value that is possible with the current framerate the value will be set. __This can cause problems.__  
 _Default_: Highest value the framerate allows i.e. 30 fps => 1000000 / (30 / 1) = 33333.3  
            This is a maximum exposure time of 33333 micro seconds.  
PI: Exposure Auto Max

- __gain-min__ - Minimum Gain Value the element is allowed to set.  
If the user defined value is lower than the camera internal value the camera minimum will be used.  
If the user defined value is not a possible value it will be rejected.  
 _Default_: 0  
PI: Gain Auto Min

- __gain-max__ - Maximum Gain Value the element is allowed to set.

The following properties are related to the region of interest.
The region of interest is a section of the entire image that shall be used by the element for its
auto algorithm.
Per default the region equals the entire image unless the user defines these values.

- __left__ - X coordinate of the upper left corner. Values are in image pixel.
_Default_: 0
PI: Exposure ROI Left

- __top__ - Y coordinate of the upper left corner. Values are in image pixel.  
         _Default_: 0  
PI: Exposure ROI Top

- __width__ - Width the ROI shall have.  
    _Default_ : image width  
    _Minimum_ : 8  
    _Maximum_ : image width - exposure roi left  
PI: Exposure ROI Width

- __height__ - Height the ROI shall have.  
 _Default_ : image height  
 _Minimum_ : 8  
 _Maximum_ : image height - exposure roi top  
PI: Exposure ROI Height

### tcamwhitebalance

Color correction for bayer images.

- __red__ - Red Channel

- __green__ - Green Channel

- __blue__ - Blue Channel

- __auto__ - Bool stating if the module should automatically adjust the rgb values or if static values should be used to allow user defined whitebalance.  
 _Default_: True   
PI: Whitebalance Auto

- __module-enabled__ - Bool stating if whitebalance values will be applied or note  
 *Default* : True  
PI: Exposure Auto

- __camera-whitebalance__ - Bool stating if the whitebalance values shall be applied via software or in the device. Currently only the 72 usb cameras support this.  
 _Default_: False  
PI: Camera Whitebalance



### tcamautofocus

Allows for cameras with focus elements to automatically adjust focus.

- __auto__ - Activate an auto focus run by setting this property to true. It will be set to false once the run is finished.  
 _Default_: False  
PI: Focus Auto


The following properties are related to the region of interest.
The region of interest is a section of the entire image that shall be used by the element for its
auto algorithm.
Per default the region equals the entire image unless the user defines these values.

- __left__ - X coordinate of the upper left corner. Values are in image pixel.  
 _Default_: 0  
PI: Focus ROI Left

- __top__ - Y coordinate of the upper left corner. Values are in image pixel.  
 _Default_: 0  
PI: Focus ROI Top

- __width__ - Width the ROI shall have.  
 _Default_: image width  
 _Minimum_: 8  
 _Maximum_: image width - exposure roi left  
PI: Focus ROI Width

- __height__ - Height the ROI shall have.  
 _Default_: image height  
 _Minimum_: 8  
 _Maximum_: image height - exposure roi top  
PI: Focus ROI Height



### tcamdutils

Closed source optional transformation and interpretation filter.
Allows the transformation of bayer 12-bit and 16-bit formats to BGRx 64-Bit.
Implements features like HDR.
Optimized for x64 plattforms.

### tcambiteater

Converts BGRx 64-bit to BGRx 32-Bit. Only required when using tcamdutils.

#### tcambin

Wrapper around all the previous elements, allowing for an easy all-in-one handling.
The tcambin will prefer bayer 8-bit over bayer 12/16-bit. Currently tcamdutils are required
for a correct conversion of these formats. Since tcamdutils are an optional module its existence
can not be expected. To ensure identical behavior whether or not tcamdutils are installed, bayer 8-bit will be preferred unless the user explicitly specifies bayer 12/16-bit for the source through the property 'device-caps'. The selected caps for the internal tcamscr will be propagated as a gstbus message with the prefix "Working with src caps: ".

- __serial__ - Serial number of the device you want to use

- __device-caps__ - String that overwrites the auto-detection of the gstreamer caps that will be set for the internal tcamsrc

- __use-dutils__ - Use the tcamdutils element, if present.  
 _Default_: True
 
 

## Useful modules

#### clockoverlay

Allows overlays of timestamps or duration's.

#### capssetter

Capssetter allows you to overwrite existing caps or to add information to existing ones.

#### tee

Tee allows you to send a buffer to multiple modules at once.
A typical example would be to send a videostream to a window for live preview
and to additionally process it in the background.

After adding tee always use a queue to ensure that the following pipeline runs
in its own thread.

For further info please consult the
[gstreamer documentation](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/section-threads-uses.html).

#### valve

Let image buffer and bus messages pass only while the valve is open

#### videorate

Change the framerate by duplicating or dropping frames.

#### videoscale

Change the resolution of your image stream by scaling up or down.

#### appsink

Receive image buffer in you application and use them directly.
For examples how this may work, look at our examples folder.

## Debugging

For further info, see [logging](@ref logging).

## Environment

To add additional directories to the gstreamer search path you need to set
GST_PLUGIN_PATH_1_0.

    export GST_PLUGIN_PATH_1_0="/home/user/tiscamera/build/src/gstreamer-1.0/:${GST_PLUGIN_SYSTEM_PATH_1_0}"

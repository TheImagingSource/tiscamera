# tcam-capture {#tcam-capture}

tcam-capture is the general purpose image retrieval application of The Imaging Source under Linux.

![](tcam-capture-running.png)

## Optional Arguments

tcam-capture has several optional arguments to change its behavior:

    -h, --help         Show this help message and exit
    --serial SERIAL    Open device with serial immediately
    --format CAPS_STR  Open device with this gstreamer format
    --verbose, -v      Increase logging level
    --reset            Reset application settings and clear cache
    --fullscreen       Start the application in fullscreen mode

Additionally you can pass gstreamer arguments to retrieve debug information about the streams.
Currently supported are

    --gst-debug
    --gst-debug-level
    --gst-debug-no-color

For more information concerning gstreamer debugging go here: [logging](@ref logging)

## Options

tcam-capture offers several options to change its behavior.
The configuration file can be found under `$XDG_CONFIG_DIR/tcam-capture.conf`.
This is `~/.config/tcam-capture.conf`, unless changed.

### Image/Video

![](tcam-capture-options-saving.png)

#### Save Location

Default: /tmp

Folder in which images/videos shall be saved.

#### Image Type

Image encoding that shall be used when saving images.

_Default_: png

#### Video Type

_Default_: avi

Video encoding that shall be used when saving videos.

#### Naming Options

The available options are identical for images and videos.

-  __User Prefix__ - Random string defined by the user that is prepended to the
  file name. The maximum length is 100 characters

  _Default_: Empty
  
- __Include Serial__ - Adds the serial number of the used device to the filename

  _Default_: True

- __Include Format__ - Include a simple format description.
  This description contains all information you configured your device with.
  The string will have the format: format\_widthxheight\_framerate-numerator\_framerate-denominator
  To ensure the file can be saved characters like '/' are replaced with underscores.

  _Default_: True

- __Include Counter__ - Include a unique counter in the filename. If the
  application is restarted the counter will pickup where it left, assuming all
  other parts of the name remain identical.

  _Default_: True

- __Counter Size__ - Size of the padding the counter shall have

  _Maximum_: 10<br>
  _Default_: 5

- __Include Timestamp__ - Include a timestamp with your local time in the
  filename. The timestamp will be in ISO format i.e. YYYYmmddTHHMMSS.
  Please be aware that should both timestamp and counter be active your counter
  will be reset once the timestamp changes.

  _Default_: True


### General

![](tcam-capture-options-general.png)


#### Show Device Dialog On Startup

Whether or not to show the device selection dialog on startup.
Will be ignored when a device shall be reopened.

_Default_: True

#### Reopen Device On Startup

When a device was open during the last application shutdown, tcam-capture will
automatically try to reopen the device. If the device does not exist it will
fall back to its default behavior.

_Default_: True

#### Use Dutils

A toggle to disable the usage of tiscamera-dutils.
The package tiscamera-dutils will have to be installed for this to be enabled.

_Default_: True

## Caching

tcam-capture has a cache directory that you can find at
`$XDG_CACHE_DIR/tcam-capture/`.
This is `~/.cache/tcam-capture/` unless changed.

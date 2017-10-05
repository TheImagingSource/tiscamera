# GObject Introspection - TcamProp
@addtogroup gobject-introspection
To allow camera interaction from many different environments we offer a gobject
introspection interface. This interface, that can be accessed through our
gstreamer modules, allows full control of all device properties.

## Accessing the interface

To access the tcamprop interface two things are required:
- gstreamer-1.0
- gobject introspection

To see examples of how the interface is used take a look at our examples
directory.

After creating any tcam* gstreamer element you can use it as an TcamProp
instance.

Only tcamsrc and tcambin will give you device information.

Tcambin will internally fuse the information of its internal elements and return
the combined properties of these elements.

__!!! Caution !!!__

When changing the currently used format a reiteration of the properties is
advised. The tcambin will delete internal elements should the not be required
for the current format. This typically happens when switching between bayer and
other formats.

### Adding additional directories to the search path

To index additional directories set the environment variable GI_TYPELIB_PATH.

    export GI_TYPELIB_PATH=/home/user/tiscamera/build/src/gobject/


# tcam.so

The tcam library offers you a C++ and a C interface.
Both offer the same functionality.

Generally most classes you will interact with are wrappers around structs.
Thus many actions can be done by manipulating the underlying structs.

## C++

### DeviceIndex

#### Finding a device

To find a device you have to create a DeviceIndex.
The DeviceIndex object will keep an eye on available capture devices and
will allow you to be notified in the event, that a device is not reachable.

If you use a GigE camera it is advised, that you wait for a certain period to
give all network devices enough time to register with the DeviceIndex.

#+BEGIN_SRC C

std::shared_ptr<tcam::DeviceIndex> index = get_device_index();

sleep(3);

std::vector<DeviceInfo> device_list = device_index->get_device_list();

if (device_list.size() == 0)
{
std::cout << "No device found." << std::endl;
return 0;
}

for (const auto& d : device_list)
{
if (d.get_serial().compare("08311014") == 0)
{
// CaptureDevice dev(d);

}
}

#+END_SRC

An up-to-date device list will be maintained as long as the DeviceIndex
object exists.

#### Device lost

To register a callback function that will be informed in the case of a lost
device you will have to call 'DeviceIndex::register_device_lost'.

#+BEGIN_SRC CPP

void callback_function(const DeviceInfo& info)
{
std::cout << "Lost device: " << info->get_serial() << std::endl;
}

int main ()
{
auto index = get_device_index();
index->register_device_lost(callback_function);
}

#+END_SRC

### CaptureDevice

CaptureDevice is the main class of the tcam library. It enables all needed
interactions with your capture device (properties/format settings/streams).

#### Creating a CaptureDevice

To create a CaptureDevice you must have a valid DeviceInfo object. This
object can be manually generated or taken from a DeviceIndex.

Creation with DeviceIndex:
#+BEGIN_SRC C
std::shared_ptr<tcam::DeviceIndex> index = get_device_index();

sleep(3);

std::vector<DeviceInfo> device_list = device_index->get_device_list();

if (device_list.size() == 0)
{
std::cout << "No device found." << std::endl;
return 0;
}

for (const auto& d : device_list)
{
if (d.get_serial().compare("08311014") == 0)
{
// Use DeviceInfo to open a CaptureDevice
CaptureDevice dev(d);
}
}


#+END_SRC

Manual creation:
#+BEGIN_SRC C

struct tcam_device_info i;

// TODO what is required?
i.type = TCAM_DEVICE_TYPE_V4L2;
i.name =
i.identifier =
i.serial_number =

CaptureDevice dev(DeviceInfo(i));



#+END_SRC

#### Getting and setting properties

##### Iterating available properties

To iterate all properties that your device offers you have to call
"CaptureDevice::get_available_properties".

#+BEGIN_SRC C

CaptureDevice device = new CaptureDevice(info);

for (const Property# p : device->get_available_properties())
{
std::cout << std::left;
switch (p.get_type())
{
case TCAM_PROPERTY_TYPE_INTEGER:
{
PropertyInteger& i = (PropertyInteger&) p;
std::cout << std::setw(20) << i->get_name()
<< std::setw(10) << " (int)"<< std::right
<< "min=" << std::setw(5)<< i->get_min()
<< " max=" << std::setw(8) << i->get_max()
<< " step="<< std::setw(2)  << i->get_step()
<< " default=" << std::setw(5) << i->get_default()
<< " value=" << i->get_value()
<< std::endl;
break;
}
case TCAM_PROPERTY_TYPE_DOUBLE:
{
PropertyDouble& i = (PropertyDouble&) p;
std::cout << std::setw(20) << i->get_name()
<< std::setw(10) << " (double)"<< std::right
<< "min=" << std::setw(5)<< i->get_min()
<< " max=" << std::setw(8) << i->get_max()
<< " step="<< std::setw(2)  << i->get_step()
<< " default=" << std::setw(5) << i->get_default()
<< " value=" << i->get_value()
<< std::endl;
break;
}
case TCAM_PROPERTY_TYPE_STRING:
case TCAM_PROPERTY_ENUMERATION:
{
// TODO
}
case TCAM_PROPERTY_TYPE_BOOLEAN:
{
PropertyBoolean& s = (PropertyBoolean&) p;

std::cout << std::setw(20) << s->get_name()
<< std::setw(10) << "(bool)"
<< std::setw(31) << " "
<< "default="<< std::setw(5) << s->get_default()
<< "value=";
if (s->get_value())
{
std::cout << "true";
}
else
{
std::cout << "false";
}
std::cout << std::endl;
break;
}
case TCAM_PROPERTY_TYPE_BUTTON:
{
std::cout << std::setw(20) << p->get_name()
<< std::setw(10) << "(button)"
<< std::endl;
break;
}
case TCAM_PROPERTY_TYPE_UNKNOWN:
default:
{
std::cerr << "Unknown property type " << p->get_name() << std::endl;
}
}
}

#+END_SRC

##### Setting properties

To set a property you convert it to its correct type and call set_value.

#+BEGIN_SRC C

PropertyInteger& prop_i = (PropertyInteger&) p;

int value = 42;

bool ret = prop_i.set_value(value);

if (ret == false)
{
Error err = getError();
std::cout << "Unable to set value. Received error: " << err.get_string() << std::endl;
}

#+END_SRC


#### Getting and setting video formats



#### Receiving images

To receive an image you need to define a callback function that will be
called once for every image the device creates.

#+BEGIN_SRC C

bool continue_stream = true;
int max_image = 5;
int image_count = 0;

void new_image_received (const struct tcam_image_buffer# buffer, void# user_data)
{
if (image_count >= max_image)
{
continue_stream = false;
}
}

int main ()
{
auto sink = std::make_shared<ImageSink>();

sink.registerCallback(new_image_received, nullptr);

CaptureDevice device;

device.start_stream(sink)

while (continue_stream)
{
// wait for sign to end stream
}

device.stop_stream();

return 0;
}
#+END_SRC

### Logging

Per default the internal logging facility will only inform you of errors,
that is failures from which tcam cannot recover by itself.

These failures are sent to stderr by default.

If you wish to increase the log information or change the output to
a file or a user defined function you can do so by using one of the
following functions:
#+BEGIN_SRC C

tcam_set_logging_target (enum TCAM_LOG_TARGET target);

tcam_set_logging_file (const char# logfile);

const char# filename = tcam_get_logging_file ();

#+END_SRC

#### Changing the log level from the console

To change the log level of tcam for an already compilated program you can
set the environment variable TCAM_LOG.

#+BEGIN_SRC BASH

export TCAM_LOG=INFO

#+END_SRC

This will change the log level to the given setting. The available levels
are:
OFF, DEBUG, INFO, WARNING, ERROR

#### Using the logging facility in your application

To use the tcam logging facility call:

#+BEGIN_SRC C

tcam_log(TCAM_LOG_INFO, "FILENAME", 179, "Variable i has value: %d", i);

#+END_SRC

The parameter are log-level, filename or module, line number and the actual
message, which follows the printf format.

#### Redirecting logging to a user defined function

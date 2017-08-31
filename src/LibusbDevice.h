

#ifndef TCAM_LIBUSBDEVICE_H
#define TCAM_LIBUSBDEVICE_H

#include <memory>

#include "UsbSession.h"

namespace tcam
{

class LibusbDevice
{

    LibusbDevice (std::shared_ptr<tcam::UsbSession>);

private:

    std::shared_ptr<tcam::UsbSession> session;



}; // class LibusbDevice

} // namespace tcam

#endif /* TCAM_LIBUSBDEVICE_H */

#ifndef DEVICE_H
#define DEVICE_H


#include <gst/gst.h>
#include <string>

class Device
{
public:
    Device();
    Device(const std::string& model,
           const std::string& serial,
           const std::string& type,
           GstCaps* caps = nullptr);
    Device(const Device& other);

    Device& operator=(const Device& other);

    bool operator==(const Device& other) const;

    ~Device();

    std::string serial() const;
    std::string serial_long() const;
    std::string model() const;
    std::string type() const;

    std::string str() const;

    void set_caps(GstCaps* caps);
    GstCaps* caps() const;

private:
    std::string m_serial;
    std::string m_model;
    std::string m_type;

    GstCaps* p_caps;
};

#endif // DEVICE_H

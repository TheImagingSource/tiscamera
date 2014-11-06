
#ifndef CONTROLS_H
#define CONTROLS_H

#include <tcam.h>

#include <vector>
#include <string>


/**
 * @brief print function for properties
 * @param properties - Property collection that shall be printed
 */
void print_properties (const std::vector<Property>& properties);


/**
 * @brief Set property described in string to new value
 * @param g - CaptureDevice of the device that shall be used
 * @param new_prop - string describing the format
 * @return true on success
 */
bool set_property (CaptureDevice& g, const std::string& new_prop);

#endif /* CONTROLS_H */


#ifndef CONTROLS_H
#define CONTROLS_H

#include <tis.h>

#include <vector>
#include <string>

void print_properties (const std::vector<Property>& properties);

bool set_property (Grabber& g, const std::string& new_prop);

#endif /* CONTROLS_H */






#ifndef PROPERTYHANDLER_H
#define PROPERTYHANDLER_H

#include "Properties.h"

#include <vector>
#include <memory>

namespace tcam
{

class PropertyHandler : public PropertyImpl, public std::enable_shared_from_this<PropertyHandler>
{
public:
    PropertyHandler ();
    ~PropertyHandler ();

    bool set_properties (std::vector<std::shared_ptr<Property>> device_properties,
                         std::vector<std::shared_ptr<Property>> emulated_properties);


    std::vector<std::shared_ptr<Property>> get_properties ();

    /**
     * Synchronize all properties to hold up to date values
     */
    void sync ();

    /**
     * Delete all properties
     */
    void clear ();

    bool setProperty (const Property&);

    bool getProperty (Property&);

private:

    std::vector<std::shared_ptr<Property>> device_properties;
    std::vector<std::shared_ptr<Property>> emulated_properties;

    struct property_mapping
    {
        std::shared_ptr<Property> external_property;
        std::shared_ptr<Property> internal_property;
    };


    std::vector<std::shared_ptr<Property>> external_properties;
    std::vector<property_mapping> properties;

    property_mapping find_mapping_external (PROPERTY_ID id);
    property_mapping find_mapping_internal (PROPERTY_ID id);

    /**
     *
     */
    void generate_properties ();

};

} /* namespace tcam */

#endif /* PROPERTYHANDLER_H */

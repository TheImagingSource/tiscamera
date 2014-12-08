

#ifndef TCAM_QT4_H
#define TCAM_QT4_H

#include <tcam.h>

#include "VideoArea.h"
#include "DeviceSelectionDialog.h"

#include "propertywidget.h"
#include "propertydialog.h"
#include <QTreeWidget>
#include <memory>

namespace tcam
{

struct prop_category
{
    TCAM_PROPERTY_CATEGORY category;

    struct prop_group
    {
        TCAM_PROPERTY_ID group;
        Property master;
        std::vector<Property> props;
    };

    std::vector<prop_group> groups;
};


QTreeWidget* create_property_tree (QWidget* parent, std::vector<Property>);

typedef void (*tcam_device_selected)(tcam::DeviceInfo);

std::shared_ptr<DeviceSelectionDialog> create_device_selection_dialog (QObject* caller, tcam_device_selected);

VideoArea* create_video_area (QWidget* parent = 0 , tcam::DeviceInfo* info = 0);

} /* namespace tcam */

#endif /* TCAM_QT4_H */



#include "tcam_qt4.h"

#include "propertywidget.h"

#include <QTreeWidgetItem>
#include <QHeaderView>

#include <QObject> // connect

using namespace tcam;

static std::vector<prop_category> categorize (std::vector<Property> props)
{
    std::vector<prop_category> categories;

    auto add_to_category = [&categories] (const Property& p)
        {
            for (auto& c : categories)
            {
                if (c.category == p.getStruct().group.property_category)
                {
                    for (auto& g : c.groups)
                    {
                        if (g.group == p.getStruct().group.property_group)
                        {
                            if (p.getID() == p.getStruct().group.property_group)
                            {
                                g.master = p;
                            }
                            else
                            {
                                g.props.push_back(p);
                            }
                            return;
                        }
                    }
                    // create new group

                    prop_category::prop_group pg = {};
                    pg.group = p.getStruct().group.property_group;

                    if (p.getID() == p.getStruct().group.property_group)
                    {
                        pg.master = p;
                    }
                    else
                    {
                        pg.props.push_back(p);
                    }
                    c.groups.push_back(pg);

                    return;
                }
            }

            // create a new branch

            prop_category pc = {};
            pc.category = p.getStruct().group.property_category;

            prop_category::prop_group pg = {};

            pg.group = p.getStruct().group.property_group;

            // pg.props.push_back(p);


            if (p.getID() == p.getStruct().group.property_group)
            {
                pg.master = p;
            }
            else
            {
                pg.props.push_back(p);
            }

            pc.groups.push_back(pg);
            categories.push_back(pc);
        };

    // index properties
    for (const auto& p : props)
    {
        add_to_category(p);
    }
    return categories;
}


static QString getCategoryName (TCAM_PROPERTY_CATEGORY c)
{
    switch (c)
    {
        case TCAM_PROPERTY_CATEGORY_EXPOSURE:
            return "Exposure";
        case TCAM_PROPERTY_CATEGORY_COLOR:
            return "Color";
        case TCAM_PROPERTY_CATEGORY_LENS:
            return "Lens";
        case TCAM_PROPERTY_CATEGORY_SPECIAL:
            return "Special";
        case TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN:
            return "Partial Scan";
        case TCAM_PROPERTY_CATEGORY_IMAGE:
            return "Image";
        case TCAM_PROPERTY_CATEGORY_UNKNOWN:
        default:
            return "Unknown";
    }
}


QTreeWidget* tcam::create_property_tree (QWidget* parent, std::vector<Property> properties)
{
    auto categories = categorize (properties);

    // QTreeWidget* tree = new QTreeWidget(parent);
    QTreeWidget* tree = dynamic_cast<QTreeWidget*>(parent);

    // populate property tree
    tree->invisibleRootItem()->setFlags( Qt::ItemIsSelectable |
                                         Qt::ItemIsUserCheckable |
                                         Qt::ItemIsEnabled );

    tree->header()->close();

    for (auto& c : categories)
    {
        QTreeWidgetItem* master = new QTreeWidgetItem(tree);
        master->setText(0, getCategoryName(c.category));

        for (auto& g : c.groups)
        {
            QTreeWidgetItem* group_master = new QTreeWidgetItem(master);

            PropertyWidget* pw_master = new PropertyWidget(tree, &g.master);
            tree->setItemWidget(group_master, 0, pw_master);

            // connect(pw_master,
            //         SIGNAL(changed(PropertyWidget*)),
            //         this,
            //         SLOT(property_changed(PropertyWidget*)));


            for (auto& p : g.props)
            {
                PropertyWidget* pw = new PropertyWidget(tree, &p);

                // connect(pw,
                //         SIGNAL(changed(PropertyWidget*)),
                //         this,
                //         SLOT(property_changed(PropertyWidget*)));

                QTreeWidgetItem* item = new QTreeWidgetItem(group_master);
                tree->setItemWidget(item, 0, pw);
            }

            group_master->setExpanded(true);
        }
        master->setExpanded(true);
        tree->addTopLevelItem(master);
    }

    tree->setDragEnabled(false);


    return tree;
}


std::shared_ptr<DeviceSelectionDialog> tcam::create_device_selection_dialog (QObject* caller, tcam_device_selected callback)
{
    auto dialog = std::make_shared<DeviceSelectionDialog>();

    // QObject::connect(*dialog,
    //                  Q_SIGNAL(*dialog->device_selected),
    //                  *caller,
    //                  Q_SLOT(callback));


    return dialog;
}


VideoArea* tcam::create_video_area (QWidget* parent, tcam::DeviceInfo* info)
{
    return new VideoArea(parent, info);
}

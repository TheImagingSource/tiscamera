#include "propertydialog.h"
#include "ui_propertydialog.h"

#include "propertywidget.h"

#include <QListWidget>
#include <QTreeWidget>
#include <QList>

#include <tcam.h>
#include <vector>

using namespace tcam;

PropertyDialog::PropertyDialog (QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDialog)
{
    ui->setupUi(this);


}



PropertyDialog::PropertyDialog (QWidget *parent, CaptureDevice* dev) :
    QDialog(parent),
    ui(new Ui::PropertyDialog),
    device(dev)
{
    ui->setupUi(this);

    populate_dialog ();
}


PropertyDialog::~PropertyDialog ()
{
    delete ui;
}


static QString getCategoryName(TCAM_PROPERTY_CATEGORY c)
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


void PropertyDialog::populate_dialog ()
{
    if (device == nullptr)
    {
        return;
    }

    properties = device->get_available_properties();

    // tree structure for property ui creation

    struct prop_category
    {
        TCAM_PROPERTY_CATEGORY category;


        struct prop_group
        {
            TCAM_PROPERTY_ID group;
            std::vector<Property*> props;
        };

        std::vector<prop_group> groups;
    };

    std::vector<prop_category> categories;

    auto add_to_category = [&categories] (Property* p)
        {
            for (auto& c : categories)
            {
                if (c.category == p->get_struct().group.property_category)
                {
                    for (auto& g : c.groups)
                        if (g.group == p->get_struct().group.property_group)
                        {
                            g.props.push_back(p);
                            return;
                        }
                    // create new group

                    prop_category::prop_group pg = {};
                    pg.group = p->get_struct().group.property_group;

                    pg.props.push_back(p);
                    c.groups.push_back(pg);

                    return;
                }
            }

            // create a new branch

            prop_category pc = {};
            pc.category = p->get_struct().group.property_category;

            prop_category::prop_group pg = {};
            pg.group = p->get_struct().group.property_group;

            pg.props.push_back(p);
            pc.groups.push_back(pg);
            categories.push_back(pc);
        };

    // index properties
    for (auto p : properties)
    {
        add_to_category(p);
    }

    // create gui elements for indexed properties
    for (auto& c : categories)
    {
        QListWidget* list = new QListWidget();

        for (auto& g : c.groups)
        {
            // create/add tree that will be filled
            QTreeWidget* tree = new QTreeWidget();
            QListWidgetItem* item = new QListWidgetItem(list);
            list->addItem(item);
            // list->setItemWidget(item, tree);

            // fill tree with items
            // group master will be used as top level element

            QTreeWidgetItem* master = nullptr;
            QList<QTreeWidgetItem*> children;

            for (auto& p : g.props)
            {
                // QTreeWidgetItem* ti = new QTreeWidgetItem();

                // group master
                if (p->get_struct().group.property_group == g.group)
                {
                    master = new QTreeWidgetItem();;
                    tree->addTopLevelItem(master);
                    tree->setItemWidget(master, 0, new PropertyWidget(tree, p));
                }
            }

            tree->show();
            ui->categoryWidget->addTab(tree, getCategoryName(c.category));

            // for (auto& p : g.props)
            // {
            //     QTreeWidgetItem* ti = new QTreeWidgetItem();


            //     // group master
            //     if (p.get_struct().group.property_group == g.group)
            //     {
            //         master = ti;
            //         tree->addTopLevelItem(ti);
            //     }
            //     else
            //     {
            //         children.append(ti);
            //     }
            // }

            // if (master != nullptr)
            // {
            //     master->addChildren(children);
            // }
        }
        //ui->categoryWidget->addTab(list, getCategoryName(c.category));
    }
}

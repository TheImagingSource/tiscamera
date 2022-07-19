
#include "tcamcollection.h"

#include <QDebug>
#include <string>

TcamCollection::TcamCollection(GstBin* pipeline)
{
    for (int i = 0; i < 100; i++)
    {
        std::string name = "tcam";
        name += std::to_string(i);

        auto element = gst_bin_get_by_name(pipeline, name.c_str());
        if (!element)
        {
            break;
        }
        m_elements.push_back(element);
    }

    for (auto e : m_elements)
    {
        GError* err = nullptr;
        GSList* names =
            tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(e), &err);
        if (err)
        {
            qInfo("Error while retrieving property names: %s", err->message);
            g_error_free(err);
            continue;
        }

        for (GSList* n = names; n != nullptr; n = n->next)
        {
            m_prop_origin.push_back(entry { (char*)n->data, e });
        }
        g_slist_free_full(names, g_free);
    }
}

TcamCollection::~TcamCollection()
{
    for (auto&& elem : m_elements) { gst_object_unref(elem); }
}

std::vector<std::string> TcamCollection::get_names() const
{
    std::vector<std::string> ret;
    ret.reserve(m_prop_origin.size());
    for (const auto& o : m_prop_origin) { ret.push_back(o.prop_name); }
    return ret;
}

TcamPropertyBase* TcamCollection::get_property(const std::string& name)
{
    for (const auto& o : m_prop_origin)
    {
        if (o.prop_name == name)
        {
            GError* err = nullptr;
            auto prop = tcam_property_provider_get_tcam_property(
                TCAM_PROPERTY_PROVIDER(o.elem), name.c_str(), &err);
            if (err)
            {
                qWarning("Error while retrieving property \"%s\": %s", name.c_str(), err->message);
                g_error_free(err);
            }

            if (prop)
            {
                return prop;
            }
        }
    }
    return nullptr;
}


bool TcamCollection::is_trigger_mode_active()
{
    auto base = get_property("TriggerMode");

    if (!base)
    {
        // device does not have trigger mode
        // likely a FPD/MiPi camera
        return false;
    }

    auto tm = TCAM_PROPERTY_ENUMERATION(base);

    GError* err = nullptr;
    const char* value = tcam_property_enumeration_get_value(tm, &err);

    if (value)
    {
        if (strcmp(value, "On") == 0)
        {
            return true;
        }
        return false;
    }

    if (err)
    {
        qWarning("Querying TriggerMode caused an error: %s", err->message);
        g_error_free(err);
    }

    return false;
}

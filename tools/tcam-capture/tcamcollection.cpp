
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
        GSList* names = tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(e), &err);

        if (err)
        {
            qInfo("Error while retrieving property names: %s", err->message);
            g_error_free(err);
            continue;
        }

        for (GSList* n = names; n != nullptr; n = n->next)
        {
            m_prop_origin[(char*)n->data] = e;
        }
        g_slist_free_full(names, g_free);
    }

}


std::vector<std::string> TcamCollection::get_names() const
{
    std::vector<std::string> ret;
    ret.reserve(m_prop_origin.size());
    for (const auto& o : m_prop_origin)
    {
        ret.push_back(o.first);
    }
    return ret;
}


TcamPropertyBase* TcamCollection::get_property(const std::string& name)
{
    if (auto e = origin_of_property(name.c_str()))
    {
        GError* err = nullptr;
        auto prop = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(e), name.c_str(), &err);

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
    return nullptr;
}


gboolean TcamCollection::set_property(const std::string& /*name*/, const GValue* /*value*/)
{
    //if (auto e = origin_of_property(name))
        //{
        //GError* err = nullptr;
        //return tcam_prop_set_tcam_property(TCAM_PROP(e), name.c_str(), value, &err);
        //}
    return FALSE;
}

GstElement* TcamCollection::origin_of_property(const std::string& name)
{
    try
    {
        return m_prop_origin.at(name);
    }
    catch(const std::out_of_range&)
    {
        return nullptr;
    }
}

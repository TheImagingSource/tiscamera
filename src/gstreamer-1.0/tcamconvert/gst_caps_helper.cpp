
//#include <gst_helper/gst_caps_helper.h>
#include "gst_caps_helper.h"

#include <dutils_img_lib/dutils_gst_interop.h>

std::optional<img::dim> gst_helper::get_gst_struct_image_dim(const GstStructure* structure)
{
    int width;
    int height;
    if (!gst_structure_get_int(structure, "width", &width)
        || !gst_structure_get_int(structure, "height", &height))
    {
        return {};
    }
    return img::dim { width, height };
}

img::fourcc gst_helper::get_gst_struct_fcc(const GstStructure* structure)
{
    const auto type = gst_structure_get_field_type(structure, "format");

    assert(gst_structure_get_name(structure) != nullptr);

    const char* format = nullptr;
    if (type == G_TYPE_STRING)
    {
        format = gst_structure_get_string(structure, "format");
    }
    else if (type == GST_TYPE_LIST)
    {
        GST_WARNING( "format is a list not a single entry" );
        return img::fourcc::FCC_NULL;
    }
    else
    {
        //GST_ERROR( "%s", g_type_name( type ) );
        return img::fourcc::FCC_NULL;
    }
    return img_lib::gst::gst_caps_string_to_fourcc(gst_structure_get_name(structure),
                                                   format == nullptr ? "" : format);
}

img::img_type gst_helper::get_gst_struct_image_type(const GstStructure* structure)
{
    auto dim_opt = get_gst_struct_image_dim(structure);
    if (!dim_opt)
    {
        return {};
    }
    img::fourcc fcc = get_gst_struct_fcc(structure);
    if (fcc == img::fourcc::FCC_NULL)
    {
        return {};
    }
    return img::make_img_type(fcc, dim_opt.value());
}

std::optional<double> gst_helper::get_gst_struct_framerate(const GstStructure* structure)
{
    const GValue* frame_rate_field = gst_structure_get_value(structure, "framerate");

    if (frame_rate_field == nullptr)
    {
        return {};
    }

    int fps_numerator = gst_value_get_fraction_numerator(frame_rate_field);
    int fps_denominator = gst_value_get_fraction_denominator(frame_rate_field);
    return (double)fps_numerator / (double)fps_denominator;
}

void gst_helper::set_gst_struct_framerate(GstStructure* structure, double framerate) noexcept
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, GST_TYPE_FRACTION);

    int fps_min_num = 0;
    int fps_min_den = 0;
    gst_util_double_to_fraction(framerate, &fps_min_num, &fps_min_den);
    gst_value_set_fraction(&value, fps_min_num, fps_min_den);
    gst_structure_set_value(structure, "framerate", &value);
    g_value_unset(&value);
}

std::string gst_helper::caps_to_string(const GstCaps* structure)
{
    gchar* tmp = ::gst_caps_to_string(structure);
    if (tmp == nullptr)
    {
        return {};
    }
    std::string rval = tmp;
    g_free(tmp);
    return rval;
}

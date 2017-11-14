

#include "tcamcamera.h"
#include "tcamprop.h"

#include <vector>
#include <string>
#include <stdexcept>

#include <iostream>

#include <assert.h>

using namespace gsttcam;

std::vector<CameraInfo>
gsttcam::get_device_list()
{
    if (!gst_is_initialized())
        throw std::runtime_error("GStreamer is not initialized! gst_init(...) needs to be called before using this function.");
    GstElement* source = gst_element_factory_make("tcambin", "source");
    if (!source)
        throw std::runtime_error("'tcambin' could not be initialized! Check tiscamera installation");
    GSList* serials = tcam_prop_get_device_serials(TCAM_PROP(source));
    std::vector<CameraInfo> ret;
    for (GSList* elem = serials; elem; elem = elem->next)
    {
        gchar *serial = reinterpret_cast<gchar *>(elem->data);
        gchar *name = nullptr;
        gchar *identifier = nullptr;
        gchar *connection_type = nullptr;
        if (tcam_prop_get_device_info(TCAM_PROP(source),
                                      serial,
                                      &name, &identifier, &connection_type))
        {
            CameraInfo info;
            info.serial = serial;
            info.name = name;
            info.identifier = identifier;
            info.connection_type = connection_type;
            ret.push_back(info);
        }

        g_free(name);
        g_free(identifier);
        g_free(connection_type);
    }
    gst_object_unref(source);
    g_slist_free_full(serials, free);

    return ret;
}

std::string
VideoFormatCaps::to_string()
{
    std::string ret;
    if(size.width != 0)
    {
        ret += "size = " + std::to_string(size.width) + " x " + std::to_string(size.height) + " ";
    }
    else
    {
        ret += "size range = [" + std::to_string(size_min.width) + " x ";
        ret += std::to_string(size_min.height) + ", ";
        ret += std::to_string(size_max.width) + " x " + std::to_string(size_max.height) + "] ";
    }
    ret += "color formats = {";
    for(std::string &colorfmt : formats)
    {
        ret += colorfmt + ",";
    }
    ret += "} ";
    if(!framerates.empty())
    {
        ret += "frame rates = {";
        for(FrameRate &rate : framerates)
        {
            ret += std::to_string(rate.numerator) + "/" + std::to_string(rate.denominator) + ",";
        }
        ret += "}";
    }
    else
    {
        ret += "frame rate range = [";
        ret += std::to_string(framerate_min.numerator) + "/";
        ret += std::to_string(framerate_min.denominator) + ",";
        ret += std::to_string(framerate_max.numerator) + "/";
        ret += std::to_string(framerate_max.denominator) + "]";
    }

    return ret;
}

std::string
Property::to_string()
{
    return std::string(type + " property '" + name + "': ");
}

std::string
IntegerProperty::to_string()
{
    return Property::to_string() + "value = " + std::to_string(value) + " " +
           "default = " + std::to_string(default_value) + " " +
           "min = " + std::to_string(min) + " " +
           "max = " + std::to_string(max) + " " +
           "step_size = " + std::to_string(step_size);
}

bool IntegerProperty::set(TcamCamera &cam, int _value)
{
    bool ret = FALSE;
    if(type == "integer")
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_INT);
        g_value_set_int(&gval, _value);
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}
bool IntegerProperty::get(TcamCamera &cam, int &_value)
{
    bool ret = FALSE;
    if(type == "integer")
    {
        std::shared_ptr<IntegerProperty> prop =
            std::dynamic_pointer_cast<IntegerProperty>(cam.get_property(name));
        assert(prop);
        value = _value = prop->value;
        min = prop->min;
        max = prop->max;
        default_value = prop->default_value;
        step_size = prop->step_size;
    }
    return ret;
}

std::string
DoubleProperty::to_string()
{
    return Property::to_string() + "value = " + std::to_string(value) + " " +
    "default = " + std::to_string(default_value) + " " +
    "min = " + std::to_string(min) + " " +
    "max = " + std::to_string(max) + " " +
    "step_size = " + std::to_string(step_size);
}

bool DoubleProperty::set(TcamCamera &cam, double _value)
{
    bool ret = FALSE;
    if(type == "double")
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_DOUBLE);
        g_value_set_double(&gval, _value);
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}

bool DoubleProperty::get(TcamCamera &cam, double &_value)
{
    bool ret = FALSE;
    if(type == "double")
    {
        std::shared_ptr<DoubleProperty> prop =
            std::dynamic_pointer_cast<DoubleProperty>(cam.get_property(name));
        assert(prop);
        value = _value = prop->value;
        min = prop->min;
        max = prop->max;
        default_value = prop->default_value;
        step_size = prop->step_size;
    }
    return ret;
}

std::string
StringProperty::to_string()
{
    return Property::to_string() + "value = '" + value + "' default_value = '" + default_value +"'";
}

bool StringProperty::set(TcamCamera &cam, std::string _value)
{
    bool ret = FALSE;
    if((type == "string") || (type == "enum"))
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_STRING);
        g_value_set_string(&gval, _value.c_str());
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}

bool StringProperty::get(TcamCamera &cam, std::string &_value)
{
    bool ret = FALSE;
    if((type == "string") || (type == "enum"))
    {
        std::shared_ptr<StringProperty> prop =
            std::dynamic_pointer_cast<StringProperty>(cam.get_property(name));
        assert(prop);
        value = _value = prop->value;
    }
    return ret;
}

std::string
EnumProperty::to_string()
{
    std::string ret = StringProperty::to_string() + " values = {";
    for(std::string &value : values)
    {
        ret += value + ", ";
    }
    ret += "}";

    return ret;
}

std::string
BooleanProperty::to_string()
{
    return Property::to_string() + "value = " + std::string(value ? "true" : "false") +
    " default_value = " + std::string(default_value ? "true" : "false");
}

bool
BooleanProperty::get(TcamCamera &cam, int &value)
{
    bool ret = FALSE;
    if(type == "boolean")
    {
        std::shared_ptr<BooleanProperty> prop =
            std::dynamic_pointer_cast<BooleanProperty>(cam.get_property(name));
        assert(prop);
        value = prop->value ? 1 : 0;
        ret = TRUE;
    }
    return ret;
}

bool
BooleanProperty::set(TcamCamera &cam, int value)
{
    bool ret = FALSE;
    if((type == "boolean") || (type == "button"))
    {
        GValue gval = G_VALUE_INIT;
        g_value_init(&gval, G_TYPE_BOOLEAN);
        g_value_set_boolean(&gval, value ? TRUE : FALSE);
        ret = cam.set_property(name.c_str(), gval);
    }
    return ret;
}

TcamCamera::TcamCamera(std::string serial = "")
{
    if (!gst_is_initialized())
        throw std::runtime_error("GStreamer is not initialized! gst_init(...) needs to be called before using this function.");
    create_pipeline();
    if (serial != "")
        g_object_set(tcambin_, "serial", serial.c_str(), nullptr);
    ensure_ready_state();
    GstElement *src = gst_bin_get_by_name(GST_BIN(tcambin_), "tcambin-source");
    assert(src);
    g_object_set(src, "do-timestamp", false, nullptr);
    gst_object_unref(src);

    videocaps_ = initialize_format_list();
}

TcamCamera::~TcamCamera()
{
    g_print("pipeline refcount at cleanup: %d\n", GST_OBJECT_REFCOUNT_VALUE(pipeline_));
    gst_object_unref(pipeline_);
}

void
TcamCamera::create_pipeline()
{
    pipeline_ = gst_pipeline_new("pipeline");
    tcambin_ = gst_element_factory_make("tcambin", nullptr);
    if (!tcambin_)
        throw std::runtime_error("'tcambin' could not be initialized! Check tiscamera installation");
    capturecapsfilter_ = gst_element_factory_make("capsfilter", nullptr);
    tee_ = gst_element_factory_make("tee", "tee");
    GstElement *queue = gst_element_factory_make("queue", nullptr);
    capturesink_ = gst_element_factory_make("appsink", nullptr);
    g_object_set(capturesink_, "max-buffers", 4, "drop", true, nullptr);
    assert(pipeline_ && tee_ && capturecapsfilter_ && queue && capturesink_);


    gst_bin_add_many(GST_BIN(pipeline_),
                     tcambin_, capturecapsfilter_, tee_, queue, capturesink_, nullptr);
    assert(gst_element_link_many(tcambin_, capturecapsfilter_, tee_, queue, capturesink_, nullptr));
}

void
TcamCamera::ensure_ready_state()
{
    GstState state;
    if ((gst_element_get_state(tcambin_, &state, nullptr, GST_CLOCK_TIME_NONE) == GST_STATE_CHANGE_SUCCESS) &&
        state == GST_STATE_NULL)
    {
        gst_element_set_state(tcambin_, GST_STATE_READY);
        gst_element_get_state(tcambin_, nullptr, nullptr, GST_CLOCK_TIME_NONE);
    }
}

std::vector<VideoFormatCaps>
TcamCamera::initialize_format_list()
{
    GstState state;
    gst_element_set_state(tcambin_, GST_STATE_READY);
    gst_element_get_state(tcambin_, nullptr, nullptr, GST_CLOCK_TIME_NONE);

    std::vector<VideoFormatCaps> ret;

    GstPad *pad = gst_element_get_static_pad(tcambin_, "src");
    assert(pad);
    GstCaps *caps = gst_pad_query_caps(pad, nullptr);
    assert(caps);

    for(int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        VideoFormatCaps fmt = {0};
        GstStructure* s = gst_caps_get_structure(caps, i);
        if (s)
        {
            if (!g_strcmp0(gst_structure_get_name(s), "ANY"))
                continue;
            const GValue *width = gst_structure_get_value(s, "width");
            const GValue *height = gst_structure_get_value(s, "height");
            assert(width && height);
            if (G_VALUE_HOLDS_INT(width))
            {
                assert(G_VALUE_HOLDS_INT(height));
                fmt.size.width = g_value_get_int(width);
                fmt.size.height = g_value_get_int(height);
            }
            else if (GST_VALUE_HOLDS_INT_RANGE(width))
            {
                assert(GST_VALUE_HOLDS_INT_RANGE(height));
                fmt.size_min.width = gst_value_get_int_range_min(width);
                fmt.size_min.height = gst_value_get_int_range_min(height);
                fmt.size_max.width = gst_value_get_int_range_max(width);
                fmt.size_max.height = gst_value_get_int_range_max(height);
            }
            else
            {
                assert(FALSE && "Invalid or missing width/height");
            }

            // Handle color formats
            const GValue *format = gst_structure_get_value(s, "format");
            // TODO: Support jpeg
            if (format == nullptr)
                continue;
            if (G_VALUE_HOLDS_STRING(format))
            {
                fmt.formats.push_back(std::string(gst_structure_get_string(s, "format")));
            }
            else if (GST_VALUE_HOLDS_LIST(format))
            {
                for (int i=0; i < gst_value_list_get_size(format); i++)
                {
                    const GValue *val = gst_value_list_get_value(format, i);
                    assert(val && G_VALUE_HOLDS_STRING(val));
                    fmt.formats.push_back(std::string(g_value_get_string(val)));
                }
            }
            else
            {
                assert(FALSE && "Invalid or missing format");
            }

            // Handle frame rates
            const GValue *framerates = gst_structure_get_value(s, "framerate");
            assert(framerates);
            if (GST_VALUE_HOLDS_LIST(framerates))
            {
                for (int i=0; i < gst_value_list_get_size(framerates); i++)
                {
                    const GValue *val = gst_value_list_get_value(framerates, i);
                    assert(val && GST_VALUE_HOLDS_FRACTION(val));
                    FrameRate rate;
                    rate.numerator = gst_value_get_fraction_numerator(val);
                    rate.denominator = gst_value_get_fraction_denominator(val);
                    fmt.framerates.push_back(rate);
                }
            }
            else if (GST_VALUE_HOLDS_FRACTION_RANGE(framerates))
            {
                const GValue *min = gst_value_get_fraction_range_min(framerates);
                const GValue *max = gst_value_get_fraction_range_max(framerates);
                assert(min && max);
                fmt.framerate_min.numerator = gst_value_get_fraction_numerator(min);
                fmt.framerate_min.denominator = gst_value_get_fraction_denominator(min);
                fmt.framerate_min.numerator = gst_value_get_fraction_numerator(min);
                fmt.framerate_min.denominator = gst_value_get_fraction_denominator(min);
            }
            else
            {
                assert(FALSE && "Invalid or missing framerate");
            }

            ret.push_back(fmt);
        }
    }
    gst_caps_unref(caps);
    g_object_unref(pad);
    gst_element_set_state(tcambin_, GST_STATE_NULL);

    return ret;
}

std::vector<VideoFormatCaps>
TcamCamera::get_format_list()
{
    return videocaps_;
}

std::shared_ptr<Property>
TcamCamera::get_property(std::string name)
{
    GValue value = G_VALUE_INIT;
    GValue min = G_VALUE_INIT;
    GValue max = G_VALUE_INIT;
    GValue default_value = G_VALUE_INIT;
    GValue step_size = G_VALUE_INIT;
    GValue type = G_VALUE_INIT;
    GValue flags = G_VALUE_INIT;
    GValue category = G_VALUE_INIT;
    GValue group = G_VALUE_INIT;

    gboolean ret = tcam_prop_get_tcam_property(TCAM_PROP(tcambin_),
                                               (gchar*)name.c_str(),
                                               &value,
                                               &min,
                                               &max,
                                               &default_value,
                                               &step_size,
                                               &type,
                                               &flags,
                                               &category,
                                               &group);

    if (!ret)
    {
        throw std::runtime_error("Failed to query property");
    }
    std::shared_ptr<Property> prop;
    const char *t = g_value_get_string(&type);
    if(!g_strcmp0(t, "integer"))
    {
        std::shared_ptr<IntegerProperty> intprop(new IntegerProperty);
        intprop->value = g_value_get_int(&value);
        intprop->min = g_value_get_int(&min);
        intprop->max = g_value_get_int(&max);
        intprop->default_value = g_value_get_int(&default_value);
        intprop->step_size = g_value_get_int(&step_size);
        prop = intprop;
    }
    else if(!g_strcmp0(t, "double"))
    {
        std::shared_ptr<DoubleProperty> dblprop(new DoubleProperty);
        dblprop->value = g_value_get_double(&value);
        dblprop->min = g_value_get_double(&min);
        dblprop->max = g_value_get_double(&max);
        dblprop->default_value = g_value_get_double(&default_value);
        dblprop->step_size = g_value_get_double(&step_size);
        prop = dblprop;
    }
    else if(!g_strcmp0(t, "string"))
    {
        std::shared_ptr<StringProperty> strprop(new StringProperty);
        strprop->value = std::string(g_value_get_string(&value));
        strprop->default_value = std::string(g_value_get_string(&default_value));
        prop = strprop;
    }
    else if(!g_strcmp0(t, "enum"))
    {
        std::shared_ptr<EnumProperty> enumprop(new EnumProperty);
        enumprop->value = std::string(g_value_get_string(&value));
        enumprop->default_value = std::string(g_value_get_string(&default_value));
        GSList *entries = tcam_prop_get_tcam_menu_entries(TCAM_PROP(tcambin_),
                                                          (gchar*)name.c_str());
        for(GSList *entry = entries; entry; entry = g_slist_next(entry))
        {
            enumprop->values.push_back((char*)entry->data);
        }
        prop = enumprop;
    }
    else if(!g_strcmp0(t, "boolean") || !g_strcmp0(t, "button"))
    {
        std::shared_ptr<BooleanProperty> booleanprop(new BooleanProperty);
        booleanprop->value = g_value_get_boolean(&value);
        booleanprop->default_value = g_value_get_boolean(&default_value);
        prop = booleanprop;
    }
    else
    {
        prop = std::shared_ptr<Property> (new Property);
    }
    prop->name = name;
    prop->group = std::string(g_value_get_string(&group));
    prop->category = std::string(g_value_get_string(&category));
    prop->type = std::string(t);
    return prop;
}

std::vector<std::shared_ptr<Property>>
TcamCamera::get_camera_property_list()
{
    GSList *names = tcam_prop_get_tcam_property_names(TCAM_PROP(tcambin_));
    std::vector<std::shared_ptr<Property>> pptylist;

    for(GSList *name = names; name; name = g_slist_next(name))
    {
        try
        {
            std::shared_ptr<Property> prop = get_property(std::string((char*)name->data));
            pptylist.push_back(prop);
        }
        catch(...)  {}
    }

    return pptylist;
}

bool
TcamCamera::set_property(std::string name, GValue &value)
{
    bool ret;
    ret = tcam_prop_set_tcam_property(TCAM_PROP(tcambin_),
                                      (gchar*)name.c_str(),
                                      &value);
    return ret;
}

void
TcamCamera::set_capture_format(std::string format, FrameSize size, FrameRate framerate)
{
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, size.width,
                                        "height", G_TYPE_INT, size.height,
                                        "framerate", GST_TYPE_FRACTION, framerate.numerator, framerate.denominator,
                                        nullptr);
    assert(caps);
    if(format != "")
        gst_caps_set_simple(caps, "format", G_TYPE_STRING, format.c_str(), nullptr);

    g_object_set(G_OBJECT(capturecapsfilter_), "caps", caps, nullptr);
    gst_caps_unref(caps);
}

bool
TcamCamera::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    gst_element_get_state(pipeline_, NULL, NULL, GST_CLOCK_TIME_NONE);
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline_),
                              GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
    return TRUE;
}

bool
TcamCamera::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_element_get_state(pipeline_, NULL, NULL, GST_CLOCK_TIME_NONE);
    return TRUE;
}

GstFlowReturn
TcamCamera::new_frame_callback(GstAppSink *appsink, gpointer data)
{
    TcamCamera *this_ = static_cast<TcamCamera *>(data);
    if (this_->callback_)
    {
        return this_->callback_(appsink, this_->callback_data_);
    }
    return GST_FLOW_OK;
}

void
TcamCamera::set_new_frame_callback(std::function<GstFlowReturn(GstAppSink *appsink, gpointer data)>callback,
                                   gpointer data)
{
    callback_ = callback;
    callback_data_ = data;
    GstAppSinkCallbacks callbacks = {nullptr, nullptr, new_frame_callback};
    gst_app_sink_set_callbacks(GST_APP_SINK(capturesink_), &callbacks, this, nullptr);
}

GstElement *
TcamCamera::get_pipeline()
{
    gst_object_ref(pipeline_);
    return pipeline_;
}

void
TcamCamera::enable_video_display(GstElement *displaysink)
{
    if(displaybin_)
        return;
    displaybin_ = gst_element_factory_make("bin", nullptr);
    GstElement *queue = gst_element_factory_make("queue", nullptr);
    GstElement *convert = gst_element_factory_make("videoconvert", nullptr);
    GstElement *capsfilter = gst_element_factory_make("capsfilter", nullptr);
    gst_bin_add(GST_BIN(pipeline_), displaybin_);
    gst_bin_add_many(GST_BIN(displaybin_), queue, convert, capsfilter, displaysink, nullptr);
    if (!gst_element_link_many(tee_, queue, convert, capsfilter, displaysink, nullptr))
        throw std::runtime_error("Could not link elements");
}

void
TcamCamera::disable_video_display()
{
    if(displaybin_)
    {
        gst_object_ref(displaybin_);
        gst_bin_remove(GST_BIN(pipeline_), displaybin_);
        gst_element_set_state(displaybin_, GST_STATE_NULL);
        gst_element_get_state(displaybin_, nullptr, nullptr, GST_CLOCK_TIME_NONE);
        gst_object_unref(displaybin_);
        displaybin_ = nullptr;
    }
}

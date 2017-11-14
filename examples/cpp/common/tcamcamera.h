#ifndef __TCAMCAMERA_H__
#define __TCAMCAMERA_H__

#include <vector>
#include <string>
#include <memory>
#include <functional>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/videooverlay.h>


namespace gsttcam {


class TcamCamera;

/*
*
*/
struct CameraInfo
{
    std::string serial;
    std::string name;
    std::string identifier;
    std::string connection_type;
};

/*
*
*/
struct FrameRate
{
    int numerator;
    int denominator;
};

/*
*
*/
struct FrameSize
{
    int width;
    int height;
};

/*
*
*/
class VideoFormatCaps
{
public:
    FrameSize size;
    FrameSize size_min;
    FrameSize size_max;
    std::vector<std::string> formats;
    std::vector<FrameRate> framerates;
    FrameRate framerate_min;
    FrameRate framerate_max;

    std::string to_string();
};

/*
*
*/
class Property
{
public:
    std::string name;
    std::string category;
    std::string group;
    std::string type;

    virtual std::string to_string();

    // Convenience getters / setters
    virtual bool get(TcamCamera &cam, int &value) {return false;}
    virtual bool get(TcamCamera &cam, double &value) {return false;}
    virtual bool get(TcamCamera &cam, std::string &value) {return false;}
    virtual bool set(TcamCamera &cam, int value)  {return false;}
    virtual bool set(TcamCamera &cam, double value) {return false;}
    virtual bool set(TcamCamera &cam, std::string value) {return false;}
};

class IntegerProperty : public Property
{
public:
    int value;
    int default_value;
    int min;
    int max;
    int step_size;

	virtual std::string to_string() override;
	virtual bool get( TcamCamera &cam, int &value ) override;
	virtual bool set(TcamCamera &cam, int value) override;
};

class DoubleProperty : public Property
{
public:
    double value;
    double default_value;
    double min;
    double max;
    double step_size;

    virtual std::string to_string() override;
    virtual bool get(TcamCamera &cam, double &value) override;
    virtual bool set(TcamCamera &cam, double value) override;
};

class StringProperty : public Property
{
public:
    std::string value;
    std::string default_value;

    virtual std::string to_string();
    virtual bool get(TcamCamera &cam, std::string &value) override;
    virtual bool set(TcamCamera &cam, std::string value) override;
};

class EnumProperty : public StringProperty
{
public:
    std::vector<std::string> values;

    virtual std::string to_string() override;
};

class BooleanProperty : public Property
{
public:
    bool value;
    bool default_value;

    virtual std::string to_string() override;
    virtual bool get(TcamCamera &cam, int &value) override;
    virtual bool set(TcamCamera &cam, int value) override;
};

class ButtonProperty : public BooleanProperty
{
public:
    virtual bool set(TcamCamera &cam, int value=true) override;
private:
    virtual bool get(TcamCamera &cam, int &value) override;
};

class TcamCamera
{
    public:
        TcamCamera(std::string serial);
        ~TcamCamera();

		TcamCamera( TcamCamera& ) = delete;
		TcamCamera( TcamCamera&& other )
			: pipeline_ { other.pipeline_ }
		{
			other.pipeline_ = nullptr;
		};

		TcamCamera& operator= ( const TcamCamera& ) = delete;
		TcamCamera& operator= ( TcamCamera&& other)
		{
			gst_object_unref( pipeline_ );
			pipeline_ = other.pipeline_;
			other.pipeline_ = nullptr;
		}


        /*
        * Get a list of all video formats supported by the device
        */
        std::vector<VideoFormatCaps> get_format_list();
        /*
        * Get a list of all properties supported by the device
        */
        std::vector<std::shared_ptr<Property>> get_camera_property_list();
        /*
        * Get a single camera property
        */
        std::shared_ptr<Property> get_property(std::string name);

		template<typename T>
		std::shared_ptr<T> get_property( std::string name )
		{
			return std::dynamic_pointer_cast<T>( get_property(name) );
		}

        bool set_property(std::string name, GValue &val);
        /*
        * Set the video format for capturing
        */
        void set_capture_format(std::string format, FrameSize size, FrameRate framerate);
        /*
        * Set a callback to be called for each new frame
        */
        void set_new_frame_callback(std::function<GstFlowReturn(GstAppSink *appsink, gpointer data)>callback,
                                    gpointer data);;
        /*
        * Start capturing video data
        */
        bool start();
        /*
        * Stop capturing video data
        */
        bool stop();
        /*
        * Get the GStreamer pipeline used for video capture
        */
        GstElement *get_pipeline();
        /*
        * Connect a video display sink element to the capture pipeline
        */
        void enable_video_display(GstElement *displaysink);
        /*
        * Disconnect the video display sink element from the capture pipeline
        */
        void disable_video_display();

    private:
        GstElement *pipeline_ = nullptr;
        GstElement *tcambin_ = nullptr;
        GstElement *capturecapsfilter_ = nullptr;
        GstElement *tee_ = nullptr;
        GstElement *capturesink_ = nullptr;
        GstElement *displaybin_ = nullptr;
        GstElement *displaysink_ = nullptr;
        std::vector<VideoFormatCaps> videocaps_;
        std::function<GstFlowReturn(GstAppSink *appsink, gpointer data)>callback_;
        gpointer callback_data_ = nullptr;
        guintptr window_handle_ = 0;

        static GstFlowReturn new_frame_callback(GstAppSink *appsink, gpointer data);
        void ensure_ready_state();
        void create_pipeline();
        std::vector<VideoFormatCaps> initialize_format_list();
};

std::vector<CameraInfo>get_device_list();

};

#endif//__TCAMCAMERA_H__
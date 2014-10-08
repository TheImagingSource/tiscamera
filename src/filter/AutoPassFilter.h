
#ifndef AUTOPASSFILTER_H_
#define AUTOPASSFILTER_H_

#include <auto_alg/auto_alg_pass.h>

#include <FilterBase.h>

// forward declaration
class AutoPassFilter;

class PropertyHandler: public PropertyImpl
{
public:

    friend class AutoPassFilter;
    
    PropertyHandler ();
    
    bool isAvailable (const Property&);

    bool setProperty (const Property&);
    
    bool getProperty (Property&);

protected:
    
    // device properties
    std::weak_ptr<PropertyInteger> property_exposure;
    std::weak_ptr<PropertyInteger> property_gain;
    std::weak_ptr<PropertyInteger> property_iris;

    // generated properties
    std::shared_ptr<PropertySwitch> prop_auto_exposure;
    std::shared_ptr<PropertySwitch> prop_auto_gain;
    std::shared_ptr<PropertySwitch> prop_auto_iris;
    
    std::shared_ptr<PropertySwitch>  prop_auto_wb;
    std::shared_ptr<PropertyInteger> prop_wb_r;
    std::shared_ptr<PropertyInteger> prop_wb_g;
    std::shared_ptr<PropertyInteger> prop_wb_b;
        
    
};


class AutoPassFilter : public FilterBase, public std::enable_shared_from_this<AutoPassFilter>
{
public:

    AutoPassFilter ();

    void reset ();

    struct FilterDescription getDescription () const;

    bool transform (MemoryBuffer& in, MemoryBuffer& out );

    bool apply (std::shared_ptr<MemoryBuffer>);
    
    bool setStatus (const PIPELINE_STATUS&);
    PIPELINE_STATUS getStatus () const;

    void getVideoFormat (VideoFormat& in, VideoFormat& out) const;
    bool setVideoFormat(const VideoFormat&);
    bool setVideoFormat(const VideoFormat& in, const VideoFormat& out);
    
    void setDeviceProperties (std::vector<std::shared_ptr<Property>>);

    std::vector<std::shared_ptr<Property>> getFilterProperties ();

private:

    void update_params ();

    void set_gain     (int);
    void set_exposure (int);
    void set_iris     (int);

    // general
    bool valid;
    unsigned int skipped_buffer;

    int		wb_r;
    int		wb_g;
    int		wb_b;
    unsigned int exposure_max;
    
    //tis_imaging::
    FilterDescription description;
    PIPELINE_STATUS current_status;
    tis_imaging::VideoFormat input_format;
    
    auto_alg::auto_pass_params params;
    auto_alg::auto_pass_state state;

    std::shared_ptr<PropertyHandler> handler;


};


extern "C"
{

    // FilterBase* create ();
    FB* create ();

    void close (FB*);

}

#endif /* AUTOPASSFILTER_H_ */















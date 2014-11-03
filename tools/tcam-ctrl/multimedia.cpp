
#include "multimedia.h"

#include <iostream>
#include <unistd.h>

using namespace tcam;


bool tcam::save_image (CaptureDevice& g, const std::string& filename)
{

    struct data
    {
        CaptureDevice* g;
        std::string filename;
        int count;
        bool wait;
    };

    auto f = [] (MemoryBuffer* buf, void* userdata)
        {
            data* d = static_cast<data*>(userdata);

            if (d->count >= 2)
            {
                //save_jpeg(*buf, d->filename);
                d->wait = false;
            }
            else
            {
                d->count++;
            }
        };

    auto sink = std::make_shared<ImageSink>();

    data d;
    d.g = &g;
    d.filename = filename;
    d.count = 0;
    d.wait = true;

    sink->registerCallback(f, &d);

    VideoFormat v;
    v.fromString("format=RGB32,width=640,height=480,binning=0,framerate=0.000000");
    v.setFourcc(FOURCC_RGB24);
    g.setVideoFormat(v);
    g.startStream(sink);

    // wait until image was captured
    while(d.wait)
    {
        usleep(500);
    }
    d.g->stopStream();

    std::cout << "Successfully saved image" << std::endl;

    return true;
}


bool tcam::save_stream (CaptureDevice& g, const std::string& filename)
{
    return false;
}

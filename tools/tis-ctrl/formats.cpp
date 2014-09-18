
#include "formats.h"

#include <iostream>
#include <iomanip>

void list_formats (const std::vector<VideoFormatDescription>& available_formats)
{
    for (const VideoFormatDescription& f : available_formats)
    {
        auto desc = f.getFormatDescription();

        std::cout << "Format: " << desc.description << " - Fourcc(" << desc.fourcc << ")" //<< std::endl
                  << std::endl;
        for (const auto& s : f.getResolutionsFramesrates())
        {
            std::cout << "\tResolution: " << s.resolution.width << "x" << s.resolution.height << std::endl;
            for (const auto& fps : s.fps)
            {
                std::cout << "\t\t" << std::setw(8) << std::fixed << std::setprecision(4)<< fps << " fps" << std::endl;
            }
        }
        std::cout << std::endl;
    }
}


void print_active_format (const VideoFormat& format)
{
    std::cout << "Active format:" << std::endl;
    std::cout << fourcc2description(format.getFourcc()) << std::endl
              << format.getSize().width << "x" << format.getSize().height << std::endl
              << format.getFramerate() << std::endl
              << format.getBinning() << std::endl << std::endl;
}


bool set_active_format (Grabber& g, const std::string& new_format)
{
    VideoFormat v;

    bool ret = v.fromString(new_format);

    if (ret)
    {
        ret = g.setVideoFormat(v);

        return ret;
    }
    else
    {
        std::cout << "Invalid string description!" << std::endl;
    }
    return false;
}

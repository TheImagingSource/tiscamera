

#include "tcam_transform.h"

#include <by8/by8torgb_conv.h>
#include "dutils_header.h"

#include <vector>
#include <algorithm>

using namespace tcam;


static void transform_by8_to_rgb (const struct tcam_image_buffer* in,
                                  struct tcam_image_buffer* out)
{
    auto img_out = to_img_desc(*out);
    auto img_in  = to_img_desc(*in);

    by8_transform::transform_by8_options options = {};

    options.options = by8_transform::transform_by8_options::FlipImage;
    options.opt_level = 0;

    by8_transform::transform_by8_to_dest(img_out, img_in, options);
}


bool tcam::convert_to_format (const struct tcam_image_buffer* in,
                              struct tcam_image_buffer* out)
{
    if (img::isBayerFCC(in->format.fourcc) &&
        (out->format.fourcc == FOURCC_RGB32 || out->format.fourcc == FOURCC_RGB24))
    {
        transform_by8_to_rgb(in, out);
    }

    return false;
}

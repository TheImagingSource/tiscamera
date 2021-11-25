
#pragma once

#include "../dutils_img/img_type.h"
#include "../dutils_img/image_transform_base.h"

#include <string>

namespace img_lib
{
    std::string     to_string( const img::img_type& type );
    std::string     to_string( const img::img_descriptor& cfg );
} // img_lib


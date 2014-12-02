
#include "public_utils.h"

#include "utils.h"

using namespace tcam;

const char* tcam_fourcc_to_description (uint32_t fourcc)
{
    return fourcc2description(fourcc);
}


uint32_t tcam_description_to_fourcc (const char* description)
{
    return description2fourcc(description);
}

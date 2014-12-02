

#ifndef TCAM_ALGORITHM_CONVERT_H
#define TCAM_ALGORITHM_CONVERT_H

#include "base_types.h"

#include "tcam_alg_base.h"

namespace tcam
{

bool convert_to_format (const struct tcam_image_buffer* in, struct tcam_image_buffer* out);

} /* namespace tcam */

#endif /* TCAM_ALGORITHM_CONVERT_H */

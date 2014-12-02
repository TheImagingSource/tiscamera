
#ifndef TCAM_AUTO_ALG_H
#define TCAM_AUTO_ALG_H

#include "tcam.h"

#include "tcam_alg_base.h"

namespace tcam
{

void reinit_auto_pass_context (tcam_auto_alg_context);

size_t get_auto_pass_context_size ();

tcam_auto_alg_context create_auto_pass_context (void* context_space,
                                                struct tcam_create_auto_alg_params* params);

tcam_auto_alg_results auto_pass (const struct tcam_image_buffer* buffer,
                                 struct tcam_auto_alg_params* params,
                                 tcam_auto_alg_context state);

void destroy_auto_pass_context (tcam_auto_alg_context);


void apply_wb_to_bayer_img (struct tcam_image_buffer* buffer,
                            unsigned char r, unsigned char g, unsigned char b, unsigned int i);


bool overlay (const struct tcam_image_buffer*, const char*, unsigned int pos_x, unsigned int pos_y);

} /* namespace tcam */

#endif /* TCAM_AUTO_ALG_H */

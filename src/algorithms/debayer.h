/*
 * Copyright 2016 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCAM_ALGORITHM_DEBAYER_H
#define TCAM_ALGORITHM_DEBAYER_H

namespace tcam
{

namespace algorithm
{


enum {
    WB_MODE_MANUAL = 0,
    WB_MODE_AUTO,
    WB_MODE_ONE_PUSH
};


struct _debayer_data
{
    int use_ccm;
    int use_rbgain;

    int wb_auto_mode;

    // Color Correction Matrix
    int ccm[3][3];

    // red / blue gain
    int rgain;
    int bgain;
};

typedef struct _debayer_data debayer_data_t;


void debayer (const unsigned char* input_buffer,
              unsigned int input_pitch,
              const unsigned int fourcc_in,
              unsigned char* output_buffer,
              unsigned int output_pitch,
              const unsigned int fourcc_out);

} /* namespace algorithm */

} /* namespace tcam */

#endif /* TCAM_ALGORITHM_DEBAYER_H */

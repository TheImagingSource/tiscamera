/*
 * Copyright 2017 The Imaging Source Europe GmbH
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


#ifndef TCAM_BACKEND_AFU050_DEFINITIONS_H
#define TCAM_BACKEND_AFU050_DEFINITIONS_H

enum
{
    PU_CONTROL_UNDEFINE = 0x00,
    PU_CONTRAST_CONTROL = 0x03,
    PU_GAIN_CONTROL = 0x04,
    PU_SATURATION_CONTROL = 0x07,
    PU_SHARPNESS_CONTROL = 0x08,
    PU_GAMMA_CONTROL = 0x09,
};

enum
{
    CT_CONTROL_UNDEFINE = 0x00,
    CT_EXPOSURE_TIME_ABSOLUTE_CONTROL = 0x04,
    CT_FOCUS_AUTO_CONTROL = 0x08,
};


enum
{
    XU_CONTROL_UNDEFINED = 0x00,
    XU_GAIN_R_CONTROL = 0x06,
    XU_GAIN_G_CONTROL = 0x07,
    XU_GAIN_B_CONTROL = 0x08,
    XU_FOCUS_ONE_PUSH = 0x0c, // size: 1, 1 = enable, self clearing
    XU_AUTO_EXPOSURE = 0x0d, // size: 1, 0 = disable, 1 = enable
    XU_AUTO_GAIN = 0x0e, // size: 1, 0 = disable, 1 = enable
    XU_AUTO_WHITE_BALANCE = 0x0f, // size: 1, 0 = disable, 1 = enable
};


enum VC_UNIT
{
    VC_UNIT_HEADER = 0x00,
    VC_UNIT_INPUT_TERMINAL = 0x01,
    VC_UNIT_OUTPUT_TERMINAL = 0x02,
    VC_UNIT_PROCESSING_UNIT = 0x03,
    VC_UNIT_EXTENSION_UNIT = 0x04
};


enum CONTROL_CMD
{
    RC_UNDEFINED = 0x00,
    SET_CUR = 0x01,
    GET_CUR = 0x81,
    GET_MIN = 0x82,
    GET_MAX = 0x83,
    GET_RES = 0x84,
    GET_LEN = 0x85,
    GET_INFO = 0x86,
    GET_DEF = 0x87
};


#endif /* TCAM_BACKEND_AFU050_DEFINITIONS_H */

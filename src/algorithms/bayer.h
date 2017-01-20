/*
 * Copyright 2013 The Imaging Source Europe GmbH
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

#ifndef TCAM_ALGORITHM_BAYER_H
#define TCAM_ALGORITHM_BAYER_H


#ifdef __cplusplus
extern "C"
{
#endif


/* available bayer patterns */
typedef enum
{
    BG = 0, /* BGGR */
    GB,     /* GBRG */
    GR,     /* GRBG */
    RG,     /* RGGB */
} tBY8Pattern;


/* available image formats */
typedef enum
{
    BAYER,
    RGB,
    GRAY,
    UNDEFINED_FORMAT,
} format;


typedef struct
{
    unsigned int R;
    unsigned int G;
    unsigned int B;
} rgb_tripel;


/**
 * @name next_pixel
 * @param pattern - the pattern of the current pixel
 * @return bayer pattern of the following pixel
 */
tBY8Pattern next_pixel (tBY8Pattern pattern);


/**
 * @name next_line
 * @param pattern - the pattern of the current line
 * @return bayer pattern of the next line
 */
tBY8Pattern next_line (tBY8Pattern pattern);


/**
 * @name bayer_to_string
 * @param pattern - bayer pattern that shall be represented via string
 * @return char* to a string representation of pattern
 */
const char* bayer_to_string (tBY8Pattern pattern);


/**
 * @name initial_offset
 * @param pattern
 * @param line_width
 * @param bytes_per_pixel
 * @return
 * @brief
 */
unsigned int initial_offset (tBY8Pattern pattern, unsigned int line_width, unsigned int bytes_per_pixel);


/* convert desc to int */
#define MAKE_FOURCC(a,b,c,d)        ((uint32_t)((a)|(b)<<8|(c)<<16|(d)<<24))


#ifdef __cplusplus
}
#endif

#endif  /* TCAM_ALGORITHM_BAYER_H */

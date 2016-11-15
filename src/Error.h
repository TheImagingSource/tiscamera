/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#ifndef TCAM_ERROR_H
#define TCAM_ERROR_H

#include "simplectypes.h"

#include <string>

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

class Error
{

public:

    /** Constructs an unknown Error **/
    Error ();

    /** Constructs an unknown error with given message
     * @param errordesc description of the error
     **/
    // Error (const std::string& errordesc);
    Error (const std::string& errordesc, int err_no);


    /** constructs an error with given error code and uses the string from the resources
     * @param e error code
     **/
    Error (tErrorEnum e);

    Error (int errno);

    /** constructs an error as a copy of the given one
     * @param e Error to copy
     **/
    Error (const Error& e);

    std::string get_string () const
    {
        return to_string();
    }

    std::string to_string () const
    {
        return m_String;
    }

    tErrorEnum getVal () const
    {
        return m_Enum;
    }

    int get_errno () const
    {
        return m_errno;
    }

    bool is_error () const
    {
        return eNOERROR != m_Enum;
    }

    static int terror2errno (tErrorEnum err);

private:

    int m_errno;
    tErrorEnum m_Enum;
    std::string m_String;

};


Error get_error ();

void set_error (const Error& err);

void reset_error ();

} /* namespace tcam */

/** @} */

#endif /* TCAM_ERROR_H */

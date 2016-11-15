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

#include "Error.h"

using namespace tcam;


Error::Error ():
    m_errno(0), m_Enum(eNOERROR), m_String("")
{}


Error::Error (const std::string& errordesc, int err_no)
    : m_errno(err_no), m_String(errordesc)
{
    // TODO: determine other values
}


Error::Error (tErrorEnum e)
    : m_Enum(e), m_String("")
{
    // TODO: determine other values
}


Error::Error (int errno):
    m_errno(errno), m_String("")
{
    // TODO: determine other values
}


Error::Error (const Error& e)
    : m_errno(e.m_errno), m_Enum(e.m_Enum), m_String(e.m_String)
{}


Error global_last_error;

Error tcam::get_error ()
{
    return global_last_error;
}


void tcam::set_error (const Error& err)
{
    global_last_error = err;
}


void tcam::reset_error ()
{
    set_error(Error());
}

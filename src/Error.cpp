


#include "Error.h"

using namespace tis_imaging;


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


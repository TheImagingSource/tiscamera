
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

    std::string getString () const
    {
        return toString();
    }

    std::string toString () const
    {
        return m_String;
    }

    tErrorEnum getVal () const
    {
        return m_Enum;
    }

    int getErrno () const
    {
        return m_errno;
    }

    bool isError () const
    {
        return eNOERROR != m_Enum;
    }

    static int terror2errno (tErrorEnum err);

private:

    int m_errno;
    tErrorEnum m_Enum;
    std::string m_String;

};


Error getError ();

void setError (const Error& err);

void resetError ();

} /* namespace tcam */

/** @} */

#endif /* TCAM_ERROR_H */

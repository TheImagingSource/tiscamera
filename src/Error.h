
#ifndef ERROR_H_
#define ERROR_H_

#include "simplectypes.h"

#include <string>


namespace tis_imaging
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

} /* namespace tis_imaging */

#endif /* ERROR_H_ */

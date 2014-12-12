



#ifndef TCAM_MEMORYBUFFER_H
#define TCAM_MEMORYBUFFER_H

#include "base_types.h"

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

class MemoryBuffer
{

public:

    MemoryBuffer (const struct tcam_image_buffer&);

    MemoryBuffer () = delete;

    ~MemoryBuffer ();

    /**
     *
     */
    tcam_image_buffer getImageBuffer ();

    void setImageBuffer (tcam_image_buffer);

    /**
     * @return Pointer to actual image data
     */
    unsigned char* getData ();


    struct tcam_stream_statistics getStatistics () const;

    bool setStatistics (const struct tcam_stream_statistics&);

    /**
     * @brief Fills MemoryBuffer with 0
     */
    void clear ();

private:

    struct tcam_image_buffer buffer;

    unsigned int lock_count;

};


} /* namespace tcam*/

/** @} */

#endif /* TCAM_MEMORYBUFFER_H */

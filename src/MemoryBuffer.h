



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

    void set_image_buffer (tcam_image_buffer);

    /**
     * @return Pointer to actual image data
     */
    unsigned char* get_data ();


    struct tcam_stream_statistics get_statistics () const;

    bool set_statistics (const struct tcam_stream_statistics&);

    bool lock ();

    bool unlock ();

    bool is_locked () const;

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

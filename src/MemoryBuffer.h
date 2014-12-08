



#ifndef MEMORYBUFFER_H_
#define MEMORYBUFFER_H_

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

    /**
     * @brief Increases lock count by 1
     */
    void lock ();

    /**
     * @return true if user holds MemoryBuffer lock
     */
    bool isLocked ();

    /**
     * @brief Decreases lock count by 1
     */
    void unlock ();

    /**
     * Forcefully reset lock count to 0
     */
    void forceUnlock ();

private:

    struct tcam_image_buffer buffer;

    unsigned int lock_count;

};


} /* namespace tcam*/

/** @} */

#endif /* MEMORYBUFFER_H_ */

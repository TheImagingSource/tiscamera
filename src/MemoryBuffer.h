



#ifndef MEMORYBUFFER_H_
#define MEMORYBUFFER_H_

#include "base_types.h"

namespace tis_imaging
{

class MemoryBuffer
{

public:

    MemoryBuffer (const struct image_buffer&);

    MemoryBuffer () = delete;

    ~MemoryBuffer ();

    /**
     *
     */
    image_buffer getImageBuffer ();

    void setImageBuffer (image_buffer);

    /**
     * @return Pointer to actual image data
     */
    unsigned char* getData ();


    struct stream_statistics getStatistics () const;

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

    struct image_buffer buffer;

    unsigned int lock_count;

};


} /* namespace tis_imaging*/

#endif /* MEMORYBUFFER_H_ */

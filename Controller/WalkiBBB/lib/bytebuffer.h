#ifndef DEF_LIB_BYTEBUFFER_H
#define DEF_LIB_BYTEBUFFER_H

#include <vector>
#include <string>
#include <cstdint>
#include "syncvar/syncvar.h"

/**
 * @defgroup ByteBuffer Byte Buffer
 * @brief Bytes vector with convenient "append" operators.
 * @ingroup Lib
 * @{
 */

/**
 * @brief Bytes vector with convenient "append" operators.
 * @ingroup Lib
 */
class ByteBuffer : public std::vector<uint8_t>
{
public:
    ByteBuffer(int reserveSize = 0);

    ByteBuffer& operator<<(const std::string& str);
    ByteBuffer& operator<<(SyncVar &sv);
    ByteBuffer& operator<<(const vector<uint8_t> &data);

    template<typename T>
    ByteBuffer& operator<<(T obj);
};

/**
 * @brief Appends any object bytes to the ByteBuffer.
 * @param obj the object to append to the byte buffer. The raw object bytes will
 * be copied at the back of the vector.
 * @return A reference to the ByteBuffer.
 */
template<typename T>
ByteBuffer& ByteBuffer::operator<<(T obj)
{
    insert(end(), (uint8_t*)(&obj), (uint8_t*)(&obj) + sizeof(T));

    return *this;
}

/**
 * @}
 */

#endif

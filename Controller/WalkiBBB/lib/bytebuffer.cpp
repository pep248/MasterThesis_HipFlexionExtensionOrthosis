#include "bytebuffer.h"

/**
 * @brief ByteBuffer constructor.
 * @param reserveSize initial capacity of the vector. This is useful to prevent
 * future reallocation at a random time.
 */
ByteBuffer::ByteBuffer(int reserveSize)
{
    reserve(reserveSize);
}

/**
 * @brief Appends a string to the ByteBuffer.
 * @param str the string to append to the ByteBuffer. Only the string characters
 * (data()) will be added to the vector.
 * @return A reference to the ByteBuffer.
 */
ByteBuffer& ByteBuffer::operator<<(const std::string& str)
{
    insert(end(), (uint8_t*)str.c_str(), (uint8_t*)(str.c_str()+str.size()));

    return *this;
}

/**
 * @brief Appends a SyncVar value to the ByteBuffer.
 * @param sv the SyncVar to append. Only the data bytes of the variable pointed
 * by the SyncVar will be added to the vector.
 * @return A reference to the ByteBuffer.
 */
ByteBuffer& ByteBuffer::operator<<(SyncVar &sv)
{
    insert(end(), (uint8_t*)sv.getData(),
           ((uint8_t*)sv.getData()) + sv.getLength());

    return *this;
}

/**
 * @brief Appends a vector of bytes to the ByteBuffer.
 * @param data bytes vector to append. Only the bytes contained into the given
 * vector will be added to the ByteBuffer.
 * @return A reference to the ByteBuffer.
 */
ByteBuffer &ByteBuffer::operator<<(const vector<uint8_t> &data)
{
    insert(end(), data.begin(), data.end());

    return *this;
}

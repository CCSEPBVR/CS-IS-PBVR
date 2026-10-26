/*****************************************************************************/
/**
 *  @file   Surface.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/ValueArray>
#include <kvs/Type>


namespace kvs
{

namespace osmesa
{

class Surface
{
private:
    std::size_t m_width = 0; ///< width of image buffer
    std::size_t m_height = 0; ///< height of image buffer
    kvs::ValueArray<kvs::UInt8> m_buffer{}; ///< image buffer

public:
    Surface() = default;
    ~Surface() { this->destroy(); }

    void* handle() { return m_buffer.data(); }
    std::size_t width() const { return m_width; }
    std::size_t height() const { return m_height; }
    const kvs::ValueArray<kvs::UInt8>& buffer() const { return m_buffer; }

    bool create( const std::size_t width, const std::size_t height, const int format );
    void destroy();
};

} // end of namespace osmesa

} // end of namespace kvs

/****************************************************************************/
/**
 *  @file   Text.h
 *  @author Naohisa Sakamoto
 */
/****************************************************************************/
#pragma once
#include <string>
#include <vector>
#include <kvs/RGBColor>
#include <kvs/Deprecated>


namespace kvs
{

namespace glut
{

/*==========================================================================*/
/**
 *  Text drawing class.
 */
/*==========================================================================*/
class Text
{
protected:
    std::size_t m_x; ///< position x
    std::size_t m_y; ///< position y
    kvs::RGBColor m_color; ///< text color
    std::vector<std::string> m_text;  ///< text string

public:
    KVS_DEPRECATED( Text() );

    void setPosition( const std::size_t x, const std::size_t y );
    void setColor( const kvs::RGBColor& color );
    void setText( const char* text, ... );
    void setText( const std::string& text );
    void addText( const char* text, ... );
    void addText( const std::string& text );

    void draw();
};

} // end of namespace glut

} // end of namespace kvs

/****************************************************************************/
/**
 *  @file Text.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Text.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLUT__TEXT_H_INCLUDE
#define KVS__GLUT__TEXT_H_INCLUDE

#include <string>
#include <vector>
#include <kvs/ClassName>
#include <kvs/RGBColor>


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
    kvsClassName_without_virtual( kvs::glut::Text );

protected:

    size_t                   m_x;     ///< position x
    size_t                   m_y;     ///< position y
    kvs::RGBColor            m_color; ///< text color
    std::vector<std::string> m_text;  ///< text string

public:

    Text( void );

public:

    void setPosition( const size_t x, const size_t y );

    void setColor( const kvs::RGBColor& color );

    void setText( const char* text, ... );

    void setText( const std::string& text );

    void addText( const char* text, ... );

    void addText( const std::string& text );

public:

    void draw( void );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__TEXT_H_INCLUDE

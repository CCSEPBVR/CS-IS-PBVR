/*****************************************************************************/
/**
 *  @file   DisplayList.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DisplayList.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__DISPLAY_LIST_H_INCLUDE
#define VIS_MODULE__DISPLAY_LIST_H_INCLUDE

#include <vismodule/OpenGL>
#include <vismodule/ClassName>


namespace vismodule
{

class DisplayList
{
    visModuleClassName( vismodule::DisplayList );

protected:

    GLuint  m_id;    ///< display list ID
    GLsizei m_range; ///< number of lists

public:

    DisplayList( void );

    DisplayList( const GLsizei range );

    virtual ~DisplayList( void );

public:

    const GLuint id( void ) const;

    const GLsizei range( void ) const;

public:

    void begin( const GLuint index = 0, const GLenum mode = GL_COMPILE );

    void end( void );

    bool create( const GLsizei range );

    void clear( void );

    void render( const GLuint index = 0 );
};

} // end of namespace vismodule

#endif // VIS_MODULE__DISPLAY_LIST_H_INCLUDE

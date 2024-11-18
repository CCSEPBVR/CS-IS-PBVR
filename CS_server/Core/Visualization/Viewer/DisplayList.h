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
#ifndef KVS__DISPLAY_LIST_H_INCLUDE
#define KVS__DISPLAY_LIST_H_INCLUDE

#include <kvs/OpenGL>
#include <kvs/ClassName>


namespace kvs
{

class DisplayList
{
    kvsClassName( kvs::DisplayList );

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

} // end of namespace kvs

#endif // KVS__DISPLAY_LIST_H_INCLUDE

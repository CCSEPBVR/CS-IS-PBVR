/*****************************************************************************/
/**
 *  @file   Trackpad.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#ifndef KVS__OPENCABIN__TRACKPAD_H_INCLUDE
#define KVS__OPENCABIN__TRACKPAD_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/glut/Screen>
#include <kvs/opencabin/Application>


namespace kvs
{

namespace opencabin
{

class Trackpad : public kvs::glut::Screen
{
    kvsClassName_without_virtual( kvs::opencabin::Trackpad );

public:

    typedef kvs::glut::Screen BaseClass;

protected:

    kvs::Vector2f m_screen_scale;

public:

    Trackpad( kvs::opencabin::Application* application );

public:

    int show( void );

    void updateXform( void );

protected:

    void default_mouse_press_event( kvs::MouseEvent* event );

    void default_mouse_move_event( kvs::MouseEvent* event );

    void default_mouse_release_event( kvs::MouseEvent* event );
};

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__TRACKPAD_H_INCLUDE

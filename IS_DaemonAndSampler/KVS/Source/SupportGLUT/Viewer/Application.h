/*****************************************************************************/
/**
 *  @file   Application.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Application.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__APPLICATION_H_INCLUDE
#define KVS__GLUT__APPLICATION_H_INCLUDE

#include <kvs/ApplicationBase>
#include <kvs/ClassName>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  GLUT viewer application class.
 */
/*===========================================================================*/
class Application : public kvs::ApplicationBase
{
    kvsClassName( kvs::glut::Application );

public:

    Application( int argc, char** argv );

    virtual ~Application( void );

public:

    virtual int run( void );

    virtual void quit( void );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__APPLICATION_H_INCLUDE

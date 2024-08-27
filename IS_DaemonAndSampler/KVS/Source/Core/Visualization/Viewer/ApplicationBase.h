/*****************************************************************************/
/**
 *  @file   ApplicationBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ApplicationBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__APPLICATION_BASE_H_INCLUDE
#define KVS__APPLICATION_BASE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/ScreenBase>
#include <kvs/Compiler>
#include <list>

#if defined( KVS_COMPILER_VC )
#pragma warning(disable:4800)
#endif

namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  ApplicationBase class.
 */
/*===========================================================================*/
class ApplicationBase
{
    kvsClassName( kvs::ApplicationBase );

protected:

    int m_argc;    ///< argument count
    char** m_argv; ///< argument values

    std::list<kvs::ScreenBase*> m_screens; ///< list of the pointer to the screen

public:

    ApplicationBase( int argc, char** argv );

    virtual ~ApplicationBase( void );

public:

    int argc( void );

    char** argv( void );

    void attach( kvs::ScreenBase* screen );

    void detach( kvs::ScreenBase* screen );

    virtual int run( void ) = 0;

    virtual void quit( void ) = 0;
};

} // end of namespace kvs

#endif // KVS__APPLICATION_BASE_H_INCLUDE

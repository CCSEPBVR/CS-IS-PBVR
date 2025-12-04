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
#ifndef VIS_MODULE__APPLICATION_BASE_H_INCLUDE
#define VIS_MODULE__APPLICATION_BASE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ScreenBase>
#include <vismodule/Compiler>
#include <list>

#if defined( VIS_MODULE_COMPILER_VC )
#pragma warning(disable:4800)
#endif

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  ApplicationBase class.
 */
/*===========================================================================*/
class ApplicationBase
{
    visModuleClassName( vismodule::ApplicationBase );

protected:

    int m_argc;    ///< argument count
    char** m_argv; ///< argument values

    std::list<vismodule::ScreenBase*> m_screens; ///< list of the pointer to the screen

public:

    ApplicationBase( int argc, char** argv );

    virtual ~ApplicationBase( void );

public:

    int argc( void );

    char** argv( void );

    void attach( vismodule::ScreenBase* screen );

    void detach( vismodule::ScreenBase* screen );

    virtual int run( void ) = 0;

    virtual void quit( void ) = 0;
};

} // end of namespace vismodule

#endif // VIS_MODULE__APPLICATION_BASE_H_INCLUDE

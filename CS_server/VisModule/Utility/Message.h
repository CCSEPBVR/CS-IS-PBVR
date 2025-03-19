/****************************************************************************/
/**
 *  @file Message.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Message.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__MESSAGE_H_INCLUDE
#define VIS_MODULE__MESSAGE_H_INCLUDE

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <vismodule/ClassName>
#include <vismodule/Macro>
#include <vismodule/Platform>

// Error message
#define visModuleMessageError \
    ( vismodule::Message( vismodule::Message::Error, VIS_MODULE_MACRO_FILE, VIS_MODULE_MACRO_LINE, VIS_MODULE_MACRO_FUNC ) )

// Debug message
#if defined ( VIS_MODULE_ENABLE_DEBUG )
#define visModuleMessageDebug( message ) \
    ( vismodule::Message( vismodule::Message::Debug, VIS_MODULE_MACRO_FILE, VIS_MODULE_MACRO_LINE, VIS_MODULE_MACRO_FUNC ) ( message ) )
#else
#define visModuleMessageDebug( message )
#endif

// Warning message
#if defined ( VIS_MODULE_ENABLE_DEBUG )
#define visModuleMessageWarning( expression, message ) \
    ( vismodule::Message( vismodule::Message::Warning, VIS_MODULE_MACRO_FILE, VIS_MODULE_MACRO_LINE, VIS_MODULE_MACRO_FUNC, ( expression ) ) ( message ) )
#else
#define visModuleMessageWarning( expression, message )
#endif

// Assert message
#if defined ( VIS_MODULE_ENABLE_DEBUG )
#define visModuleMessageAssert( expression, message ) \
    ( vismodule::Message( vismodule::Message::Assert, VIS_MODULE_MACRO_FILE, VIS_MODULE_MACRO_LINE, VIS_MODULE_MACRO_FUNC, ( expression ) ) ( message ) )
#else
#define visModuleMessageAssert( expression, message )
#endif


#define VIS_MODULE_MESSAGE_RED    "31"
#define VIS_MODULE_MESSAGE_GREEN  "32"
#define VIS_MODULE_MESSAGE_BROWN  "33"
#define VIS_MODULE_MESSAGE_BLUE   "34"
#define VIS_MODULE_MESSAGE_PURPLE "35"
#define VIS_MODULE_MESSAGE_SKY    "36"
#define VIS_MODULE_MESSAGE_WHITE  "37"
#define VIS_MODULE_MESSAGE_NORMAL "0"


#if defined ( VIS_MODULE_PLATFORM_WINDOWS )
#define VIS_MODULE_MESSAGE_SET_COLOR( color ) ""
#define VIS_MODULE_MESSAGE_RESET_COLOR        ""
#else
#define VIS_MODULE_MESSAGE_CHANGE_COLOR_BEGIN
#define VIS_MODULE_MESSAGE_SET_COLOR( color ) "\x1b[" << color << "m"
#define VIS_MODULE_MESSAGE_RESET_COLOR        "\x1b[0m"
#endif


namespace vismodule
{

/*==========================================================================*/
/**
 *  Message class.
 */
/*==========================================================================*/
class Message
{
    visModuleClassName_without_virtual( vismodule::Message );

public:

    enum MessageType
    {
        Error = 0,
        Warning,
        Debug,
        Assert,
        NumberOfMessageTypes
    };

protected:

    MessageType m_type; ///< message type
    const char* m_file; ///< file name
    int         m_line; ///< line number
    const char* m_func; ///< function name
    bool        m_expr; ///< expression

public:

    Message( MessageType type, const char* file, int line, const char* func, bool expr = false );

    void operator ()( const char* msg, ... );
};

} // end of namespace vismodule

#endif // VIS_MODULE__MESSAGE_H_INCLUDE

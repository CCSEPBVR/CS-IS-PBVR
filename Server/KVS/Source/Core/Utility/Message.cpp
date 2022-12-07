/****************************************************************************/
/**
 *  @file Message.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Message.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Message.h"
#include <kvs/Breakpoint>


namespace
{

const size_t MaxMessageSize = 512;

const std::string tag[kvs::Message::NumberOfMessageTypes] =
{
    "KVS ERROR",
    "KVS WARNING",
    "KVS DEBUG",
    "KVS ASSERT"
};

const std::string color[kvs::Message::NumberOfMessageTypes] =
{
    KVS_MESSAGE_RED,
    KVS_MESSAGE_PURPLE,
    KVS_MESSAGE_BLUE,
    KVS_MESSAGE_BROWN
};

}


namespace kvs
{

/*==========================================================================*/
/**
 *  Constuctor.
 *
 *  @param type [in] message type
 *  @param file [in] file name
 *  @param line [in] line number
 *  @param func [in] function name
 *  @param expr [in] expression
 */
/*==========================================================================*/
Message::Message( MessageType type, const char* file, int line, const char* func, bool expr )
    : m_type( type )
    , m_file( file )
    , m_line( line )
    , m_func( func )
    , m_expr( expr )
{
}

/*==========================================================================*/
/**
 *  Operator '()'.
 *
 *  @param msg [in] message
 */
/*==========================================================================*/
void Message::operator ()( const char* msg, ... )
{
    if ( !m_expr )
    {
        char buffer[ ::MaxMessageSize ];

        va_list args;
        va_start( args, msg );
        vsprintf( buffer, msg, args );
        va_end( args );

        // Output message tag.
        std::cerr << KVS_MESSAGE_SET_COLOR( ::color[m_type] );
        std::cerr << ::tag[m_type];
        std::cerr << KVS_MESSAGE_RESET_COLOR;

        // Output message.
        std::cerr << ": " << std::string( buffer ) << std::endl;
        std::cerr << "\t" << "FILE: " << std::string( m_file ) << " (" << m_line << ")" << std::endl;
        std::cerr << "\t" << "FUNC: " << std::string( m_func ) << std::endl;

        if ( m_type == Message::Assert ) { KVS_BREAKPOINT; }
    }
}

} // end of namespace kvs

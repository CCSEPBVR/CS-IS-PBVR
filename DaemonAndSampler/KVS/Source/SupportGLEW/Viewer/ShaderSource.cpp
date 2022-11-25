/*****************************************************************************/
/**
 *  @file   ShaderSource.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ShaderSource.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ShaderSource.h"
#include <kvs/File>
#include <kvs/Message>
#include <sstream>
#include <fstream>


namespace kvs
{

namespace glew
{

ShaderSource::ShaderSource( void )
{
}

ShaderSource::ShaderSource( const std::string& source )
{
    const kvs::File file( source );
    if ( file.isExisted() )
    {
        const std::string filename( source );
        this->read( filename );
    }
    else
    {
        const std::string code( source );
        this->setCode( code );
    }
}

ShaderSource::~ShaderSource( void )
{
}

const std::string& ShaderSource::code( void ) const
{
    return( m_code );
}

void ShaderSource::setCode( const std::string& code )
{
    m_code = code;
}

void ShaderSource::clearCode( void )
{
    m_code.erase();
}

void ShaderSource::define( const std::string& name )
{
    const std::string define = "#define " + name + "\n";
    m_code = define + m_code;
}

const bool ShaderSource::read( const std::string& filename )
{
    std::ifstream ifs( filename.c_str() );
    if ( ifs.fail() )
    {
        kvsMessageError( "Cannot open '%s'.", filename.c_str() );
        return( false );
    }

    std::ostringstream buffer;
    buffer << ifs.rdbuf();

    std::string code( buffer.str() );
    ifs.close();

    this->setCode( code );

    return( true );
}

} // end of namespace glew

} // end of namespace kvs

/****************************************************************************/
/**
 *  @file OpenGLChecker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpenGLChecker.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSCHECK__OPENGL_CHECKER_H_INCLUDE
#define KVSCHECK__OPENGL_CHECKER_H_INCLUDE

#include <string>


namespace kvscheck
{

/*===========================================================================*/
/**
 *  @brief  OpenGL information checker class.
 */
/*===========================================================================*/
class OpenGLChecker
{
protected:

    std::string m_vendor;       ///< vendor name
    std::string m_renderer;     ///< renderer (GPU) name
    std::string m_gl_version;   ///< OpenGL version
    std::string m_glsl_version; ///< GLSL version
    std::string m_glu_version;  ///< GLU version

public:

    OpenGLChecker( int argc, char** argv );

public:

    const std::string& vendor( void ) const;

    const std::string& renderer( void ) const;

    const std::string& GLVersion( void ) const;

    const std::string& GLUVersion( void ) const;

    const std::string& GLSLVersion( void ) const;

    friend std::ostream& operator << ( std::ostream& os, const OpenGLChecker& checker );
};

} // end of namespace kvscheck

#endif // KVSCHECK__OPENGL_CHECKER_H_INCLUDE

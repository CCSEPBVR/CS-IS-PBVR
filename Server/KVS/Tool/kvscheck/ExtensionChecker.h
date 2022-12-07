/****************************************************************************/
/**
 *  @file ExtensionChecker.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/****************************************************************************/
#ifndef KVSCHECK__EXTENSION_CHECKER_H_INCLUDE
#define KVSCHECK__EXTENSION_CHECKER_H_INCLUDE

#include <string>
#include <kvs/StringList>


namespace kvscheck
{

/*===========================================================================*/
/**
 *  @brief  OpenGL extension information checker class.
 */
/*===========================================================================*/
class ExtensionChecker
{
protected:

    kvs::StringList m_gl_extensions;  ///< OpenGL extensions
    kvs::StringList m_glu_extensions; ///< GLU extensions

public:

    ExtensionChecker( int argc, char** argv );

public:

    const kvs::StringList& GLExtensions( void ) const;

    const kvs::StringList& GLUExtensions( void ) const;

    friend std::ostream& operator << ( std::ostream& os, const ExtensionChecker& checker );
};

} // end of namespace kvscheck

#endif // KVSCHECK__EXTENSION_CHECKER_H_INCLUDE

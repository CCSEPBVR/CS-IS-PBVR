/****************************************************************************/
/**
 *  @file WriteMakefile.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteMakefile.h 626 2010-10-01 07:45:20Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSMAKE__WRITE_MAKEFILE_H_INCLUDE
#define KVSMAKE__WRITE_MAKEFILE_H_INCLUDE

#include <string>  // For std::string.
#include <fstream> // For std::ifstream, std::ofstream.


namespace kvsmake
{

void WriteMakefile( const std::string& project_name );

void WriteMakefileBody(
    std::ifstream&     in,
    std::ofstream&     out,
    const std::string& project_name );

} // end of namespace kvsmake

#endif // KVSMAKE__WRITE_MAKEFILE_H_INCLUDE

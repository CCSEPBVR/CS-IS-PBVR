/****************************************************************************/
/**
 *  @file WriteVCProject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: WriteVCXProject.h 659 2011-01-15 03:55:08Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSMAKE__WRITE_VCX_PROJECT_H_INCLUDE
#define KVSMAKE__WRITE_VCX_PROJECT_H_INCLUDE

#include <string>  // For std::string.
#include <fstream> // For std::ifstream, std::ofstream.


namespace kvsmake
{

void WriteVCXProject( const std::string& project_name );

void WriteVCXProjectBody(
    std::ifstream&     in,
    std::ofstream&     out,
    const std::string& project_name );

} // end of namespace kvsmake

#endif // KVSMAKE__WRITE_VCX_PROJECT_H_INCLUDE

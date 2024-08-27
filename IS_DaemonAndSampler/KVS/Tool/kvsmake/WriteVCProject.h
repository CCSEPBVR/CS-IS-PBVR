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
 *  $Id: WriteVCProject.h 626 2010-10-01 07:45:20Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVSMAKE__WRITE_VC_PROJECT_H_INCLUDE
#define KVSMAKE__WRITE_VC_PROJECT_H_INCLUDE

#include <string>  // For std::string.
#include <fstream> // For std::ifstream, std::ofstream.


namespace kvsmake
{

void WriteVCProject( const std::string& project_name );

void WriteVCProjectBody(
    std::ifstream&     in,
    std::ofstream&     out,
    const std::string& project_name );

} // end of namespace kvsmake

#endif // KVSMAKE__WRITE_VC_PROJECT_H_INCLUDE

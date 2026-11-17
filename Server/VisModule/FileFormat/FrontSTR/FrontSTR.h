/*****************************************************************************/
/**
 *  @file   FrontSTR.h
 *  @author Guo Jiazhen
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: FrontSTR.h 870 2011-07-27 00:53:17Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__FRONT_STR_H_INCLUDE
#define VIS_MODULE__FRONT_STR_H_INCLUDE

#include <string>
#include <fstream>
#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>
#include <vismodule/Type>
#include <vismodule/ValueArray>
#include "MeshData.h"
#include "ResultData.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  FrontSTR class.
 */
/*===========================================================================*/
class FrontSTR : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::FrontSTR );

protected:

    std::size_t m_nmeshes; ///< number of mesh data
    std::size_t m_nresults; ///< number of result data
    vismodule::fstr::MeshData* m_mesh_data; ///< mesh data
    vismodule::fstr::ResultData* m_result_data; ///< result data

public:

    FrontSTR( void );

    FrontSTR( const std::string& filenames );

    FrontSTR( const std::string& msh_filename, const std::string& res_filename );

    FrontSTR( const std::vector<std::string>& msh_filenames, const std::vector<std::string>& res_filenames );

    virtual ~FrontSTR( void );

public:

    const std::size_t numberOfMeshData( void ) const;

    const std::size_t numberOfResultData( void ) const;

    const vismodule::fstr::MeshData& meshData( const std::size_t index = 0 ) const;

    const vismodule::fstr::ResultData& resultData( const std::size_t index = 0 ) const;

public:

    const bool read( const std::string& filenames );

    const bool read( const std::string& msh_filename, const std::string& res_filename );

    const bool read( const std::vector<std::string>& msh_filenames, const std::vector<std::string>& res_filenames );

private:

    const bool allocate_data( void );

    void delete_data( void );

private:

    const bool write( const std::string& filename );
};

} // end of namespace vismodule

#endif // VIS_MODULE__FRONT_STR_H_INCLUDE

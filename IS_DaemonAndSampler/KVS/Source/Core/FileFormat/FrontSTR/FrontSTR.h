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
#ifndef KVS__FRONT_STR_H_INCLUDE
#define KVS__FRONT_STR_H_INCLUDE

#include <string>
#include <fstream>
#include <kvs/FileFormatBase>
#include <kvs/ClassName>
#include <kvs/Type>
#include <kvs/ValueArray>
#include "MeshData.h"
#include "ResultData.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  FrontSTR class.
 */
/*===========================================================================*/
class FrontSTR : public kvs::FileFormatBase
{
    kvsClassName( kvs::FrontSTR );

protected:

    size_t m_nmeshes; ///< number of mesh data
    size_t m_nresults; ///< number of result data
    kvs::fstr::MeshData* m_mesh_data; ///< mesh data
    kvs::fstr::ResultData* m_result_data; ///< result data

public:

    FrontSTR( void );

    FrontSTR( const std::string& filenames );

    FrontSTR( const std::string& msh_filename, const std::string& res_filename );

    FrontSTR( const std::vector<std::string>& msh_filenames, const std::vector<std::string>& res_filenames );

    virtual ~FrontSTR( void );

public:

    const size_t numberOfMeshData( void ) const;

    const size_t numberOfResultData( void ) const;

    const kvs::fstr::MeshData& meshData( const size_t index = 0 ) const;

    const kvs::fstr::ResultData& resultData( const size_t index = 0 ) const;

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

} // end of namespace kvs

#endif // KVS__FRONT_STR_H_INCLUDE

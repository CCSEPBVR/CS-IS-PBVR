/*****************************************************************************/
/**
 *  @file   Ply.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Ply.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__PLY_H_INCLUDE
#define KVS__PLY_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>
#include <kvs/FileFormatBase>
#include <kvs/Vector3>
#include <kvs/ValueArray>
#include <kvs/ClassName>
#include <kvs/Platform>
#include "PlyFile.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  PLY class.
 */
/*===========================================================================*/
class Ply : public kvs::FileFormatBase
{
    kvsClassName( kvs::Ply );

public:

    enum FileType
    {
        Ascii = PLY_ASCII, // ascii type
#if defined( KVS_PLATFORM_LITTLE_ENDIAN )
        Binary = PLY_BINARY_LE // binary type (little endian)
#else
        Binary = PLY_BINARY_BE // binary type (big endian)
#endif
    };

protected:

    FileType           m_file_type; ///< file type
    size_t             m_nverts; ///< number of vertices
    size_t             m_nfaces; ///< number of faces
    kvs::ValueArray<kvs::Real32> m_coords; ///< coordinate value array
    kvs::ValueArray<kvs::UInt8>  m_colors; ///< color value array
    kvs::ValueArray<kvs::Real32> m_normals; ///< normal value array
    kvs::ValueArray<kvs::UInt32> m_connections; ///< connection array
    bool               m_has_connections; ///< true, if the connections is included
    bool               m_has_colors; ///< true, if the colors is included
    bool               m_has_normals; ///< true, if the normals is included
    kvs::Vector3f      m_min_coord; ///< min. coordinate
    kvs::Vector3f      m_max_coord; ///< max. coordinate

public:

    Ply( void );

    Ply( const std::string& filename );

    virtual ~Ply( void );

public:

    void initialize( void );

    void calculateNormals( void );

    void calculateMinMaxCoord( void );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator << ( std::ostream& os, const Ply& ply );

public:

    const FileType fileType( void ) const;

    const size_t numberOfVertices( void ) const;

    const size_t numberOfFaces( void ) const;

    const bool hasConnections( void ) const;

    const bool hasColors( void ) const;

    const bool hasNormals( void ) const;

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::Real32>& normals( void ) const;

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

    const kvs::Vector3f& minCoord( void ) const;

    const kvs::Vector3f& maxCoord( void ) const;

public:

    void setFileType( const FileType file_type );

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setNormals( const kvs::ValueArray<kvs::Real32>& normals );

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );
};

} // end of namespace kvs

#endif // KVS__PLY_H_INCLUDE

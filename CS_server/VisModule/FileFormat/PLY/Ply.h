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
#ifndef VIS_MODULE__PLY_H_INCLUDE
#define VIS_MODULE__PLY_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>
#include <vismodule/FileFormatBase>
#include <vismodule/Vector3>
#include <vismodule/ValueArray>
#include <vismodule/ClassName>
#include <vismodule/Platform>
#include "PlyFile.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  PLY class.
 */
/*===========================================================================*/
class Ply : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::Ply );

public:

    enum FileType
    {
        Ascii = PLY_ASCII, // ascii type
#if defined( VIS_MODULE_PLATFORM_LITTLE_ENDIAN )
        Binary = PLY_BINARY_LE // binary type (little endian)
#else
        Binary = PLY_BINARY_BE // binary type (big endian)
#endif
    };

protected:

    FileType           m_file_type; ///< file type
    size_t             m_nverts; ///< number of vertices
    size_t             m_nfaces; ///< number of faces
    vismodule::ValueArray<vismodule::Real32> m_coords; ///< coordinate value array
    vismodule::ValueArray<vismodule::UInt8>  m_colors; ///< color value array
    vismodule::ValueArray<vismodule::Real32> m_normals; ///< normal value array
    vismodule::ValueArray<vismodule::UInt32> m_connections; ///< connection array
    bool               m_has_connections; ///< true, if the connections is included
    bool               m_has_colors; ///< true, if the colors is included
    bool               m_has_normals; ///< true, if the normals is included
    vismodule::Vector3f      m_min_coord; ///< min. coordinate
    vismodule::Vector3f      m_max_coord; ///< max. coordinate

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

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& colors( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& normals( void ) const;

    const vismodule::ValueArray<vismodule::UInt32>& connections( void ) const;

    const vismodule::Vector3f& minCoord( void ) const;

    const vismodule::Vector3f& maxCoord( void ) const;

public:

    void setFileType( const FileType file_type );

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setNormals( const vismodule::ValueArray<vismodule::Real32>& normals );

    void setConnections( const vismodule::ValueArray<vismodule::UInt32>& connections );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );
};

} // end of namespace vismodule

#endif // VIS_MODULE__PLY_H_INCLUDE

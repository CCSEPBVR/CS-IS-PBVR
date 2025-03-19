/*****************************************************************************/
/**
 *  @file   Stl.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Stl.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__STL_H_INCLUDE
#define VIS_MODULE__STL_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>
#include <vismodule/ValueArray>
#include <vismodule/FileFormatBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  STL file format class.
 */
/*===========================================================================*/
class Stl : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::Stl );

public:

    typedef vismodule::FileFormatBase BaseClass;

    enum FileType
    {
        Ascii, // ascii type
        Binary // binary type
    };

protected:

    FileType m_file_type; ///< file type
    vismodule::ValueArray<vismodule::Real32> m_normals; /// normal vector array
    vismodule::ValueArray<vismodule::Real32> m_coords; /// coordinate value array

public:

    Stl( void );

    Stl( const std::string& filename );

    virtual ~Stl( void );

public:

    const FileType fileType( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& normals( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const size_t ntriangles( void ) const;

public:

    void setFileType( const FileType file_type );

    void setNormals( const vismodule::ValueArray<vismodule::Real32>& normals );

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    friend std::ostream& operator << ( std::ostream& os, const Stl& stl );

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

private:

    const bool is_ascii_type( FILE* ifs );

    const bool read_ascii( FILE* ifs );

    const bool read_binary( FILE* ifs );

    const bool write_ascii( FILE* ifs );

    const bool write_binary( FILE* ifs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STL_H_INCLUDE

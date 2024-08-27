/****************************************************************************/
/**
 *  @file UnstructuredVolumeImporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeImporter.cpp 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "UnstructuredVolumeImporter.h"
#include <kvs/DebugNew>
#include <kvs/AVSUcd>
#include <kvs/Message>
#include <kvs/Vector3>
#include "endian2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace
{

/*==========================================================================*/
/**
 *  @brief  Converts to the cell type from the given string.
 *  @param  cell_type [in] grid type string
 *  @return cell type
 */
/*==========================================================================*/
const pbvr::UnstructuredVolumeObject::CellType StringToCellType( const std::string& cell_type )
{
    if (      cell_type == "tetrahedra" )
    {
        return pbvr::UnstructuredVolumeObject::Tetrahedra;
    }
    else if ( cell_type == "quadratic tetrahedra" )
    {
        return pbvr::UnstructuredVolumeObject::QuadraticTetrahedra;
    }
    else if ( cell_type == "hexahedra"  )
    {
        return pbvr::UnstructuredVolumeObject::Hexahedra;
    }
    else if ( cell_type == "quadratic hexahedra"  )
    {
        return pbvr::UnstructuredVolumeObject::QuadraticHexahedra;
    }
    else if ( cell_type == "prism"  )
    {
        return pbvr::UnstructuredVolumeObject::Prism;
    }
    else if ( cell_type == "pyramid"  )
    {
        return pbvr::UnstructuredVolumeObject::Pyramid;
    }
    else if ( cell_type == "point"  )
    {
        return pbvr::UnstructuredVolumeObject::Point;
    }
    else if ( cell_type == "triangle"  )
    {
        return pbvr::UnstructuredVolumeObject::Triangle;
    }
    else if ( cell_type == "triangle2"  )
    {
        return pbvr::UnstructuredVolumeObject::QuadraticTriangle;
    }
    else if ( cell_type == "quadratic"  )
    {
        return pbvr::UnstructuredVolumeObject::Square;
    }
    else if ( cell_type == "quadratic2"  )
    {
        return pbvr::UnstructuredVolumeObject::QuadraticSquare;
    }
    else
    {
        kvsMessageError( "Unknown cell type '%s'.", cell_type.c_str() );
        return pbvr::UnstructuredVolumeObject::UnknownCellType;
    }
}

/*==========================================================================*/
/**
 *  @brief  Converts to the cell type from the given element type.
 *  @param  element_type [in] element type
 *  @return cell type
 */
/*==========================================================================*/
const pbvr::UnstructuredVolumeObject::CellType ElementTypeToCellType(
    const kvs::AVSUcd::ElementType& element_type )
{
    if ( element_type == kvs::AVSUcd::Tetrahedra  )
    {
        return pbvr::UnstructuredVolumeObject::Tetrahedra;
    }
    else if ( element_type == kvs::AVSUcd::Tetrahedra2 )
    {
        return pbvr::UnstructuredVolumeObject::QuadraticTetrahedra;
    }
    else if ( element_type == kvs::AVSUcd::Hexahedra )
    {
        return pbvr::UnstructuredVolumeObject::Hexahedra;
    }
    else if ( element_type == kvs::AVSUcd::Hexahedra2 )
    {
        return pbvr::UnstructuredVolumeObject::QuadraticHexahedra;
    }
    else
    {
        kvsMessageError( "Unknown element type." );
        return pbvr::UnstructuredVolumeObject::UnknownCellType;
    }
}

} // end of namespace


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVolumeImporter class.
 */
/*===========================================================================*/
UnstructuredVolumeImporter::UnstructuredVolumeImporter()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVolumeImporter class.
 *  @param  filename [in] input filename
 */
/*===========================================================================*/
UnstructuredVolumeImporter::UnstructuredVolumeImporter( const std::string& filename )
{
    if ( pbvr::SPLITTypeSubvolume::CheckFileExtension( filename ) )
    {
        pbvr::SPLITTypeSubvolume* file_format = new pbvr::SPLITTypeSubvolume( filename );
        if ( !file_format )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s'.", filename.c_str() );
            delete file_format;
            return;
        }

        this->import( *file_format );
        delete file_format;
    }
    else if ( kvs::AVSUcd::CheckFileExtension( filename ) )
    {
        kvs::AVSUcd* file_format = new kvs::AVSUcd( filename );
        if ( !file_format )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s'.", filename.c_str() );
            delete file_format;
            return;
        }

        this->import( *file_format );
        delete file_format;
    }
    /* 131017 removed
        else if ( kvs::AVSField::CheckFileExtension( filename ) )
        {
            kvs::AVSField* file_format = new kvs::AVSField( filename );
            if( !file_format )
            {
                BaseClass::m_is_success = false;
                kvsMessageError("Cannot read '%s'.",filename.c_str());
                return;
            }

            if( file_format->isFailure() )
            {
                BaseClass::m_is_success = false;
                kvsMessageError("Cannot read '%s'.",filename.c_str());
                delete file_format;
                return;
            }

            this->import( file_format );
            delete file_format;
        }
    */
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Cannot import '%s'.", filename.c_str() );
        return;
    }
}

UnstructuredVolumeImporter::UnstructuredVolumeImporter( const std::string& filename,
                                                        const int fileType, const int st, const int vl )
{
    if ( fileType == 1 )  // Gathered Subvolume file
    {
        kvs::AggregateTypeSubvolume* file_format =
            new kvs::AggregateTypeSubvolume( filename, st, vl );
        if ( !file_format )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s*'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s*'.", filename.c_str() );
            delete file_format;
            return;
        }
        this->import( *file_format );
        delete file_format;

    }
    else if ( fileType == 2 ) // Gathered Timestep file
    {
        kvs::StepAggregateTypeSubvolume* file_format =
            new kvs::StepAggregateTypeSubvolume( filename, st, vl );
        if ( !file_format )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s*'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            kvsMessageError( "Cannot read '%s*'.", filename.c_str() );
            delete file_format;
            return;
        }
        this->import( *file_format );
        delete file_format;

    }
    else                          // Unsupported filetype
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported fileType '%d'.", fileType );
    }

    return;
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVolumeImporter class.
 *  @param  file_format [in] pointer to the file format data
 */
/*==========================================================================*/
UnstructuredVolumeImporter::UnstructuredVolumeImporter( const kvs::FileFormatBase& file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the UnstructuredVolumeImporter class.
 */
/*===========================================================================*/
UnstructuredVolumeImporter::~UnstructuredVolumeImporter()
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the import process.
 *  @param  file_format [in] pointer to the file format data
 *  @return pointer to the imported unstructured volume object
 */
/*===========================================================================*/
UnstructuredVolumeImporter::SuperClass* UnstructuredVolumeImporter::exec( const kvs::FileFormatBase& file_format )
{
    if ( !&file_format )
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Input file format is NULL." );
        return NULL;
    }

    const std::string class_name = file_format.className();
    if ( class_name == "pbvr::SPLITTypeSubvolume" )
    {
        const pbvr::SPLITTypeSubvolume* fformat = static_cast<const pbvr::SPLITTypeSubvolume*>( &file_format );
        this->import( *fformat );
    }
    else if ( class_name == "kvs::AVSUcd" )
    {
        const kvs::AVSUcd* fformat = static_cast<const kvs::AVSUcd*>( &file_format );
        this->import( *fformat );
    }
    /* 131017 removed
        else if ( class_name == "kvs::AVSField" )
        {
            this->import( static_cast<const kvs::AVSField*>( file_format ) );
        }
    */
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Input file format is not supported." );
        return NULL;
    }

    return this;
}

/*==========================================================================*/
/**
 *  @brief  Imports a KVSML format data.
 *  @param  kvsml [in] pointer to the KVSML format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const pbvr::SPLITTypeSubvolume& kvsml )
{
    if ( kvsml.objectTag().hasExternalCoord() )
    {
        const kvs::Vector3f min_coord( kvsml.objectTag().minExternalCoord() );
        const kvs::Vector3f max_coord( kvsml.objectTag().maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( kvsml.objectTag().hasObjectCoord() )
    {
        const kvs::Vector3f min_coord( kvsml.objectTag().minObjectCoord() );
        const kvs::Vector3f max_coord( kvsml.objectTag().maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    SuperClass::setVeclen( kvsml.veclen() );
    SuperClass::setNNodes( kvsml.nnodes() );
    SuperClass::setNCells( kvsml.ncells() );
    SuperClass::setCellType( ::StringToCellType( kvsml.cellType() ) );
    SuperClass::setCoords( kvsml.coords() );
    SuperClass::setConnections( kvsml.connections() );
    SuperClass::setValues( kvsml.values() );
    SuperClass::updateMinMaxCoords();

    if ( kvsml.hasMinValue() && kvsml.hasMaxValue() )
    {
        const double min_value = kvsml.minValue();
        const double max_value = kvsml.maxValue();
        SuperClass::setMinMaxValues( min_value, max_value );
    }
    else
    {
        SuperClass::updateMinMaxValues();
        const double min_value = kvsml.hasMinValue() ? kvsml.minValue() : SuperClass::minValue();
        const double max_value = kvsml.hasMaxValue() ? kvsml.maxValue() : SuperClass::maxValue();
        SuperClass::setMinMaxValues( min_value, max_value );
    }
}

/*==========================================================================*/
/**
 *  @brief  Imports the AVS UCD format data.
 *  @param  ucd [in] pointer to the AVS UCD format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::AVSUcd& ucd )
{
    SuperClass::setVeclen( ucd.veclens()[ ucd.componentID() ] );
    SuperClass::setNNodes( ucd.nnodes() );
    SuperClass::setNCells( ucd.nelements() );
    SuperClass::setCellType( ::ElementTypeToCellType( ucd.elementType() ) );
    SuperClass::setCoords( ucd.coords() );
    SuperClass::setConnections( ucd.connections() );
    SuperClass::setValues( kvs::AnyValueArray( ucd.values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

/*===========================================================================*/
/**
 *  @brief  Imports the AVS Filed format data.
 *  @param  field [in] pointer to the AVS Filed format data
 */
/*===========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::AVSField& field )
{
    if ( field.fieldType() != kvs::AVSField::Irregular )
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Cannot import uniform/rectilinear type AVS field data." );
        return;
    }

    const size_t line_size  = field.dim().x();
    const size_t slice_size = field.dim().y();
    const kvs::Vector3ui ncells( field.dim() - kvs::Vector3ui( 1, 1, 1 ) );
    SuperClass::Connections connections( ncells.x() * ncells.y() * ncells.z() * 8 );

    size_t vertex_index = 0;
    size_t connection_index = 0;

    for ( size_t z = 0; z < ncells.z(); ++z )
    {
        for ( size_t y = 0; y < ncells.y(); ++y )
        {
            for ( size_t x = 0; x < ncells.x(); ++x )
            {
                const size_t local_vertex_index[8] =
                {
                    vertex_index,
                    vertex_index + 1,
                    vertex_index + line_size,
                    vertex_index + line_size + 1,
                    vertex_index + slice_size,
                    vertex_index + slice_size + 1,
                    vertex_index + slice_size + line_size,
                    vertex_index + slice_size + line_size + 1
                };

                // hexahedra-1
                connections[ connection_index++ ] = local_vertex_index[ 0 ];
                connections[ connection_index++ ] = local_vertex_index[ 1 ];
                connections[ connection_index++ ] = local_vertex_index[ 3 ];
                connections[ connection_index++ ] = local_vertex_index[ 2 ];
                connections[ connection_index++ ] = local_vertex_index[ 4 ];
                connections[ connection_index++ ] = local_vertex_index[ 5 ];
                connections[ connection_index++ ] = local_vertex_index[ 7 ];
                connections[ connection_index++ ] = local_vertex_index[ 6 ];

                vertex_index++;
            }
            vertex_index++;
        }
        vertex_index += line_size;
    }

    SuperClass::setVeclen( field.veclen() );
    //SuperClass::setNNodes( field.values().size() ); /* 131017 removed */
    SuperClass::setNCells( ncells.x() * ncells.y() * ncells.z() );
    SuperClass::setCellType( Hexahedra );
    SuperClass::setCoords( field.coords() );
    SuperClass::setConnections( connections );
    //SuperClass::setValues( field.values() ); /* 131017 removed */
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

/*==========================================================================*/
/**
 *  @brief  Imports the Gathered Subvolume format data.
 *  @param  gs [in] pointer to the Gathered Subvolume format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::AggregateTypeSubvolume& gs )
{
    SuperClass::setVeclen( gs.veclen() );
    SuperClass::setNNodes( gs.nnodes() );
    SuperClass::setNCells( gs.ncells() );
    // printf(" gs: %d %d %d\n", fgs.veclen(), fgs.nnodes(), fgs.ncells());
    SuperClass::setCellType( ::StringToCellType( gs.cellType() ) );
    SuperClass::setCoords( gs.coords() );
    SuperClass::setConnections( gs.connections() );
    SuperClass::setValues( kvs::AnyValueArray( gs.values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

/*==========================================================================*/
/**
 *  @brief  Imports the Gathered Timestep format data.
 *  @param  gs [in] pointer to the Gathered Subvolume format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::StepAggregateTypeSubvolume& gt )
{
    SuperClass::setVeclen( gt.veclen() );
    SuperClass::setNNodes( gt.nnodes() );
    SuperClass::setNCells( gt.ncells() );
    // printf(" gt: %d %d %d\n", gt.veclen(), gt.nnodes(), gt.ncells());
    SuperClass::setCellType( ::StringToCellType( gt.cellType() ) );
    SuperClass::setCoords( gt.coords() );
    SuperClass::setConnections( gt.connections() );
    SuperClass::setValues( kvs::AnyValueArray( gt.values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

} // end of namespace pbvr

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


namespace
{

/*==========================================================================*/
/**
 *  @brief  Converts to the cell type from the given string.
 *  @param  cell_type [in] grid type string
 *  @return cell type
 */
/*==========================================================================*/
const kvs::UnstructuredVolumeObject::CellType StringToCellType( const std::string& cell_type )
{
    if (      cell_type == "tetrahedra" ) { return( kvs::UnstructuredVolumeObject::Tetrahedra ); }
    else if ( cell_type == "quadratic tetrahedra" ) { return( kvs::UnstructuredVolumeObject::QuadraticTetrahedra ); }
    else if ( cell_type == "hexahedra"  ) { return( kvs::UnstructuredVolumeObject::Hexahedra );  }
    else if ( cell_type == "quadratic hexahedra"  ) { return( kvs::UnstructuredVolumeObject::QuadraticHexahedra );  }
    else if ( cell_type == "pyramid"  ) { return( kvs::UnstructuredVolumeObject::Pyramid );  }
    else if ( cell_type == "point"  ) { return( kvs::UnstructuredVolumeObject::Point );  }
    else
    {
        kvsMessageError( "Unknown cell type '%s'.", cell_type.c_str() );
        return( kvs::UnstructuredVolumeObject::UnknownCellType );
    }
}

/*==========================================================================*/
/**
 *  @brief  Converts to the cell type from the given element type.
 *  @param  element_type [in] element type
 *  @return cell type
 */
/*==========================================================================*/
const kvs::UnstructuredVolumeObject::CellType ElementTypeToCellType(
    const kvs::AVSUcd::ElementType element_type )
{
    if ( element_type == kvs::AVSUcd::Tetrahedra  )
    {
        return( kvs::UnstructuredVolumeObject::Tetrahedra );
    }
    else if ( element_type == kvs::AVSUcd::Tetrahedra2 )
    {
        return( kvs::UnstructuredVolumeObject::QuadraticTetrahedra );
    }
    else if ( element_type == kvs::AVSUcd::Hexahedra )
    {
        return( kvs::UnstructuredVolumeObject::Hexahedra );
    }
    else if ( element_type == kvs::AVSUcd::Hexahedra2 )
    {
        return( kvs::UnstructuredVolumeObject::QuadraticHexahedra );
    }
    else
    {
        kvsMessageError( "Unknown element type." );
        return( kvs::UnstructuredVolumeObject::UnknownCellType );
    }
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVolumeImporter class.
 */
/*===========================================================================*/
UnstructuredVolumeImporter::UnstructuredVolumeImporter( void )
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
    if ( kvs::KVSMLObjectUnstructuredVolume::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectUnstructuredVolume* file_format = new kvs::KVSMLObjectUnstructuredVolume( filename );
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
    else if ( kvs::AVSUcd::CheckFileExtension( filename ) )
    {
        kvs::AVSUcd* file_format = new kvs::AVSUcd( filename );
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
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Cannot import '%s'.",filename.c_str());
        return;
    }
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVolumeImporter class.
 *  @param  file_format [in] pointer to the file format data
 */
/*==========================================================================*/
UnstructuredVolumeImporter::UnstructuredVolumeImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the UnstructuredVolumeImporter class.
 */
/*===========================================================================*/
UnstructuredVolumeImporter::~UnstructuredVolumeImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the import process.
 *  @param  file_format [in] pointer to the file format data
 *  @return pointer to the imported unstructured volume object
 */
/*===========================================================================*/
UnstructuredVolumeImporter::SuperClass* UnstructuredVolumeImporter::exec( const kvs::FileFormatBase* file_format )
{
    if ( !file_format )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name = file_format->className();
    if ( class_name == "kvs::KVSMLObjectUnstructuredVolume" )
    {
        this->import( static_cast<const kvs::KVSMLObjectUnstructuredVolume*>( file_format ) );
    }
    else if ( class_name == "kvs::AVSUcd" )
    {
        this->import( static_cast<const kvs::AVSUcd*>( file_format ) );
    }
    else if ( class_name == "kvs::AVSField" )
    {
        this->import( static_cast<const kvs::AVSField*>( file_format ) );
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is not supported.");
        return( NULL );
    }

    return( this );
}

/*==========================================================================*/
/**
 *  @brief  Imports a KVSML format data.
 *  @param  kvsml [in] pointer to the KVSML format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::KVSMLObjectUnstructuredVolume* kvsml )
{
    if ( kvsml->objectTag().hasExternalCoord() )
    {
        const kvs::Vector3f min_coord( kvsml->objectTag().minExternalCoord() );
        const kvs::Vector3f max_coord( kvsml->objectTag().maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( kvsml->objectTag().hasObjectCoord() )
    {
        const kvs::Vector3f min_coord( kvsml->objectTag().minObjectCoord() );
        const kvs::Vector3f max_coord( kvsml->objectTag().maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    SuperClass::setVeclen( kvsml->veclen() );
    SuperClass::setNNodes( kvsml->nnodes() );
    SuperClass::setNCells( kvsml->ncells() );
    SuperClass::setCellType( ::StringToCellType( kvsml->cellType() ) );
    SuperClass::setCoords( kvsml->coords() );
    SuperClass::setConnections( kvsml->connections() );
    SuperClass::setValues( kvsml->values() );
    SuperClass::updateMinMaxCoords();

    if ( kvsml->hasMinValue() && kvsml->hasMaxValue() )
    {
        const double min_value = kvsml->minValue();
        const double max_value = kvsml->maxValue();
        SuperClass::setMinMaxValues( min_value, max_value );
    }
    else
    {
        SuperClass::updateMinMaxValues();
        const double min_value = kvsml->hasMinValue() ? kvsml->minValue() : SuperClass::minValue();
        const double max_value = kvsml->hasMaxValue() ? kvsml->maxValue() : SuperClass::maxValue();
        SuperClass::setMinMaxValues( min_value, max_value );
    }
}

/*==========================================================================*/
/**
 *  @brief  Imports the AVS UCD format data.
 *  @param  ucd [in] pointer to the AVS UCD format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::AVSUcd* ucd )
{
    SuperClass::setVeclen( ucd->veclens()[ ucd->componentID() ] );
    SuperClass::setNNodes( ucd->nnodes() );
    SuperClass::setNCells( ucd->nelements() );
    SuperClass::setCellType( ::ElementTypeToCellType( ucd->elementType() ) );
    SuperClass::setCoords( ucd->coords() );
    SuperClass::setConnections( ucd->connections() );
    SuperClass::setValues( kvs::AnyValueArray( ucd->values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

/*===========================================================================*/
/**
 *  @brief  Imports the AVS Filed format data.
 *  @param  field [in] pointer to the AVS Filed format data
 */
/*===========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::AVSField* field )
{
    if( field->fieldType() != kvs::AVSField::Irregular )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Cannot import uniform/rectilinear type AVS field data.");
        return;
    }

    const size_t line_size  = field->dim().x();
    const size_t slice_size = field->dim().y();
    const kvs::Vector3ui ncells( field->dim() - kvs::Vector3ui( 1, 1, 1 ) );
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

    SuperClass::setVeclen( field->veclen() );
    SuperClass::setNNodes( field->values().size() );
    SuperClass::setNCells( ncells.x() * ncells.y() * ncells.z() );
    SuperClass::setCellType( Hexahedra );
    SuperClass::setCoords( field->coords() );
    SuperClass::setConnections( connections );
    SuperClass::setValues( field->values() );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

} // end of namespace kvs

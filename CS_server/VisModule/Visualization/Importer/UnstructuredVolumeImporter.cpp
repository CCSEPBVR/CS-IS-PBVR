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
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/DebugNew>
#include <vismodule/AVSUcd>
#include <vismodule/Message>
#include <vismodule/Vector3>
#include <vismodule/endian2>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef EXTEND_FILE_FORMAT 
#include <kvs/extendedfileformat/VtkImporter>
#endif

namespace
{

/*==========================================================================*/
/**
 *  @brief  Converts to the cell type from the given string.
 *  @param  cell_type [in] grid type string
 *  @return cell type
 */
/*==========================================================================*/
const vismodule::UnstructuredVolumeObject::CellType StringToCellType( const std::string& cell_type )
{
    if (      cell_type == "tetrahedra" )
    {
        return vismodule::UnstructuredVolumeObject::Tetrahedra;
    }
    else if ( cell_type == "quadratic tetrahedra" )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticTetrahedra;
    }
    else if ( cell_type == "hexahedra"  )
    {
        return vismodule::UnstructuredVolumeObject::Hexahedra;
    }
    else if ( cell_type == "quadratic hexahedra"  )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticHexahedra;
    }
    else if ( cell_type == "prism"  )
    {
        return vismodule::UnstructuredVolumeObject::Prism;
    }
    else if ( cell_type == "pyramid"  )
    {
        return vismodule::UnstructuredVolumeObject::Pyramid;
    }
    else if ( cell_type == "point"  )
    {
        return vismodule::UnstructuredVolumeObject::Point;
    }
    else if ( cell_type == "triangle"  )
    {
        return vismodule::UnstructuredVolumeObject::Triangle;
    }
    else if ( cell_type == "triangle2"  )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticTriangle;
    }
    else if ( cell_type == "quadratic"  )
    {
        return vismodule::UnstructuredVolumeObject::Square;
    }
    else if ( cell_type == "quadratic2"  )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticSquare;
    }
    else
    {
        visModuleMessageError( "Unknown cell type '%s'.", cell_type.c_str() );
        return vismodule::UnstructuredVolumeObject::UnknownCellType;
    }
}

/*==========================================================================*/
/**
 *  @brief  Converts to the cell type from the given element type.
 *  @param  element_type [in] element type
 *  @return cell type
 */
/*==========================================================================*/
const vismodule::UnstructuredVolumeObject::CellType ElementTypeToCellType(
    const vismodule::AVSUcd::ElementType& element_type )
{
    if ( element_type == vismodule::AVSUcd::Tetrahedra  )
    {
        return vismodule::UnstructuredVolumeObject::Tetrahedra;
    }
    else if ( element_type == vismodule::AVSUcd::Tetrahedra2 )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticTetrahedra;
    }
    else if ( element_type == vismodule::AVSUcd::Hexahedra )
    {
        return vismodule::UnstructuredVolumeObject::Hexahedra;
    }
    else if ( element_type == vismodule::AVSUcd::Hexahedra2 )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticHexahedra;
    }
    else
    {
        visModuleMessageError( "Unknown element type." );
        return vismodule::UnstructuredVolumeObject::UnknownCellType;
    }
}

#ifdef EXTEND_FILE_FORMAT 
const vismodule::UnstructuredVolumeObject::CellType ConvertCellTypeKVS2VisModule( const kvs::UnstructuredVolumeObject::CellType cell_type )
{
    if ( cell_type == kvs::UnstructuredVolumeObject::Tetrahedra )
    {
        return vismodule::UnstructuredVolumeObject::Tetrahedra;
    }
    else if ( cell_type == kvs::UnstructuredVolumeObject::QuadraticTetrahedra )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticTetrahedra;
    }
    else if ( cell_type == kvs::UnstructuredVolumeObject::Hexahedra )
    {
        return vismodule::UnstructuredVolumeObject::Hexahedra;
    }
    else if ( cell_type == kvs::UnstructuredVolumeObject::QuadraticHexahedra )
    {
        return vismodule::UnstructuredVolumeObject::QuadraticHexahedra;
    }
    else if ( cell_type == kvs::UnstructuredVolumeObject::Pyramid )
    {
        return vismodule::UnstructuredVolumeObject::Pyramid;
    }
    else if ( cell_type == kvs::UnstructuredVolumeObject::Prism )
    {
        return vismodule::UnstructuredVolumeObject::Prism;
    }
    else
    {
        visModuleMessageError( "Unknown element type." );
        return vismodule::UnstructuredVolumeObject::UnknownCellType;
    }
}
#endif
} // end of namespace


namespace vismodule
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
    if ( vismodule::SPLITTypeSubvolume::CheckFileExtension( filename ) )
    {
        vismodule::SPLITTypeSubvolume* file_format = new vismodule::SPLITTypeSubvolume( filename );
        if ( !&file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            delete file_format;
            return;
        }

        this->import( *file_format );
        delete file_format;
    }
    else if ( vismodule::AVSUcd::CheckFileExtension( filename ) )
    {
        vismodule::AVSUcd* file_format = new vismodule::AVSUcd( filename );
        if ( !&file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s'.", filename.c_str() );
            delete file_format;
            return;
        }

        this->import( *file_format );
        delete file_format;
    }
    /* 131017 removed
        else if ( vismodule::AVSField::CheckFileExtension( filename ) )
        {
            vismodule::AVSField* file_format = new vismodule::AVSField( filename );
            if( !file_format )
            {
                BaseClass::m_is_success = false;
                visModuleMessageError("Cannot read '%s'.",filename.c_str());
                return;
            }

            if( file_format->isFailure() )
            {
                BaseClass::m_is_success = false;
                visModuleMessageError("Cannot read '%s'.",filename.c_str());
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
        visModuleMessageError( "Cannot import '%s'.", filename.c_str() );
        return;
    }
}

UnstructuredVolumeImporter::UnstructuredVolumeImporter( const std::string& filename,
                                                        const int fileType, const int st, const int vl )
{
    if ( fileType == 1 )  // Gathered Subvolume file
    {
        vismodule::AggregateTypeSubvolume* file_format =
            new vismodule::AggregateTypeSubvolume( filename, st, vl );
        if ( !&file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
            delete file_format;
            return;
        }
        this->import( *file_format );
        delete file_format;

    }
    else if ( fileType == 2 ) // Gathered Timestep file
    {
        vismodule::StepAggregateTypeSubvolume* file_format =
            new vismodule::StepAggregateTypeSubvolume( filename, st, vl );
        if ( !&file_format )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
            delete file_format;
            return;
        }
        this->import( *file_format );
        delete file_format;

    }
    else                          // Unsupported filetype
    {
        BaseClass::m_is_success = false;
        visModuleMessageError( "Unsupported fileType '%d'.", fileType );
    }

    return;
}

#ifdef EXTEND_FILE_FORMAT 
UnstructuredVolumeImporter::UnstructuredVolumeImporter( const std::string& filename, const int fileType, const int targetCellType, const int st, const int vl )
{
    std::string edit_filename = filename; // ファイル名編集用の文字列
    std::string time_step_str = std::to_string( st ); // タイムステップを文字列に変換
    size_t found_asterisk = edit_filename.find( '*' ); // ファイル名に時系列ファイルのアスタリスクが含まれているか確認

    // ファイルの拡張子を確認
    size_t found_vtm = edit_filename.find( ".vtm" );
    size_t found_vtu = edit_filename.find( ".vtu" );

    // 時系列ファイルの場合、アスタリスクをタイムステップに置換
    if ( found_asterisk != std::string::npos )
    {
        edit_filename.replace( found_asterisk, 1, time_step_str );
    }

    if ( found_vtm != std::string::npos )
    {
        kvs::ExtendedFileFormat::VtkXmlMultiBlock* file_format = new kvs::ExtendedFileFormat::VtkXmlMultiBlock( edit_filename );
        this->import( *file_format, targetCellType, vl );
        delete file_format;
    }
    else if ( found_vtu != std::string::npos )
    {
        kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid* file_format = new kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid( edit_filename );
        this->import( *file_format, targetCellType );
        delete file_format;
    }
}
#endif

/*==========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVolumeImporter class.
 *  @param  file_format [in] pointer to the file format data
 */
/*==========================================================================*/
UnstructuredVolumeImporter::UnstructuredVolumeImporter( const vismodule::FileFormatBase& file_format )
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
UnstructuredVolumeImporter::SuperClass* UnstructuredVolumeImporter::exec( const vismodule::FileFormatBase& file_format )
{
    if ( !&file_format )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError( "Input file format is NULL." );
        return NULL;
    }

    const std::string class_name = file_format.className();
    if ( class_name == "vismodule::SPLITTypeSubvolume" )
    {
        const vismodule::SPLITTypeSubvolume* fformat = static_cast<const vismodule::SPLITTypeSubvolume*>( &file_format );
        this->import( *fformat );
    }
    else if ( class_name == "vismodule::AVSUcd" )
    {
        const vismodule::AVSUcd* fformat = static_cast<const vismodule::AVSUcd*>( &file_format );
        this->import( *fformat );
    }
    /* 131017 removed
        else if ( class_name == "vismodule::AVSField" )
        {
            this->import( static_cast<const vismodule::AVSField*>( file_format ) );
        }
    */
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError( "Input file format is not supported." );
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
void UnstructuredVolumeImporter::import( const vismodule::SPLITTypeSubvolume& kvsml )
{
    if ( kvsml.objectTag().hasExternalCoord() )
    {
        const vismodule::Vector3f min_coord( kvsml.objectTag().minExternalCoord() );
        const vismodule::Vector3f max_coord( kvsml.objectTag().maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( kvsml.objectTag().hasObjectCoord() )
    {
        const vismodule::Vector3f min_coord( kvsml.objectTag().minObjectCoord() );
        const vismodule::Vector3f max_coord( kvsml.objectTag().maxObjectCoord() );
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
void UnstructuredVolumeImporter::import( const vismodule::AVSUcd& ucd )
{
    SuperClass::setVeclen( ucd.veclens()[ ucd.componentID() ] );
    SuperClass::setNNodes( ucd.nnodes() );
    SuperClass::setNCells( ucd.nelements() );
    SuperClass::setCellType( ::ElementTypeToCellType( ucd.elementType() ) );
    SuperClass::setCoords( ucd.coords() );
    SuperClass::setConnections( ucd.connections() );
    SuperClass::setValues( vismodule::AnyValueArray( ucd.values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

/*===========================================================================*/
/**
 *  @brief  Imports the AVS Filed format data.
 *  @param  field [in] pointer to the AVS Filed format data
 */
/*===========================================================================*/
void UnstructuredVolumeImporter::import( const vismodule::AVSField& field )
{
    if ( field.fieldType() != vismodule::AVSField::Irregular )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError( "Cannot import uniform/rectilinear type AVS field data." );
        return;
    }

    const size_t line_size  = field.dim().x();
    const size_t slice_size = field.dim().y();
    const vismodule::Vector3ui ncells( field.dim() - vismodule::Vector3ui( 1, 1, 1 ) );
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
void UnstructuredVolumeImporter::import( const vismodule::AggregateTypeSubvolume& gs )
{
    SuperClass::setVeclen( gs.veclen() );
    SuperClass::setNNodes( gs.nnodes() );
    SuperClass::setNCells( gs.ncells() );
    // printf(" gs: %d %d %d\n", fgs.veclen(), fgs.nnodes(), fgs.ncells());
    SuperClass::setCellType( ::StringToCellType( gs.cellType() ) );
    SuperClass::setCoords( gs.coords() );
    SuperClass::setConnections( gs.connections() );
    SuperClass::setValues( vismodule::AnyValueArray( gs.values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

/*==========================================================================*/
/**
 *  @brief  Imports the Gathered Timestep format data.
 *  @param  gs [in] pointer to the Gathered Subvolume format data
 */
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const vismodule::StepAggregateTypeSubvolume& gt )
{
    SuperClass::setVeclen( gt.veclen() );
    SuperClass::setNNodes( gt.nnodes() );
    SuperClass::setNCells( gt.ncells() );
    // printf(" gt: %d %d %d\n", gt.veclen(), gt.nnodes(), gt.ncells());
    SuperClass::setCellType( ::StringToCellType( gt.cellType() ) );
    SuperClass::setCoords( gt.coords() );
    SuperClass::setConnections( gt.connections() );
    SuperClass::setValues( vismodule::AnyValueArray( gt.values() ) );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();
}

#ifdef EXTEND_FILE_FORMAT 
/*==========================================================================*/
/**
*  @brief  Imports the VTK MultiBlock format data.
*  @param  vtm [in] pointer to the VTK MultiBlock format data
*  @param  targetCellType [in] cell type in KVS
*  @param  vl [in] sub volume id
*/
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::ExtendedFileFormat::VtkXmlMultiBlock& vtm , const int targetCellType, const int vl )
{
    std::unordered_map<int, int> sub_volume_ids;

    for ( auto format : vtm.eachBlock() )
    {
        if ( auto input_vtu = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
        {
            for ( auto vtu : input_vtu->eachCellType() )
            {
                kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( input_vtu );
                kvs::UnstructuredVolumeObject* object = &importer;
                auto kvs_cell_type = object->cellType();

                if ( kvs_cell_type != targetCellType ) continue;

                if ( sub_volume_ids.count(kvs_cell_type) == 0 ) sub_volume_ids[kvs_cell_type] = 0;

                if ( sub_volume_ids[kvs_cell_type] != vl ) 
                {
                    ++sub_volume_ids[kvs_cell_type];
                    continue;
                }

                auto vismodule_cell_type = ::ConvertCellTypeKVS2VisModule( kvs_cell_type );
                SuperClass::setCellType( vismodule_cell_type );
                
                vismodule::ValueArray<vismodule::UInt32> tmp_connections_array( object->ncells() * SuperClass::cellType() );
                for (int i = 0; i < (object->ncells() * SuperClass::cellType()); i++)
                {
                    tmp_connections_array[i] = object->connections()[i];
                }
                
                vismodule::ValueArray<float> tmp_coords_array( object->coords().size() );
                for (int i = 0; i < object->coords().size(); i++)
                {
                    tmp_coords_array[i] = object->coords()[i];
                }
                
                vismodule::ValueArray<float> tmp_values_array( object->values().size() );
                for (int i = 0; i < object->values().size(); i++)
                {
                    tmp_values_array[i] = object->values().asValueArray<float>()[i];
                }
                vismodule::AnyValueArray tmp_any_value_array( tmp_values_array );
                
                const double min_value = SuperClass::minValue();
                const double max_value = SuperClass::maxValue();

                SuperClass::setVeclen( object->veclen() );
                SuperClass::setNNodes( object->nnodes() );
                SuperClass::setNCells( object->ncells() );
                SuperClass::setCoords( tmp_coords_array );
                SuperClass::setConnections( tmp_connections_array );
                SuperClass::setValues( tmp_any_value_array );
                SuperClass::updateMinMaxCoords();
                SuperClass::setMinMaxValues( min_value, max_value );
                return;
            }
        }
    }
}

/*==========================================================================*/
/**
*  @brief  Imports the VTK Unstructured Grid format data.
*  @param  vtm [in] pointer to the VTK Unstructured Grid format data
*  @param  targetCellType [in] cell type in KVS
*/
/*==========================================================================*/
void UnstructuredVolumeImporter::import( const kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid& vtu , const int targetCellType )
{
        for ( auto vtu_cell : vtu.eachCellType() )
        {
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &vtu_cell );
            kvs::UnstructuredVolumeObject* object = &importer;
            auto kvs_cell_type = object->cellType();

            if ( kvs_cell_type != targetCellType ) continue;

            auto vismodule_cell_type = ::ConvertCellTypeKVS2VisModule( kvs_cell_type );
            SuperClass::setCellType( vismodule_cell_type );
            
            vismodule::ValueArray<vismodule::UInt32> tmp_connections_array( object->ncells() * SuperClass::cellType() );
            for (int i = 0; i < (object->ncells() * SuperClass::cellType()); i++)
            {
                tmp_connections_array[i] = object->connections()[i];
            }
            
            vismodule::ValueArray<float> tmp_coords_array( object->coords().size() );
            for (int i = 0; i < object->coords().size(); i++)
            {
                tmp_coords_array[i] = object->coords()[i];
            }
            
            vismodule::ValueArray<float> tmp_values_array( object->values().size() );
            for (int i = 0; i < object->values().size(); i++)
            {
                tmp_values_array[i] = object->values().asValueArray<float>()[i];
            }
            vismodule::AnyValueArray tmp_any_value_array( tmp_values_array );
            
            const double min_value = SuperClass::minValue();
            const double max_value = SuperClass::maxValue();

            SuperClass::setVeclen( object->veclen() );
            SuperClass::setNNodes( object->nnodes() );
            SuperClass::setNCells( object->ncells() );
            SuperClass::setCoords( tmp_coords_array );
            SuperClass::setConnections( tmp_connections_array );
            SuperClass::setValues( tmp_any_value_array );
            SuperClass::updateMinMaxCoords();
            SuperClass::setMinMaxValues( min_value, max_value );
            return;
        }
}
#endif

} // end of namespace vismodule

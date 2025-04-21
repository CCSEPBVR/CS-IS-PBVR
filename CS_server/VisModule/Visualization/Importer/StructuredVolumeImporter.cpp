/****************************************************************************/
/**
 *  @file StructuredVolumeImporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeImporter.cpp 867 2011-07-24 06:12:29Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/DebugNew>
#include <vismodule/AVSField>
//#include <vismodule/DicomList>
#include <vismodule/Message>
#include <vismodule/Vector3>
#include <vismodule/Directory>
#include <vismodule/Value>

#ifdef EXTEND_FILE_FORMAT 
#include <kvs/extendedfileformat/VtkImporter>
#endif

namespace
{

/*==========================================================================*/
/**
 *  @brief  Converts to the grid type from the given string.
 *  @param  grid_type [in] grid type string
 *  @return grid type
 */
/*==========================================================================*/
const vismodule::StructuredVolumeObject::GridType StringToGridType( const std::string& grid_type )
{
    if (      grid_type == "uniform"     ) { return( vismodule::StructuredVolumeObject::Uniform );     }
    else if ( grid_type == "rectilinear" ) { return( vismodule::StructuredVolumeObject::Rectilinear ); }
    else if ( grid_type == "curvilinear" ) { return( vismodule::StructuredVolumeObject::Curvilinear ); }
    else
    {
        visModuleMessageError( "Unknown grid type '%s'.", grid_type.c_str() );
        return( vismodule::StructuredVolumeObject::UnknownGridType );
    }
}

} // end of namespace


namespace vismodule
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new StructuredVolumeImporter class.
 */
/*==========================================================================*/
StructuredVolumeImporter::StructuredVolumeImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new StructuredVolumeImporter class.
 *  @param  filename [in] input filename
 */
/*===========================================================================*/
StructuredVolumeImporter::StructuredVolumeImporter( const std::string& filename )
{
    if ( vismodule::KVSMLObjectStructuredVolume::CheckFileExtension( filename ) )
    {
        vismodule::KVSMLObjectStructuredVolume* file_format = new vismodule::KVSMLObjectStructuredVolume( filename );
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
//    else if ( vismodule::DicomList::CheckDirectory( filename ) )
//    {
//        vismodule::DicomList* file_format = new vismodule::DicomList( filename );
//        if( !file_format )
//        {
//            BaseClass::m_is_success = false;
//            visModuleMessageError("Cannot read '%s'.",filename.c_str());
//            return;
//        }
//
//        if( file_format->isFailure() )
//        {
//            BaseClass::m_is_success = false;
//            visModuleMessageError("Cannot read '%s'.",filename.c_str());
//            delete file_format;
//            return;
//        }
//
////        this->import( *file_format );
//        delete file_format;
//    }
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Cannot import '%s'.",filename.c_str());
        return;
    }
}

//StructuredVolumeImporter::StructuredVolumeImporter( const std::string& filename,
//                                                        const int fileType, const int st, const int vl )
//{
//    if ( fileType == 1 )  // Gathered Subvolume file
//    {
//        vismodule::AggregateTypeSubvolume* file_format =
//            new vismodule::AggregateTypeSubvolume( filename, st, vl );
//        if ( !&file_format )
//        {
//            BaseClass::m_is_success = false;
//            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
//            return;
//        }
//
//        if ( file_format->isFailure() )
//        {
//            BaseClass::m_is_success = false;
//            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
//            delete file_format;
//            return;
//        }
//        this->import( *file_format );
//        delete file_format;
//
//    }
//    else if ( fileType == 2 ) // Gathered Timestep file
//    {
//        vismodule::StepAggregateTypeSubvolume* file_format =
//            new vismodule::StepAggregateTypeSubvolume( filename, st, vl );
//        if ( !&file_format )
//        {
//            BaseClass::m_is_success = false;
//            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
//            return;
//        }
//
//        if ( file_format->isFailure() )
//        {
//            BaseClass::m_is_success = false;
//            visModuleMessageError( "Cannot read '%s*'.", filename.c_str() );
//            delete file_format;
//            return;
//        }
//        this->import( *file_format );
//        delete file_format;
//
//    }
//    else                          // Unsupported filetype
//    {
//        BaseClass::m_is_success = false;
//        visModuleMessageError( "Unsupported fileType '%d'.", fileType );
//    }
//
//    return;
//}

#ifdef EXTEND_FILE_FORMAT 
StructuredVolumeImporter::StructuredVolumeImporter( const std::string& filename, const int st, const int vl )
{
    std::string edit_filename = filename; // ファイル名編集用の文字列
    std::string time_step_str = std::to_string( st ); // タイムステップを文字列に変換
    size_t found_asterisk = edit_filename.find( '*' ); // ファイル名に時系列ファイルのアスタリスクが含まれているか確認

    // ファイルの拡張子を確認
    size_t found_vtm = edit_filename.find( ".vtm" );
    size_t found_vti = edit_filename.find( ".vti" );

    // 時系列ファイルの場合、アスタリスクをタイムステップに置換
    if ( found_asterisk != std::string::npos )
    {
        edit_filename.replace( found_asterisk, 1, time_step_str );
    }
    
    std::cout << "edit_filename:" << edit_filename << std::endl;

    if ( found_vtm != std::string::npos )
    {
        kvs::ExtendedFileFormat::VtkXmlMultiBlock* file_format = new kvs::ExtendedFileFormat::VtkXmlMultiBlock( edit_filename );
        this->import( *file_format, vl );
        delete file_format;
    }
    else if ( found_vti != std::string::npos )
    {
        kvs::ExtendedFileFormat::VtkXmlImageData* file_format = new kvs::ExtendedFileFormat::VtkXmlImageData( edit_filename );
        this->import( *file_format, vl );
        delete file_format;
    }
}
#endif

/*==========================================================================*/
/**
 *  @brief  Constructs a new StructuredVolumeImporter class.
 *  @param  file_format [in] pointer to the file format data
 */
/*==========================================================================*/
StructuredVolumeImporter::StructuredVolumeImporter( const vismodule::FileFormatBase& file_format )
{
    if ( !this->exec( file_format ) ) BaseClass::m_is_success = true;
}

/*===========================================================================*/
/**
 *  @brief  Destructs the StructuredVolumeImporter class.
 */
/*===========================================================================*/
StructuredVolumeImporter::~StructuredVolumeImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the import process.
 *  @param  file_format [in] pointer to the file format data
 *  @return pointer to the imported structured volume object
 */
/*===========================================================================*/
//StructuredVolumeImporter::SuperClass* StructuredVolumeImporter::exec( const vismodule::FileFormatBase& file_format )
StructuredVolumeImporter::SuperClass* StructuredVolumeImporter::exec( const vismodule::FileFormatBase& file_format )
{

    if ( !(&file_format) )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name( file_format.className() );
    if ( class_name == "vismodule::KVSMLObjectStructuredVolume" )
    {
//        this->import( static_cast<const vismodule::KVSMLObjectStructuredVolume*>( &file_format ) );
        const vismodule::KVSMLObjectStructuredVolume* fformat =static_cast<const vismodule::KVSMLObjectStructuredVolume*>( &file_format ); 
        this->import( fformat );
    }
    else if ( class_name == "vismodule::AVSField" )
    {
        const vismodule::AVSField* fformat =static_cast<const vismodule::AVSField*>( &file_format ); 
        this->import( fformat );
        //this->import( static_cast<const vismodule::AVSField*>( &file_format ) );
    }
//    else if ( class_name == "vismodule::DicomList" )
//    {
//        const vismodule::DicomList* fformat =static_cast<const vismodule::DicomList*>( &file_format );
//        this->import( *fformat );
//        //this->import( static_cast<const vismodule::DicomList*>( &file_format ) );
//    }
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input file format is not supported.");
        return( NULL );
    }

    return( this );
}

/*==========================================================================*/
/**
 *  @brief  Imports the KVSML format data.
 *  @param  kvsml [in] pointer to the KVSML format data
 */
/*==========================================================================*/
void StructuredVolumeImporter::import(
    const vismodule::KVSMLObjectStructuredVolume* kvsml )
{
    if ( kvsml->objectTag().hasExternalCoord() )
    {
        const vismodule::Vector3f min_coord( kvsml->objectTag().minExternalCoord() );
        const vismodule::Vector3f max_coord( kvsml->objectTag().maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( kvsml->objectTag().hasObjectCoord() )
    {
        const vismodule::Vector3f min_coord( kvsml->objectTag().minObjectCoord() );
        const vismodule::Vector3f max_coord( kvsml->objectTag().maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    SuperClass::setGridType( ::StringToGridType( kvsml->gridType() ) );
    SuperClass::setResolution( kvsml->resolution() );
    SuperClass::setVeclen( kvsml->veclen() );
    SuperClass::setValues( kvsml->values() );

    if ( SuperClass::gridType() == SuperClass::Rectilinear ||
         SuperClass::gridType() == SuperClass::Curvilinear )
    {
        SuperClass::setCoords( kvsml->coords() );
    }
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
 *  @brief  Imports the AVS field format data.
 *  @param  field [in] pointer to the AVS field format data
 */
/*==========================================================================*/
void StructuredVolumeImporter::import( const vismodule::AVSField* field )
{
    if ( field->fieldType() != vismodule::AVSField::Uniform )
    {
        visModuleMessageWarning( field->fieldType() == vismodule::AVSField::Uniform,
                           "'Uniform' type in vismodule::AVSField format is only supported." );

        BaseClass::m_is_success = false;
    }

    const float dim_x = static_cast<float>( field->dim().x() );
    const float dim_y = static_cast<float>( field->dim().y() );
    const float dim_z = static_cast<float>( field->dim().z() );
    const vismodule::Vector3f  min_obj_coord( 0.0f );
    const vismodule::Vector3f  max_obj_coord( dim_x - 1.0f, dim_y - 1.0f, dim_z - 1.0f );
    SuperClass::setMinMaxObjectCoords( min_obj_coord, max_obj_coord );

    if ( field->hasMinMaxExt() )
    {
        const vismodule::Vector3f min_ext_coord( field->minExt() );
        const vismodule::Vector3f max_ext_coord( field->maxExt() );
        SuperClass::setMinMaxExternalCoords( min_ext_coord, max_ext_coord );
    }
    else
    {
        const vismodule::Vector3f min_ext_coord( min_obj_coord );
        const vismodule::Vector3f max_ext_coord( max_obj_coord );
        SuperClass::setMinMaxExternalCoords( min_ext_coord, max_ext_coord );
    }

    SuperClass::setGridType( vismodule::StructuredVolumeObject::Uniform );
    SuperClass::setVeclen( field->veclen() );
    SuperClass::setResolution( field->dim() );
    SuperClass::setValues( field->values() );
    SuperClass::updateMinMaxValues();
}

//void StructuredVolumeImporter::import( const vismodule::DicomList& dicom_list )
//{
//    if ( dicom_list.size() == 0 )
//    {
//        BaseClass::m_is_success = false;
//        visModuleMessageError("Dicom file is not included.");
//        return;
//    }
//
//    const float spacing = static_cast<float>( dicom_list.sliceSpacing() );
//    const float thickness = static_cast<float>( dicom_list.sliceThickness() );
//
//    const size_t x_size = dicom_list.width();
//    const size_t y_size = dicom_list.height();
//    const size_t z_size = dicom_list.nslices();
//    const float x_ratio = dicom_list.pixelSpacing()[0];
//    const float y_ratio = dicom_list.pixelSpacing()[1];
//    const float z_ratio = vismodule::Math::IsZero( spacing ) ? thickness : spacing;
//
//    const vismodule::Vector3f min_obj_coord( 0.0f, 0.0f, 0.0f );
//    const vismodule::Vector3f max_obj_coord( x_size - 1.0f, y_size - 1.0f, z_size - 1.0f );
//    SuperClass::setMinMaxObjectCoords( min_obj_coord, max_obj_coord );
//
//    const vismodule::Vector3f min_ext_coord( min_obj_coord );
//    const vismodule::Vector3f max_ext_coord( max_obj_coord.x() * x_ratio,
//                                       max_obj_coord.y() * y_ratio,
//                                       max_obj_coord.z() * z_ratio );
//    SuperClass::setMinMaxExternalCoords( min_ext_coord, max_ext_coord );
//
//    bool shift = true;
//    const vismodule::Dicom* dicom = (dicom_list)[0];
//    const vismodule::UInt32 bits_allocated = dicom->bitsAllocated();
//    const bool pixel_representation = dicom->pixelRepresentation();
//    switch ( bits_allocated )
//    {
//    case 8:
//    {
//        const vismodule::AnyValueArray values = this->get_dicom_data<vismodule::UInt8>( &dicom_list, false );
//        SuperClass::setValues( values );
//        break;
//    }
//    case 16:
//    {
//        if ( pixel_representation )
//        {
//            const vismodule::AnyValueArray values = this->get_dicom_data<vismodule::UInt16>( &dicom_list, false );
//            SuperClass::setValues( values );
//        }
//        else
//        {
//            if ( shift )
//            {
//                const vismodule::AnyValueArray values = this->get_dicom_data<vismodule::UInt16>( &dicom_list, true );
//                SuperClass::setValues( values );
//            }
//            else
//            {
//                const vismodule::AnyValueArray values = this->get_dicom_data<vismodule::Int16>( &dicom_list, false );
//                SuperClass::setValues( values );
//            }
//        }
//        break;
//    }
//    default: break;
//    }
//
//    const vismodule::Vector3ui resolution( x_size, y_size, z_size );
//    SuperClass::setGridType( vismodule::StructuredVolumeObject::Uniform );
//    SuperClass::setResolution( resolution );
//    SuperClass::setVeclen( 1 );
//    SuperClass::updateMinMaxValues();
//}

//template <typename T>
//const vismodule::AnyValueArray StructuredVolumeImporter::get_dicom_data(
//    const vismodule::DicomList* dicom_list,
//    const bool shift )
//{
//    const size_t width = dicom_list->width();
//    const size_t height = dicom_list->height();
//    const size_t nslices = dicom_list->nslices();
//    const size_t nnodes = width * height * nslices;
//
//    const double min_range = static_cast<double>( vismodule::Value<T>::Min() );
//    const double max_range = static_cast<double>( vismodule::Value<T>::Max() );
//
//    vismodule::AnyValueArray values;
//    values.template allocate<T>( nnodes );
//
//    T* pvalues = static_cast<T*>( values.pointer() );
//    for ( size_t k = 0; k < nslices; k++ )
//    {
//        const vismodule::Dicom* dicom = (*dicom_list)[k];
//        const T* const raw_data = reinterpret_cast<const T*>( dicom->rawData().pointer() );
//        const int shift_value = shift ? dicom->minRawValue() : 0;
//
//        for ( size_t j = 0; j < height; j++ )
//        {
//            for ( size_t i = 0; i < width; i++ )
//            {
//                const size_t pixel_index = ( height - j - 1 ) * width + i;
//                double value = static_cast<double>( raw_data[ pixel_index ] );
//                value = value - shift_value;
//                value = vismodule::Math::Clamp( value, min_range, max_range );
//
//                *(pvalues++) = static_cast<T>( value );
//            }
//        }
//    }
//
//    return( values );
//}

/*==========================================================================*/
/**
*  @brief  Imports the VTK MultiBlock format data.
*  @param  vtm [in] pointer to the VTK MultiBlock format data
*  @param  targetCellType [in] cell type in KVS
*  @param  vl [in] sub volume id
*/
/*==========================================================================*/
#ifdef EXTEND_FILE_FORMAT 
void StructuredVolumeImporter::import( const kvs::ExtendedFileFormat::VtkXmlMultiBlock& vtm , const int vl )
{
    int sub_volume_id = 0;

    for ( auto format : vtm.eachBlock() )
    {
        if ( auto input_vti = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlImageData*>( format.get() ) )
        {
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( input_vti );
            kvs::StructuredVolumeObject* object = &importer;
            auto cell_type = 7;

            if ( sub_volume_id != vl ) 
            {
                ++sub_volume_id;
                continue;
            }

            vismodule::Vector3f min_external_coord;
            vismodule::Vector3f max_external_coord;
            float min_external_coord_x = object->minExternalCoord()[0];
            float min_external_coord_y = object->minExternalCoord()[1];
            float min_external_coord_z = object->minExternalCoord()[2];
            float max_external_coord_x = object->maxExternalCoord()[0];
            float max_external_coord_y = object->maxExternalCoord()[1];
            float max_external_coord_z = object->maxExternalCoord()[2];
            min_external_coord.set( min_external_coord_x, min_external_coord_y, min_external_coord_z );
            max_external_coord.set( max_external_coord_x, max_external_coord_y, max_external_coord_z );
            
            vismodule::Vector3f min_object_coord;
            vismodule::Vector3f max_object_coord;
            float min_object_coord_x = object->minObjectCoord()[0];
            float min_object_coord_y = object->minObjectCoord()[1];
            float min_object_coord_z = object->minObjectCoord()[2];
            float max_object_coord_x = object->maxObjectCoord()[0];
            float max_object_coord_y = object->maxObjectCoord()[1];
            float max_object_coord_z = object->maxObjectCoord()[2];
            min_object_coord.set( min_object_coord_x, min_object_coord_y, min_object_coord_z );
            max_object_coord.set( max_object_coord_x, max_object_coord_y, max_object_coord_z );
            
            vismodule::Vector3ui resolution;
            unsigned int resolution_x = object->resolution()[0];
            unsigned int resolution_y = object->resolution()[1];
            unsigned int resolution_z = object->resolution()[2];
            resolution.set( resolution_x, resolution_y, resolution_z );

            vismodule::ValueArray<float> tmp_values_array( object->values().size() );
            for (int i = 0; i < object->values().size(); i++)
            {
                tmp_values_array[i] = object->values().asValueArray<float>()[i];
            }
            vismodule::AnyValueArray tmp_any_value_array( tmp_values_array );

            SuperClass::setMinMaxExternalCoords( min_external_coord, max_external_coord );
            SuperClass::setMinMaxObjectCoords( min_object_coord, max_object_coord );
            SuperClass::setGridType( vismodule::StructuredVolumeObject::Uniform );
            SuperClass::setResolution( resolution );
            SuperClass::setVeclen( object->veclen() );
            SuperClass::setValues( tmp_any_value_array );
            SuperClass::setMinMaxValues( object->minValue(), object->maxValue() );
            return;
        }
    }
}

void StructuredVolumeImporter::import( const kvs::ExtendedFileFormat::VtkXmlImageData& vti , const int vl )
{
    kvs::ExtendedFileFormat::VtkXmlImageData* non_const_vti = const_cast<kvs::ExtendedFileFormat::VtkXmlImageData*>( &vti );
    kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( non_const_vti );
    kvs::StructuredVolumeObject* object = &importer;
    auto cell_type = 7;

    vismodule::Vector3f min_external_coord;
    vismodule::Vector3f max_external_coord;
    float min_external_coord_x = object->minExternalCoord()[0];
    float min_external_coord_y = object->minExternalCoord()[1];
    float min_external_coord_z = object->minExternalCoord()[2];
    float max_external_coord_x = object->maxExternalCoord()[0];
    float max_external_coord_y = object->maxExternalCoord()[1];
    float max_external_coord_z = object->maxExternalCoord()[2];
    min_external_coord.set( min_external_coord_x, min_external_coord_y, min_external_coord_z );
    max_external_coord.set( max_external_coord_x, max_external_coord_y, max_external_coord_z );
    
    vismodule::Vector3f min_object_coord;
    vismodule::Vector3f max_object_coord;
    float min_object_coord_x = object->minObjectCoord()[0];
    float min_object_coord_y = object->minObjectCoord()[1];
    float min_object_coord_z = object->minObjectCoord()[2];
    float max_object_coord_x = object->maxObjectCoord()[0];
    float max_object_coord_y = object->maxObjectCoord()[1];
    float max_object_coord_z = object->maxObjectCoord()[2];
    min_object_coord.set( min_object_coord_x, min_object_coord_y, min_object_coord_z );
    max_object_coord.set( max_object_coord_x, max_object_coord_y, max_object_coord_z );
    
    vismodule::Vector3ui resolution;
    unsigned int resolution_x = object->resolution()[0];
    unsigned int resolution_y = object->resolution()[1];
    unsigned int resolution_z = object->resolution()[2];
    resolution.set( resolution_x, resolution_y, resolution_z );

    vismodule::ValueArray<float> tmp_values_array( object->values().size() );
    for (int i = 0; i < object->values().size(); i++)
    {
        tmp_values_array[i] = object->values().asValueArray<float>()[i];
    }
    vismodule::AnyValueArray tmp_any_value_array( tmp_values_array );

    SuperClass::setMinMaxExternalCoords( min_external_coord, max_external_coord );
    SuperClass::setMinMaxObjectCoords( min_object_coord, max_object_coord );
    SuperClass::setGridType( vismodule::StructuredVolumeObject::Uniform );
    SuperClass::setResolution( resolution );
    SuperClass::setVeclen( object->veclen() );
    SuperClass::setValues( tmp_any_value_array );
    SuperClass::setMinMaxValues( object->minValue(), object->maxValue() );  
}
#endif

// Instatiation.
//template
//const vismodule::AnyValueArray StructuredVolumeImporter::get_dicom_data<vismodule::UInt8>(
//    const vismodule::DicomList* dicom_list, const bool shift );
//
//template
//const vismodule::AnyValueArray StructuredVolumeImporter::get_dicom_data<vismodule::UInt16>(
//    const vismodule::DicomList* dicom_list, const bool shift );
//
//template
//const vismodule::AnyValueArray StructuredVolumeImporter::get_dicom_data<vismodule::Int16>(
//    const vismodule::DicomList* dicom_list, const bool shift );

} // end of namespace vismodule

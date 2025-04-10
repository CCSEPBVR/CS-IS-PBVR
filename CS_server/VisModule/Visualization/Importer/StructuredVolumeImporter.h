/****************************************************************************/
/**
 *  @file StructuredVolumeImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE
#define VIS_MODULE__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
//#include <vismodule/ClassName>
#include <vismodule/ClassName>
#include <vismodule/Module>
//#include <vismodule/StructuredVolumeObject>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/KVSMLObjectStructuredVolume>
#include <vismodule/AVSField>
//#include <vismodule/DicomList>

#ifdef EXTEND_FILE_FORMAT 
#include <kvs/extendedfileformat/VtkXmlMultiBlock>
#endif

namespace vismodule
{

/*==========================================================================*/
/**
 *  Structured volume object importer class.
 */
/*==========================================================================*/
class StructuredVolumeImporter :
    public vismodule::ImporterBase ,
    public vismodule::StructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::StructuredVolumeImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::StructuredVolumeObject );

public:

    StructuredVolumeImporter( void );

    StructuredVolumeImporter( const std::string& filename );

    StructuredVolumeImporter( const vismodule::FileFormatBase& file_format );

//    StructuredVolumeImporter( const std::string& filename, const int fileType, const int st, const int vl );

#ifdef EXTEND_FILE_FORMAT 
    StructuredVolumeImporter( const std::string& filename, const int st, const int vl );
#endif
    
    virtual ~StructuredVolumeImporter( void );

public:

    //SuperClass* exec( const vismodule::FileFormatBase& file_format );
    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectStructuredVolume* kvsml );
    //void import( const vismodule::KVSMLObjectStructuredVolume& kvsml );

    void import( const vismodule::AVSField* field );
    //void import( const vismodule::AVSField& field );

//    void import( const vismodule::DicomList& dicom_list );

#ifdef EXTEND_FILE_FORMAT 
    void import( const kvs::ExtendedFileFormat::VtkXmlMultiBlock& vtm , const int vl );
#endif

private:

//    template <typename T>
//    const vismodule::AnyValueArray get_dicom_data( const vismodule::DicomList* dicom_list, const bool shift );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE

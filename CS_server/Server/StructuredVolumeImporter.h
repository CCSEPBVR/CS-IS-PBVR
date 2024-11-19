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
#ifndef PBVR__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE
#define PBVR__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE

#include "ImporterBase.h"
//#include <vismodule/ClassName>
#include "ClassName.h"
#include <vismodule/Module>
//#include <vismodule/StructuredVolumeObject>
#include "StructuredVolumeObject.h"
#include <vismodule/KVSMLObjectStructuredVolume>
#include <vismodule/AVSField>
//#include <vismodule/DicomList>


namespace pbvr
{

/*==========================================================================*/
/**
 *  Structured volume object importer class.
 */
/*==========================================================================*/
class StructuredVolumeImporter :
    public vismodule::ImporterBase ,
    public pbvr::StructuredVolumeObject
{
    // Class name.
    visModuleClassName( pbvr::StructuredVolumeImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( pbvr::StructuredVolumeObject );

public:

    StructuredVolumeImporter( void );

    StructuredVolumeImporter( const std::string& filename );

    StructuredVolumeImporter( const vismodule::FileFormatBase* file_format );

//    StructuredVolumeImporter( const std::string& filename, const int fileType, const int st, const int vl );
    
    virtual ~StructuredVolumeImporter( void );

public:

    //SuperClass* exec( const vismodule::FileFormatBase* file_format );
    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectStructuredVolume* kvsml );
    //void import( const vismodule::KVSMLObjectStructuredVolume& kvsml );

    void import( const vismodule::AVSField* field );
    //void import( const vismodule::AVSField& field );

//    void import( const vismodule::DicomList& dicom_list );

private:

//    template <typename T>
//    const vismodule::AnyValueArray get_dicom_data( const vismodule::DicomList* dicom_list, const bool shift );
};

} // end of namespace vismodule

#endif // PBVR__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE

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
//#include <kvs/ClassName>
#include "ClassName.h"
#include <kvs/Module>
//#include <kvs/StructuredVolumeObject>
#include "StructuredVolumeObject.h"
#include <kvs/KVSMLObjectStructuredVolume>
#include <kvs/AVSField>
//#include <kvs/DicomList>


namespace pbvr
{

/*==========================================================================*/
/**
 *  Structured volume object importer class.
 */
/*==========================================================================*/
class StructuredVolumeImporter :
    public kvs::ImporterBase ,
    public pbvr::StructuredVolumeObject
{
    // Class name.
    kvsClassName( pbvr::StructuredVolumeImporter );

    // Module information.
    kvsModuleCategory( Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( pbvr::StructuredVolumeObject );

public:

    StructuredVolumeImporter( void );

    StructuredVolumeImporter( const std::string& filename );

    StructuredVolumeImporter( const kvs::FileFormatBase* file_format );

//    StructuredVolumeImporter( const std::string& filename, const int fileType, const int st, const int vl );
    
    virtual ~StructuredVolumeImporter( void );

public:

    //SuperClass* exec( const kvs::FileFormatBase* file_format );
    SuperClass* exec( const kvs::FileFormatBase& file_format );

private:

    void import( const kvs::KVSMLObjectStructuredVolume* kvsml );
    //void import( const kvs::KVSMLObjectStructuredVolume& kvsml );

    void import( const kvs::AVSField* field );
    //void import( const kvs::AVSField& field );

//    void import( const kvs::DicomList& dicom_list );

private:

//    template <typename T>
//    const kvs::AnyValueArray get_dicom_data( const kvs::DicomList* dicom_list, const bool shift );
};

} // end of namespace kvs

#endif // PBVR__STRUCTURED_VOLUME_IMPORTER_H_INCLUDE

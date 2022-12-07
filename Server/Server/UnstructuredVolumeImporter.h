/****************************************************************************/
/**
 *  @file UnstructuredVolumeImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE
#define PBVR__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE

#include "PbvrObjectUnstructuredVolume.h"
#include "ImporterBase.h"
#include "ClassName.h"
#include <kvs/Module>
#include "UnstructuredVolumeObject.h"
#include "SPLITTypeSubvolume.h"
#include "AggregateTypeSubvolume.h"
#include "StepAggregateTypeSubvolume.h"
#include <kvs/AVSUcd>
#include <kvs/AVSField>
#include "PbvrFilter.h"


namespace pbvr
{

/*==========================================================================*/
/**
 *  Unstructured volume object importer class.
 */
/*==========================================================================*/
class UnstructuredVolumeImporter:
    public kvs::ImporterBase,
    public pbvr::UnstructuredVolumeObject
{
    // Class name.
    kvsClassName( pbvr::UnstructuredVolumeImporter );

    // Module information.
    kvsModuleCategory( Importer );
    kvsModuleBaseClass( kvs::ImporterBase );
    kvsModuleSuperClass( pbvr::UnstructuredVolumeObject );

public:

    UnstructuredVolumeImporter();

    UnstructuredVolumeImporter( const std::string& filename );

    UnstructuredVolumeImporter( const kvs::FileFormatBase& file_format );

    UnstructuredVolumeImporter( const std::string& filename, const int fileType, const int st, const int vl );

    // add by @hira at 2016/12/01
    UnstructuredVolumeImporter( const PbvrFilterInformationFile *pbvr_fi, int step, int subvolume );

    virtual ~UnstructuredVolumeImporter();

public:

    SuperClass* exec( const kvs::FileFormatBase& file_format );

private:

    void import( const pbvr::PbvrObjectUnstructuredVolume& kvsml );

    void import( const pbvr::SPLITTypeSubvolume& kvsml );

    void import( const kvs::AVSUcd& ucd );

    void import( const kvs::AVSField& field );

    void import( const kvs::AggregateTypeSubvolume& gs );

    void import( const kvs::StepAggregateTypeSubvolume& gt );
};

} // end of namespace pbvr

#endif // KVS__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE

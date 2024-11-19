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

#include "ImporterBase.h"
#include "ClassName.h"
#include <vismodule/Module>
#include "UnstructuredVolumeObject.h"
#include "SPLITTypeSubvolume.h"
#include "AggregateTypeSubvolume.h"
#include "StepAggregateTypeSubvolume.h"
#include <vismodule/AVSUcd>
#include <vismodule/AVSField>


namespace pbvr
{

/*==========================================================================*/
/**
 *  Unstructured volume object importer class.
 */
/*==========================================================================*/
class UnstructuredVolumeImporter:
    public vismodule::ImporterBase,
    public pbvr::UnstructuredVolumeObject
{
    // Class name.
    visModuleClassName( pbvr::UnstructuredVolumeImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( pbvr::UnstructuredVolumeObject );

public:

    UnstructuredVolumeImporter();

    UnstructuredVolumeImporter( const std::string& filename );

    UnstructuredVolumeImporter( const vismodule::FileFormatBase& file_format );

    UnstructuredVolumeImporter( const std::string& filename, const int fileType, const int st, const int vl );

    virtual ~UnstructuredVolumeImporter();

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const pbvr::SPLITTypeSubvolume& kvsml );

    void import( const vismodule::AVSUcd& ucd );

    void import( const vismodule::AVSField& field );

    void import( const vismodule::AggregateTypeSubvolume& gs );

    void import( const vismodule::StepAggregateTypeSubvolume& gt );
};

} // end of namespace pbvr

#endif // VIS_MODULE__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE

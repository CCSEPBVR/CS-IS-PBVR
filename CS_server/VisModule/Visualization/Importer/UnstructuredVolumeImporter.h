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
#ifndef VIS_MODULE__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE
#define VIS_MODULE__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/SPLITTypeSubvolume>
#include <vismodule/AggregateTypeSubvolume>
#include <vismodule/StepAggregateTypeSubvolume>
#include <vismodule/AVSUcd>
#include <vismodule/AVSField>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Unstructured volume object importer class.
 */
/*==========================================================================*/
class UnstructuredVolumeImporter:
    public vismodule::ImporterBase,
    public vismodule::UnstructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::UnstructuredVolumeImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::UnstructuredVolumeObject );

public:

    UnstructuredVolumeImporter();

    UnstructuredVolumeImporter( const std::string& filename );

    UnstructuredVolumeImporter( const vismodule::FileFormatBase& file_format );

    UnstructuredVolumeImporter( const std::string& filename, const int fileType, const int st, const int vl );

    UnstructuredVolumeImporter( const std::string& filename, const int fileType, const int targetCellType, const int st, const int vl );

    virtual ~UnstructuredVolumeImporter();

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::SPLITTypeSubvolume& kvsml );

    void import( const vismodule::AVSUcd& ucd );

    void import( const vismodule::AVSField& field );

    void import( const vismodule::AggregateTypeSubvolume& gs );

    void import( const vismodule::StepAggregateTypeSubvolume& gt );
};

} // end of namespace vismodule

#endif // VIS_MODULE__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE

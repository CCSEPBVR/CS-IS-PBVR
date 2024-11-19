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

#include "ImporterBase.h"
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/KVSMLObjectUnstructuredVolume>
#include <vismodule/AVSUcd>
#include <vismodule/AVSField>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Unstructured volume object importer class.
 */
/*==========================================================================*/
class UnstructuredVolumeImporter
    : public vismodule::ImporterBase
    , public vismodule::UnstructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::UnstructuredVolumeImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::UnstructuredVolumeObject );

public:

    UnstructuredVolumeImporter( void );

    UnstructuredVolumeImporter( const std::string& filename );

    UnstructuredVolumeImporter( const vismodule::FileFormatBase* file_format );

    virtual ~UnstructuredVolumeImporter( void );

public:

    SuperClass* exec( const vismodule::FileFormatBase* file_format );

private:

    void import( const vismodule::KVSMLObjectUnstructuredVolume* const kvsml );

    void import( const vismodule::AVSUcd* const ucd );

    void import( const vismodule::AVSField* const field );

};

} // end of namespace vismodule

#endif // VIS_MODULE__UNSTRUCTURED_VOLUME_IMPORTER_H_INCLUDE

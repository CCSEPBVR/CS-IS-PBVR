/*****************************************************************************/
/**
 *  @file   UnstructuredVolumeExporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeExporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__UNSTRUCTURED_VOLUME_EXPORTER_H_INCLUDE
#define VIS_MODULE__UNSTRUCTURED_VOLUME_EXPORTER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/KVSMLObjectUnstructuredVolume>
#include <vismodule/ExporterBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  Unstructured volume exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class UnstructuredVolumeExporter : public vismodule::ExporterBase<FileFormatType>
{
    visModuleClassName( vismodule::UnstructuredVolumeExporter );

public:

    UnstructuredVolumeExporter( const vismodule::UnstructuredVolumeObject* object );

    FileFormatType* exec( const vismodule::ObjectBase* object );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a FileFormat data from given unstructured volume object.
 *  @param  object [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename FileFormatType>
UnstructuredVolumeExporter<FileFormatType>::UnstructuredVolumeExporter(
    const vismodule::UnstructuredVolumeObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports to a spcified FileFormat data.
 *  @param  object [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename FileFormatType>
FileFormatType* UnstructuredVolumeExporter<FileFormatType>::exec( const vismodule::ObjectBase* object )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Specialized structured volume exporter class for vismodule::KVSMLObjectUnstructuredVolume.
 */
/*===========================================================================*/
template <>
class UnstructuredVolumeExporter<vismodule::KVSMLObjectUnstructuredVolume> :
        public vismodule::ExporterBase<vismodule::KVSMLObjectUnstructuredVolume>
{
public:

    UnstructuredVolumeExporter( const vismodule::UnstructuredVolumeObject* object );

    vismodule::KVSMLObjectUnstructuredVolume* exec( const vismodule::ObjectBase* object );
};

} // end of namespace vismodule

#endif // VIS_MODULE__UNSTRUCTURED_VOLUME_EXPORTER_H_INCLUDE

/*****************************************************************************/
/**
 *  @file   StructuredVolumeExporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeExporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__STRUCTURED_VOLUME_EXPORTER_H_INCLUDE
#define VIS_MODULE__STRUCTURED_VOLUME_EXPORTER_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/KVSMLObjectStructuredVolume>
#include <vismodule/AVSField>
#include <vismodule/ExporterBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  Structured volume exporter class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class StructuredVolumeExporter : public vismodule::ExporterBase<FileFormatType>
{
    visModuleClassName( vismodule::StructuredVolumeExporter );

public:

    StructuredVolumeExporter( const vismodule::StructuredVolumeObject* object );

    FileFormatType* exec( const vismodule::ObjectBase* object );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a FileFormat data from given structured volume object.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
template <typename FileFormatType>
StructuredVolumeExporter<FileFormatType>::StructuredVolumeExporter(
    const vismodule::StructuredVolumeObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports to a spcified FileFormat data.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
template <typename FileFormatType>
FileFormatType* StructuredVolumeExporter<FileFormatType>::exec( const vismodule::ObjectBase* object )
{
    return( NULL );
}

/*===========================================================================*/
/**
 *  Specialized structured volume exporter class for vismodule::KVSMLObjectStructuredVolume.
 */
/*===========================================================================*/
template <>
class StructuredVolumeExporter<vismodule::KVSMLObjectStructuredVolume> :
        public vismodule::ExporterBase<vismodule::KVSMLObjectStructuredVolume>
{
public:

    StructuredVolumeExporter( const vismodule::StructuredVolumeObject* object );

    vismodule::KVSMLObjectStructuredVolume* exec( const vismodule::ObjectBase* object );
};

/*===========================================================================*/
/**
 *  Specialized structured volume exporter class for vismodule::AVSField.
 */
/*===========================================================================*/
template <>
class StructuredVolumeExporter<vismodule::AVSField> :
        public vismodule::ExporterBase<vismodule::AVSField>
{
public:

    StructuredVolumeExporter( const vismodule::StructuredVolumeObject* object );

    vismodule::AVSField* exec( const vismodule::ObjectBase* object );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STRUCTURED_VOLUME_EXPORTER_H_INCLUDE

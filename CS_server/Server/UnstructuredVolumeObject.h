/****************************************************************************/
/**
 *  @file UnstructuredVolumeObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVolumeObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define PBVR__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include "ClassName.h"
#include <kvs/Module>
#include "VolumeObjectBase.h"


namespace pbvr
{

/*==========================================================================*/
/**
 *  Unstructured volume object class.
 */
/*==========================================================================*/
class UnstructuredVolumeObject : public pbvr::VolumeObjectBase
{
    // Class name.
    kvsClassName( pbvr::UnstructuredVolumeObject );

    // Module information.
    typedef pbvr::ObjectBase::ModuleTag ModuleCategory;
    kvsModuleBaseClass( pbvr::VolumeObjectBase );

public:

    typedef kvs::ValueArray<kvs::UInt32> Connections;

private:

    CellType m_cell_type; ///< Cell type.
    size_t   m_nnodes;    ///< Number of nodes.
    size_t   m_ncells;    ///< Number of cells.

    Connections m_connections; ///< Connection ( Node ID ) array.

public:

    UnstructuredVolumeObject();

    UnstructuredVolumeObject(
        const CellType     cell_type,
        const size_t       nnodes,
        const size_t       ncells,
        const size_t       veclen,
        const Coords&      coords,
        const Connections& connections,
        const Values&      values );

    UnstructuredVolumeObject( const UnstructuredVolumeObject& other );

    virtual ~UnstructuredVolumeObject();

public:

    static pbvr::UnstructuredVolumeObject* DownCast( pbvr::ObjectBase* object );

    static const pbvr::UnstructuredVolumeObject* DownCast( const pbvr::ObjectBase& object );

public:

    UnstructuredVolumeObject& operator =( const UnstructuredVolumeObject& rhs );

    friend std::ostream& operator << ( std::ostream& os, const UnstructuredVolumeObject& object );

public:

    void shallowCopy( const UnstructuredVolumeObject& object );

    void deepCopy( const UnstructuredVolumeObject& object );

public:

    void setCellType( const CellType& cell_type );

    void setNNodes( const size_t nnodes );

    void setNCells( const size_t ncells );

    void setConnections( const Connections& connections );

public:

    const VolumeType volumeType() const;

    const GridType gridType() const;

    const CellType cellType() const;

    const size_t nnodes() const;

    const size_t ncells() const;

    const Connections& connections() const;

public:

    void updateMinMaxCoords();

private:

    void calculateMinMaxCoords();
};

} // end of namespace pbvr

#endif // KVS__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

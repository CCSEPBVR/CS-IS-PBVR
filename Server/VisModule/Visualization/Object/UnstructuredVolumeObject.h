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
#ifndef VIS_MODULE__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define VIS_MODULE__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/VolumeObjectBase>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Unstructured volume object class.
 */
/*==========================================================================*/
class UnstructuredVolumeObject : public vismodule::VolumeObjectBase
{
    // Class name.
    visModuleClassName( vismodule::UnstructuredVolumeObject );

    // Module information.
    typedef vismodule::ObjectBase::ModuleTag ModuleCategory;
    visModuleBaseClass( vismodule::VolumeObjectBase );

public:

    typedef vismodule::ValueArray<vismodule::UInt32> Connections;

private:

    CellType m_cell_type; ///< Cell type.
    std::size_t   m_nnodes;    ///< Number of nodes.
    std::size_t   m_ncells;    ///< Number of cells.

    Connections m_connections; ///< Connection ( Node ID ) array.

public:

    UnstructuredVolumeObject();

    UnstructuredVolumeObject(
        const CellType     cell_type,
        const std::size_t       nnodes,
        const std::size_t       ncells,
        const std::size_t       veclen,
        const Coords&      coords,
        const Connections& connections,
        const Values&      values );

    UnstructuredVolumeObject( const UnstructuredVolumeObject& other );

    virtual ~UnstructuredVolumeObject();

public:

    static vismodule::UnstructuredVolumeObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::UnstructuredVolumeObject* DownCast( const vismodule::ObjectBase& object );

public:

    UnstructuredVolumeObject& operator =( const UnstructuredVolumeObject& rhs );

    friend std::ostream& operator << ( std::ostream& os, const UnstructuredVolumeObject& object );

public:

    void shallowCopy( const UnstructuredVolumeObject& object );

    void deepCopy( const UnstructuredVolumeObject& object );

public:

    void setCellType( const CellType& cell_type );

    void setNNodes( const std::size_t nnodes );

    void setNCells( const std::size_t ncells );

    void setConnections( const Connections& connections );

public:

    const VolumeType volumeType() const;

    const GridType gridType() const;

    const CellType cellType() const;

    const std::size_t nnodes() const;

    const std::size_t ncells() const;

    const Connections& connections() const;

public:

    void updateMinMaxCoords();

private:

    void calculateMinMaxCoords();
};

} // end of namespace vismodule

#endif // VIS_MODULE__UNSTRUCTURED_VOLUME_OBJECT_H_INCLUDE

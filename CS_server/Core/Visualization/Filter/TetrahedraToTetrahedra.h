/*****************************************************************************/
/**
 *  @file   TetrahedraToTetrahedra.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TetrahedraToTetrahedra.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__TETRAHEDRA_TO_TETRAHEDRA_H_INCLUDE
#define KVS__TETRAHEDRA_TO_TETRAHEDRA_H_INCLUDE

#include <kvs/UnstructuredVolumeObject>
#include <kvs/FilterBase>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  TetrahedraToTetrahedra class.
 */
/*===========================================================================*/
class TetrahedraToTetrahedra : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    // Class name.
    kvsClassName( kvs::TetrahedraToTetrahedra );

    // Module information.
    kvsModuleCategory( Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );

public:

    enum Method
    {
        Subdivision8, ///< subdivide a quadratic tetrahedron into eight linear tetrahedra
        Removal       ///< remove the quadratic nodes
    };

private:

    Method m_method; ///< conversion method

public:

    TetrahedraToTetrahedra( void );

    TetrahedraToTetrahedra( const kvs::UnstructuredVolumeObject* volume, const Method method = Subdivision8 );

    virtual ~TetrahedraToTetrahedra( void );

public:

    SuperClass* exec( const kvs::ObjectBase* object );

public:

    void setMethod( const Method method );

protected:

    template <typename T>
    void remove_quadratic_nodes( const kvs::UnstructuredVolumeObject* volume );

    template <typename T>
    void subdivide_8_tetrahedra( const kvs::UnstructuredVolumeObject* volume );
};

} // end of namespace kvs

#endif // KVS__TETRAHEDRA_TO_TETRAHEDRA_H_INCLUDE

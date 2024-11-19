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
#ifndef VIS_MODULE__TETRAHEDRA_TO_TETRAHEDRA_H_INCLUDE
#define VIS_MODULE__TETRAHEDRA_TO_TETRAHEDRA_H_INCLUDE

#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/FilterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  TetrahedraToTetrahedra class.
 */
/*===========================================================================*/
class TetrahedraToTetrahedra : public vismodule::FilterBase, public vismodule::UnstructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::TetrahedraToTetrahedra );

    // Module information.
    visModuleCategory( Filter );
    visModuleBaseClass( vismodule::FilterBase );
    visModuleSuperClass( vismodule::UnstructuredVolumeObject );

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

    TetrahedraToTetrahedra( const vismodule::UnstructuredVolumeObject* volume, const Method method = Subdivision8 );

    virtual ~TetrahedraToTetrahedra( void );

public:

    SuperClass* exec( const vismodule::ObjectBase* object );

public:

    void setMethod( const Method method );

protected:

    template <typename T>
    void remove_quadratic_nodes( const vismodule::UnstructuredVolumeObject* volume );

    template <typename T>
    void subdivide_8_tetrahedra( const vismodule::UnstructuredVolumeObject* volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TETRAHEDRA_TO_TETRAHEDRA_H_INCLUDE

/*****************************************************************************/
/**
 *  @file   Streamline.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Streamline.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__STREAMLINE_H_INCLUDE
#define VIS_MODULE__STREAMLINE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include "StreamlineBase.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  Streamline class.
 */
/*===========================================================================*/
class Streamline : public vismodule::StreamlineBase
{
    // Class name.
    visModuleClassName( vismodule::Streamline );

    // Module information.
    visModuleCategory( Mapper );
    visModuleBaseClass( vismodule::StreamlineBase );

public:

    Streamline( void );

    Streamline(
        const vismodule::StructuredVolumeObject& volume,
        const vismodule::PointObject* seed_points,
        const vismodule::TransferFunction& transfer_function );

    virtual ~Streamline( void );

public:

    BaseClass::SuperClass* exec( const vismodule::ObjectBase& object );

protected:

    const bool check_for_acceptance( const std::vector<vismodule::Real32>& vertices );

    const bool check_for_termination(
        const vismodule::Vector3f& current_vertex,
        const vismodule::Vector3f& direction,
        const size_t integration_times,
        const vismodule::Vector3f& next_vertex );

    const vismodule::Vector3f interpolate_vector( const vismodule::Vector3f& vertex, const vismodule::Vector3f& direction );

    const vismodule::Vector3f calculate_vector( const vismodule::Vector3f& vertex );

    const vismodule::RGBColor calculate_color( const vismodule::Vector3f& direction );

    void set_min_max_vector_length( const vismodule::VolumeObjectBase& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STREAMLINE_BASE_H_INCLUDE

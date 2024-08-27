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
#ifndef KVS__STREAMLINE_H_INCLUDE
#define KVS__STREAMLINE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Module>
#include "StreamlineBase.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  Streamline class.
 */
/*===========================================================================*/
class Streamline : public kvs::StreamlineBase
{
    // Class name.
    kvsClassName( kvs::Streamline );

    // Module information.
    kvsModuleCategory( Mapper );
    kvsModuleBaseClass( kvs::StreamlineBase );

public:

    Streamline( void );

    Streamline(
        const kvs::StructuredVolumeObject* volume,
        const kvs::PointObject* seed_points,
        const kvs::TransferFunction& transfer_function );

    virtual ~Streamline( void );

public:

    BaseClass::SuperClass* exec( const kvs::ObjectBase* object );

protected:

    const bool check_for_acceptance( const std::vector<kvs::Real32>& vertices );

    const bool check_for_termination(
        const kvs::Vector3f& current_vertex,
        const kvs::Vector3f& direction,
        const size_t integration_times,
        const kvs::Vector3f& next_vertex );

    const kvs::Vector3f interpolate_vector( const kvs::Vector3f& vertex, const kvs::Vector3f& direction );

    const kvs::Vector3f calculate_vector( const kvs::Vector3f& vertex );

    const kvs::RGBColor calculate_color( const kvs::Vector3f& direction );

    void set_min_max_vector_length( const kvs::VolumeObjectBase* volume );
};

} // end of namespace kvs

#endif // KVS__STREAMLINE_BASE_H_INCLUDE

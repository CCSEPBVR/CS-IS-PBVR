/****************************************************************************/
/**
 *  @file VolumeRayIntersector.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VolumeRayIntersector.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__VOLUME_RAY_INTERSECTOR_H_INCLUDE
#define KVS__VOLUME_RAY_INTERSECTOR_H_INCLUDE

#include <kvs/Ray>
#include <kvs/Vector3>
#include <kvs/VolumeObjectBase>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  RayInUniformVolume class.
 */
/*==========================================================================*/
class VolumeRayIntersector : public kvs::Ray
{
    kvsClassName_without_virtual( kvs::VolumeRayIntersector );

private:

    kvs::Vector3f                m_vertex[8];
    const kvs::VolumeObjectBase* m_reference_volume;

public:

    VolumeRayIntersector( const kvs::VolumeObjectBase* volume );

public:

    const bool isIntersected( void )
    {
        return(
            kvs::Ray::isIntersected( m_vertex[0], m_vertex[3], m_vertex[2], m_vertex[1] ) ||
            kvs::Ray::isIntersected( m_vertex[0], m_vertex[1], m_vertex[5], m_vertex[4] ) ||
            kvs::Ray::isIntersected( m_vertex[1], m_vertex[2], m_vertex[6], m_vertex[5] ) ||
            kvs::Ray::isIntersected( m_vertex[2], m_vertex[3], m_vertex[7], m_vertex[6] ) ||
            kvs::Ray::isIntersected( m_vertex[3], m_vertex[0], m_vertex[4], m_vertex[7] ) ||
            kvs::Ray::isIntersected( m_vertex[4], m_vertex[5], m_vertex[6], m_vertex[7] ) );
    }

    const bool isInside( void ) const
    {
        const kvs::Vector3f point( this->point() );

        if ( m_vertex[0].z() < point.z() && point.z() < m_vertex[6].z() )
        {
            if ( m_vertex[0].y() < point.y() && point.y() < m_vertex[6].y() )
            {
                if ( m_vertex[0].x() < point.x() && point.x() < m_vertex[6].x() )
                {
                    return( true );
                }
            }
        }

        return( false );
    }

    void step( const float delta_t )
    {
        this->setT( this->t() + delta_t );
    }
};

} // end of namespace kvs

#endif // KVS__VOLUME_RAY_INTERSECTOR_H_INCLUDE

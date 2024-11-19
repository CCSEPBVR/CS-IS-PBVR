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
#ifndef VIS_MODULE__VOLUME_RAY_INTERSECTOR_H_INCLUDE
#define VIS_MODULE__VOLUME_RAY_INTERSECTOR_H_INCLUDE

#include <vismodule/Ray>
#include <vismodule/Vector3>
#include <vismodule/VolumeObjectBase>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  RayInUniformVolume class.
 */
/*==========================================================================*/
class VolumeRayIntersector : public vismodule::Ray
{
    visModuleClassName_without_virtual( vismodule::VolumeRayIntersector );

private:

    vismodule::Vector3f                m_vertex[8];
    const vismodule::VolumeObjectBase* m_reference_volume;

public:

    VolumeRayIntersector( const vismodule::VolumeObjectBase* volume );

public:

    const bool isIntersected( void )
    {
        return(
            vismodule::Ray::isIntersected( m_vertex[0], m_vertex[3], m_vertex[2], m_vertex[1] ) ||
            vismodule::Ray::isIntersected( m_vertex[0], m_vertex[1], m_vertex[5], m_vertex[4] ) ||
            vismodule::Ray::isIntersected( m_vertex[1], m_vertex[2], m_vertex[6], m_vertex[5] ) ||
            vismodule::Ray::isIntersected( m_vertex[2], m_vertex[3], m_vertex[7], m_vertex[6] ) ||
            vismodule::Ray::isIntersected( m_vertex[3], m_vertex[0], m_vertex[4], m_vertex[7] ) ||
            vismodule::Ray::isIntersected( m_vertex[4], m_vertex[5], m_vertex[6], m_vertex[7] ) );
    }

    const bool isInside( void ) const
    {
        const vismodule::Vector3f point( this->point() );

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

} // end of namespace vismodule

#endif // VIS_MODULE__VOLUME_RAY_INTERSECTOR_H_INCLUDE

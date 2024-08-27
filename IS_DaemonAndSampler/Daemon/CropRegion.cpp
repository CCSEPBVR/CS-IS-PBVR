#include "CropRegion.h"

CropRegion::CropRegion()
    : m_enabled( 0 )
{
}

CropRegion::CropRegion( const CropRegion& src )
{
    m_enabled = src.m_enabled;
    lower = src.lower;
    upper = src.upper;

    calc_max_min();
}

CropRegion::~CropRegion()
{
}

void CropRegion::set( const float& xmin, const float& ymin, const float& zmin,
                      const float& xmax, const float& ymax, const float& zmax )
{
    lower.set( xmin, ymin, zmin );
    upper.set( xmax, ymax, zmax );

    calc_max_min();
}

void CropRegion::set( const float* coords )
{
    lower.set( coords[0], coords[1], coords[2] );
    upper.set( coords[3], coords[4], coords[5] );

    calc_max_min();
}

void CropRegion::set_disable()
{
    m_enabled = false;
}
void CropRegion::set_enable()
{
    m_enabled = true;
}
void CropRegion::set_enable( const bool enable )
{
    if ( enable )
        m_enabled = 1;
}
void CropRegion::set_enable( const int enable )
{
    m_enabled = enable;
}

bool CropRegion::isenabled() const
{
    return m_enabled;
}

bool CropRegion::isinner( const kvs::Vector3f& coord ) const
{
    const float cx = coord.x();
    const float cy = coord.y();
    const float cz = coord.z();
    bool ret;

    if ( m_enabled == 1 )
    {
        ret =  ( lower.x() <= cx ) && ( cx <= upper.x() )
               && ( lower.y() <= cy ) && ( cy <= upper.y() )
               && ( lower.z() <= cz ) && ( cz <= upper.z() );
    }
    else if ( m_enabled == 2 )
    {
        const float x = lower.x() - cx;
        const float y = lower.y() - cy;
        const float z = lower.z() - cz;
        const float r = upper.x();

        ret =  ( x * x + y * y + z * z <= r * r );
    }
    else if ( m_enabled == 3 )
    {
        //pillor x-y coordinates
        const float x = lower.x() - cx;
        const float y = lower.y() - cy;
        const float z = lower.z();
        const float r = upper.x();
        const float h = upper.y();

        ret =  ( x * x + y * y <= r * r )
               && ( z <= cz ) && ( cz <= z + h  );
    }
    else if ( m_enabled == 4 )
    {
        //pillor y-z coordinates
        const float x = lower.x();
        const float y = lower.y() - cy;
        const float z = lower.z() - cz;
        const float r = upper.x();
        const float h = upper.y();

        ret =  ( y * y + z * z <= r * r )
               && ( x <= cx ) && ( cx <= x + h  );
    }
    else if ( m_enabled == 5 )
    {
        //pillor x-z coordinates
        const float x = lower.x() - cx;
        const float y = lower.y();
        const float z = lower.z() - cz;
        const float r = upper.x();
        const float h = upper.y();

        ret =  ( x * x + z * z <= r * r )
               && ( y <= cy ) && ( cy <= y + h  );
    }
    else
    {
        ret = true;
    }
    return ret;
}

CropRegion& CropRegion::operator= ( const CropRegion& src )
{
    m_enabled = src.m_enabled;
    lower = src.lower;
    upper = src.upper;

    m_max_coord = src.m_max_coord;
    m_min_coord = src.m_min_coord;

    return *this;
}

void CropRegion::calc_max_min()
{
    if ( m_enabled == 1 )
    {
        const float min_x = lower.x();
        const float min_y = lower.y();
        const float min_z = lower.z();
        const float max_x = upper.x();
        const float max_y = upper.y();
        const float max_z = upper.z();

        m_max_coord.set( max_x, max_y, max_z );
        m_min_coord.set( min_x, min_y, min_z );
    }
    else if ( m_enabled == 2 )
    {
        const float x = lower.x();
        const float y = lower.y();
        const float z = lower.z();
        const float r = upper.x();

        m_max_coord.set( x + r, y + r, z + r );
        m_min_coord.set( x - r, y - r, z - r );
    }
    else if ( m_enabled == 3 )
    {
        //pillor x-y coordinates
        const float x = lower.x();
        const float y = lower.y();
        const float z = lower.z();
        const float r = upper.x();
        const float h = upper.y();

        m_max_coord.set( x + r, y + r, z + h );
        m_min_coord.set( x - r, y - r, z );
    }
    else if ( m_enabled == 4 )
    {
        //pillor y-z coordinates
        const float x = lower.x();
        const float y = lower.y();
        const float z = lower.z();
        const float r = upper.x();
        const float h = upper.y();

        m_max_coord.set( x + h, y + r, z + r );
        m_min_coord.set( x    , y - r, z - r );
    }
    else if ( m_enabled == 5 )
    {
        //pillor x-z coordinates
        const float x = lower.x();
        const float y = lower.y();
        const float z = lower.z();
        const float r = upper.x();
        const float h = upper.y();

        m_max_coord.set( x + r, y + h, z + r );
        m_min_coord.set( x - r, y    , z - r );
    }
    else
    {
        const float min_x = lower.x();
        const float min_y = lower.y();
        const float min_z = lower.z();
        const float max_x = upper.x();
        const float max_y = upper.y();
        const float max_z = upper.z();

        m_max_coord.set( max_x, max_y, max_z );
        m_min_coord.set( min_x, min_y, min_z );
    }
}

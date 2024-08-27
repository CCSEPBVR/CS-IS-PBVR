#include "CropRegion.h"

CropRegion::CropRegion() :
    m_enabled( 0 )
{
}

CropRegion::CropRegion( const CropRegion& src )
{
    m_enabled = src.m_enabled;
    m_lower = src.m_lower;
    m_upper = src.m_upper;

    calculate_max_min();
}

CropRegion::~CropRegion()
{
}

void CropRegion::set( const float xmin, const float ymin, const float zmin,
                      const float xmax, const float ymax, const float zmax )
{
    m_lower.set( xmin, ymin, zmin );
    m_upper.set( xmax, ymax, zmax );

    calculate_max_min();
}

void CropRegion::set( const float* coords )
{
    m_lower.set( coords[0], coords[1], coords[2] );
    m_upper.set( coords[3], coords[4], coords[5] );

    calculate_max_min();
}

void CropRegion::setDisable()
{
    m_enabled = false;
}
void CropRegion::setEnable()
{
    m_enabled = true;
}
void CropRegion::setEnable( const bool enable )
{
    if ( enable )
        m_enabled = 1;
}
void CropRegion::setEnable( const int enable )
{
    m_enabled = enable;
}

bool CropRegion::isEnabled() const
{
    return m_enabled;
}

bool CropRegion::isInner( const kvs::Vector3f& coord ) const
{
    const float cx = coord.x();
    const float cy = coord.y();
    const float cz = coord.z();
    bool ret;

    if ( m_enabled == 1 )
    {
        ret =  ( m_lower.x() <= cx ) && ( cx <= m_upper.x() )
               && ( m_lower.y() <= cy ) && ( cy <= m_upper.y() )
               && ( m_lower.z() <= cz ) && ( cz <= m_upper.z() );
    }
    else if ( m_enabled == 2 )
    {
        const float x = m_lower.x() - cx;
        const float y = m_lower.y() - cy;
        const float z = m_lower.z() - cz;
        const float r = m_upper.x();

        ret =  ( x * x + y * y + z * z <= r * r );
    }
    else if ( m_enabled == 3 )
    {
        //pillor x-y coordinates
        const float x = m_lower.x() - cx;
        const float y = m_lower.y() - cy;
        const float z = m_lower.z();
        const float r = m_upper.x();
        const float h = m_upper.y();

        ret =  ( x * x + y * y <= r * r )
               && ( z <= cz ) && ( cz <= z + h  );
    }
    else if ( m_enabled == 4 )
    {
        //pillor y-z coordinates
        const float x = m_lower.x();
        const float y = m_lower.y() - cy;
        const float z = m_lower.z() - cz;
        const float r = m_upper.x();
        const float h = m_upper.y();

        ret =  ( y * y + z * z <= r * r )
               && ( x <= cx ) && ( cx <= x + h  );
    }
    else if ( m_enabled == 5 )
    {
        //pillor x-z coordinates
        const float x = m_lower.x() - cx;
        const float y = m_lower.y();
        const float z = m_lower.z() - cz;
        const float r = m_upper.x();
        const float h = m_upper.y();

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
    m_lower = src.m_lower;
    m_upper = src.m_upper;

    m_max_coord = src.m_max_coord;
    m_min_coord = src.m_min_coord;

    return *this;
}

void CropRegion::calculate_max_min()
{
    if ( m_enabled == 1 )
    {
        const float min_x = m_lower.x();
        const float min_y = m_lower.y();
        const float min_z = m_lower.z();
        const float max_x = m_upper.x();
        const float max_y = m_upper.y();
        const float max_z = m_upper.z();

        m_max_coord.set( max_x, max_y, max_z );
        m_min_coord.set( min_x, min_y, min_z );
    }
    else if ( m_enabled == 2 )
    {
        const float x = m_lower.x();
        const float y = m_lower.y();
        const float z = m_lower.z();
        const float r = m_upper.x();

        m_max_coord.set( x + r, y + r, z + r );
        m_min_coord.set( x - r, y - r, z - r );
    }
    else if ( m_enabled == 3 )
    {
        //pillor x-y coordinates
        const float x = m_lower.x();
        const float y = m_lower.y();
        const float z = m_lower.z();
        const float r = m_upper.x();
        const float h = m_upper.y();

        m_max_coord.set( x + r, y + r, z + h );
        m_min_coord.set( x - r, y - r, z );
    }
    else if ( m_enabled == 4 )
    {
        //pillor y-z coordinates
        const float x = m_lower.x();
        const float y = m_lower.y();
        const float z = m_lower.z();
        const float r = m_upper.x();
        const float h = m_upper.y();

        m_max_coord.set( x + h, y + r, z + r );
        m_min_coord.set( x    , y - r, z - r );
    }
    else if ( m_enabled == 5 )
    {
        //pillor x-z coordinates
        const float x = m_lower.x();
        const float y = m_lower.y();
        const float z = m_lower.z();
        const float r = m_upper.x();
        const float h = m_upper.y();

        m_max_coord.set( x + r, y + h, z + r );
        m_min_coord.set( x - r, y    , z - r );
    }
    else
    {
        const float min_x = m_lower.x();
        const float min_y = m_lower.y();
        const float min_z = m_lower.z();
        const float max_x = m_upper.x();
        const float max_y = m_upper.y();
        const float max_z = m_upper.z();

        m_max_coord.set( max_x, max_y, max_z );
        m_min_coord.set( min_x, min_y, min_z );
    }
}

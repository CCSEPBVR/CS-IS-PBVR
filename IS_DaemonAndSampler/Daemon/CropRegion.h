#ifndef __CROP_REGION_H__
#define __CROP_REGION_H__

#include <kvs/Vector3>

class CropRegion
{
    kvs::Vector3f lower, upper;
    int m_enabled;
    kvs::Vector3f m_max_coord;
    kvs::Vector3f m_min_coord;
    void calc_max_min();

public:
    CropRegion();
    CropRegion( const CropRegion& src );
    ~CropRegion();

    void set( const float& xmin, const float& ymin, const float& zmin,
              const float& xmax, const float& ymax, const float& zmax );
    void set( const float* coods );

    void set_disable();
    void set_enable();
    void set_enable( const bool enable );
    void set_enable( const int enable );

    bool isenabled() const;
    bool isinner( const kvs::Vector3f& coord )   const;
    CropRegion& operator= ( const CropRegion& src );

    kvs::Vector3f get_max_coord()
    {
        return ( m_max_coord );
    };

    kvs::Vector3f get_min_coord()
    {
        return ( m_min_coord );
    };

    kvs::Vector3f get_lower()
    {
        return ( lower );
    };

    kvs::Vector3f get_upper()
    {
        return ( upper );
    };
};

#endif // __CROP_REGION_H__


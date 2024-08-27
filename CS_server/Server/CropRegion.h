#ifndef PBVR__CROP_REGION_H_INCLUDE
#define PBVR__CROP_REGION_H_INCLUDE

#include <kvs/Vector3>

class CropRegion
{
    kvs::Vector3f m_lower, m_upper;
    int m_enabled;
    kvs::Vector3f m_max_coord;
    kvs::Vector3f m_min_coord;
    void calculate_max_min();

public:
    CropRegion();
    CropRegion( const CropRegion& src );
    ~CropRegion();

    void set( const float xmin, const float ymin, const float zmin,
              const float xmax, const float ymax, const float zmax );
    void set( const float* coods );

    void setDisable();
    void setEnable();
    void setEnable( const bool enable );
    void setEnable( const int enable );

    bool isEnabled() const;
    bool isInner( const kvs::Vector3f& coord )   const;
    CropRegion& operator= ( const CropRegion& src );

    kvs::Vector3f getMaxCoord()
    {
        return m_max_coord;
    };

    kvs::Vector3f getMinCoord()
    {
        return m_min_coord;
    };

    kvs::Vector3f getLower()
    {
        return m_lower;
    };

    kvs::Vector3f getUpper()
    {
        return m_upper;
    };
};

#endif // PBVR__CROP_REGION_H_INCLUDE


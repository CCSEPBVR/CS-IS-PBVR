#ifndef PBVR__CROP_REGION_H_INCLUDE
#define PBVR__CROP_REGION_H_INCLUDE

#include <vismodule/Vector3>

class CropRegion
{
    vismodule::Vector3f m_lower, m_upper;
    int m_enabled;
    vismodule::Vector3f m_max_coord;
    vismodule::Vector3f m_min_coord;
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
    bool isInner( const vismodule::Vector3f& coord )   const;
    CropRegion& operator= ( const CropRegion& src );

    vismodule::Vector3f getMaxCoord()
    {
        return m_max_coord;
    };

    vismodule::Vector3f getMinCoord()
    {
        return m_min_coord;
    };

    vismodule::Vector3f getLower()
    {
        return m_lower;
    };

    vismodule::Vector3f getUpper()
    {
        return m_upper;
    };
};

#endif // PBVR__CROP_REGION_H_INCLUDE


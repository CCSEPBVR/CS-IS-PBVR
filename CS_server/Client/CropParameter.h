#ifndef PBVR__KVS__VISCLIENT__CROP_PARAMETER_H_INCLUDE
#define PBVR__KVS__VISCLIENT__CROP_PARAMETER_H_INCLUDE

namespace kvs
{
namespace visclient
{

class CropParameter
{

public:
    enum CropType
    {
        NoCrop    = 0,
        Crop      = 1,
        Sphere    = 2,
        Pillar    = 3,
        PillarYZ  = 4,
        PillarXZ  = 5
    };

public:
    CropType m_crop_type;

    float m_region_crop_x_min_level;
    float m_region_crop_x_max_level;
    float m_region_crop_y_min_level;
    float m_region_crop_y_max_level;
    float m_region_crop_z_min_level;
    float m_region_crop_z_max_level;

    float m_region_sphere_radius;
    float m_region_sphere_center_x;
    float m_region_sphere_center_y;
    float m_region_sphere_center_z;

    float m_region_pillar_radius;
    float m_region_pillar_height;
    float m_region_pillar_center_x;
    float m_region_pillar_center_y;
    float m_region_pillar_center_z;

public:
    CropParameter() :
        m_crop_type( NoCrop )
    {
        m_region_crop_x_min_level = 0.0;
        m_region_crop_x_max_level = 0.0;
        m_region_crop_y_min_level = 0.0;
        m_region_crop_y_max_level = 0.0;
        m_region_crop_z_min_level = 0.0;
        m_region_crop_z_max_level = 0.0;

        m_region_sphere_radius  = 0.0;
        m_region_sphere_center_x = 0.0;
        m_region_sphere_center_y = 0.0;
        m_region_sphere_center_z = 0.0;

        m_region_pillar_radius  = 0.0;
        m_region_pillar_height  = 0.0;
        m_region_pillar_center_x = 0.0;
        m_region_pillar_center_y = 0.0;
        m_region_pillar_center_z = 0.0;
    }

    bool operator==( const CropParameter& s ) const
    {
        const float epsilon = 1e-6f;
        bool res = true;

        res &= ( m_crop_type == s.m_crop_type );

        res &= std::abs( m_region_crop_x_min_level - s.m_region_crop_x_min_level ) < epsilon;
        res &= std::abs( m_region_crop_x_max_level - s.m_region_crop_x_max_level ) < epsilon;
        res &= std::abs( m_region_crop_y_min_level - s.m_region_crop_y_min_level ) < epsilon;
        res &= std::abs( m_region_crop_y_max_level - s.m_region_crop_y_max_level ) < epsilon;
        res &= std::abs( m_region_crop_z_min_level - s.m_region_crop_z_min_level ) < epsilon;
        res &= std::abs( m_region_crop_z_max_level - s.m_region_crop_z_max_level ) < epsilon;

        res &= std::abs( m_region_sphere_radius  - s.m_region_sphere_radius ) < epsilon;
        res &= std::abs( m_region_sphere_center_x - s.m_region_sphere_center_x ) < epsilon;
        res &= std::abs( m_region_sphere_center_y - s.m_region_sphere_center_y ) < epsilon;
        res &= std::abs( m_region_sphere_center_z - s.m_region_sphere_center_z ) < epsilon;

        res &= std::abs( m_region_pillar_radius  - s.m_region_pillar_radius ) < epsilon;
        res &= std::abs( m_region_pillar_height  - s.m_region_pillar_height ) < epsilon;
        res &= std::abs( m_region_pillar_center_x - s.m_region_pillar_center_x ) < epsilon;
        res &= std::abs( m_region_pillar_center_y - s.m_region_pillar_center_y ) < epsilon;
        res &= std::abs( m_region_pillar_center_z - s.m_region_pillar_center_z ) < epsilon;

        return res;
    }
};

}
}

#endif // PBVR__KVS__VISCLIENT__CROP_PARAMETER_H_INCLUDE


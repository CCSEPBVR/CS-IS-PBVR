#ifndef __CROP_PARAM_H__
#define __CROP_PARAM_H__

namespace kvs
{
namespace visclient
{

struct CropParam
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
    CropType cropType;

    float rgcxminlevel;
    float rgcxmaxlevel;
    float rgcyminlevel;
    float rgcymaxlevel;
    float rgczminlevel;
    float rgczmaxlevel;

    float rgsradius;
    float rgscenterx;
    float rgscentery;
    float rgscenterz;

    float rgpradius;
    float rgpheight;
    float rgpcenterx;
    float rgpcentery;
    float rgpcenterz;

public:
    CropParam() :
        cropType( NoCrop )
    {
        rgcxminlevel = 0.0;
        rgcxmaxlevel = 0.0;
        rgcyminlevel = 0.0;
        rgcymaxlevel = 0.0;
        rgczminlevel = 0.0;
        rgczmaxlevel = 0.0;

        rgsradius  = 0.0;
        rgscenterx = 0.0;
        rgscentery = 0.0;
        rgscenterz = 0.0;

        rgpradius  = 0.0;
        rgpheight  = 0.0;
        rgpcenterx = 0.0;
        rgpcentery = 0.0;
        rgpcenterz = 0.0;
    }

    bool operator==( const CropParam& s ) const
    {
        const float epsilon = 1e-6;
        bool res = true;

        res &= ( cropType == s.cropType );

        res &= std::abs( rgcxminlevel - s.rgcxminlevel ) < epsilon;
        res &= std::abs( rgcxmaxlevel - s.rgcxmaxlevel ) < epsilon;
        res &= std::abs( rgcyminlevel - s.rgcyminlevel ) < epsilon;
        res &= std::abs( rgcymaxlevel - s.rgcymaxlevel ) < epsilon;
        res &= std::abs( rgczminlevel - s.rgczminlevel ) < epsilon;
        res &= std::abs( rgczmaxlevel - s.rgczmaxlevel ) < epsilon;

        res &= std::abs( rgsradius  - s.rgsradius ) < epsilon;
        res &= std::abs( rgscenterx - s.rgscenterx ) < epsilon;
        res &= std::abs( rgscentery - s.rgscentery ) < epsilon;
        res &= std::abs( rgscenterz - s.rgscenterz ) < epsilon;

        res &= std::abs( rgpradius  - s.rgpradius ) < epsilon;
        res &= std::abs( rgpheight  - s.rgpheight ) < epsilon;
        res &= std::abs( rgpcenterx - s.rgpcenterx ) < epsilon;
        res &= std::abs( rgpcentery - s.rgpcentery ) < epsilon;
        res &= std::abs( rgpcenterz - s.rgpcenterz ) < epsilon;

        return res;
    }
};

}
}

#endif // __CROP_PARAM_H__


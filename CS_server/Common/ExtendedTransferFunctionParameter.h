#ifndef __EXTEND_TRANSFER_FUNCTION_H__
#define __EXTEND_TRANSFER_FUNCTION_H__

#include <kvs/TransferFunction>
#include "Types.h"

class ExtendedTransferFunctionParameter : public kvs::TransferFunction
{

public:
    int32_t Resolution;
    std::string EquationR;
    std::string EquationG;
    std::string EquationB;
    std::string EquationA;
    float ColorVarMin;
    float ColorVarMax;
    float OpacityVarMin;
    float OpacityVarMax;

public:
    ExtendedTransferFunctionParameter() :
        Resolution( 256 ),
        EquationR( "" ), EquationG( "" ),
        EquationB( "" ), EquationA( "" ),
        ColorVarMin( 0.0 ), ColorVarMax( 1.0 ),
        OpacityVarMin( 0.0 ), OpacityVarMax( 1.0 )
    {
    }

    void setResolution( size_t resolution )
    {
        Resolution = resolution;

        kvs::ColorMap color_map( resolution, 0.0, 1.0 );
        kvs::OpacityMap opacity_map( resolution, 0.0, 1.0 );

        const kvs::ColorMap::Table color_table = this->colorMap().table();
        const kvs::OpacityMap::Table opacity_table = this->opacityMap().table();

        const size_t n_color   = color_table.size() / 3;
        const size_t n_opacity = opacity_table.size();

        const float color_stride = 1.0 / ( n_color - 1 );
        const float opacity_stride = 1.0 / ( n_opacity - 1 );

        for ( size_t n = 0; n < n_color; n++ )
        {
            const float x = color_stride * n;
            const kvs::UInt8 r = color_table.at( 3 * n + 0 );
            const kvs::UInt8 g = color_table.at( 3 * n + 1 );
            const kvs::UInt8 b = color_table.at( 3 * n + 2 );
            const kvs::RGBColor color( r, g, b );
            color_map.addPoint( x, color );
        }

        for ( size_t n = 0; n < n_opacity; n++ )
        {
            const float x = opacity_stride * n;
            const float opacity = opacity_table.at( n );
            opacity_map.addPoint( x, opacity );
        }

        color_map.create();
        opacity_map.create();

        kvs::TransferFunction tf( color_map, opacity_map );
        *( static_cast<kvs::TransferFunction*>( this ) ) = tf;
    }

    virtual bool operator==( const ExtendedTransferFunctionParameter& s ) const;
};

class NamedTransferFunctionParameter : public ExtendedTransferFunctionParameter
{
public:
    enum Selection
    {
        SelectExtendTransferFunction = 0,
        SelectTransferFunction = 1,
    };

public:
    Selection selection;

    std::string Name;
    std::string ColorVar;
    std::string OpacityVar;
    bool m_range_initialized;		// add by @hira at 2016/12/01

public:
    NamedTransferFunctionParameter() :
        selection( SelectTransferFunction ),
        Name( "" ), ColorVar( "" ), OpacityVar( "" )
    {
    }
    virtual bool operator==( const NamedTransferFunctionParameter& s ) const;
	static bool compareName(const NamedTransferFunctionParameter& left, const NamedTransferFunctionParameter& right);
    int getNameNumber() const;
};



#endif // __EXTEND_TRANSFER_FUNCTION_H__


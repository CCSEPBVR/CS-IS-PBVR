#include "ExtendedTransferFunctionParameter.h"

bool ExtendedTransferFunctionParameter::operator==( const ExtendedTransferFunctionParameter& s ) const
{
    bool v0 = true;

    v0 &= ( this->Resolution == s.Resolution );
    v0 &= ( this->ColorVarMin == s.ColorVarMin );
    v0 &= ( this->ColorVarMax == s.ColorVarMax );
    v0 &= ( this->OpacityVarMin == s.OpacityVarMin );
    v0 &= ( this->OpacityVarMax == s.OpacityVarMax );

    const size_t n_color   = this->colorMap().table().size();
    const size_t n_opacity = this->opacityMap().table().size();

    v0 &= ( n_color == s.colorMap().table().size() );
    v0 &= ( n_opacity == s.opacityMap().table().size() );

    if ( v0 )
    {
        for ( size_t n = 0; n < n_color; n++ )
        {
            const kvs::UInt8 r0 = this->colorMap().table().at( n );
            const kvs::UInt8 r1 = s.colorMap().table().at( n );
            v0 &= ( r0 == r1 );
        }
        for ( size_t n = 0; n < n_opacity; n++ )
        {
            const float r0 = this->opacityMap().table().at( n );
            const float r1 = s.opacityMap().table().at( n );
            v0 &= ( r0 == r1 );
        }
    }

    return v0;
}

bool NamedTransferFunctionParameter::operator==( const NamedTransferFunctionParameter& s ) const
{
    bool v0 = true;

    v0 &= ( this->Name == s.Name );
    v0 &= ( this->ColorVar == s.ColorVar );
    v0 &= ( this->OpacityVar == s.OpacityVar );

    if ( v0 )
    {
        v0 &= ExtendedTransferFunctionParameter::operator==( s );
    }

    return v0;
}


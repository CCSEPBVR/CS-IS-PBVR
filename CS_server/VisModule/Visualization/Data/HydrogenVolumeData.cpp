/*****************************************************************************/
/**
 *  @file   HydrogenVolumeData.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HydrogenVolumeData.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "HydrogenVolumeData.h"
#include <vismodule/AnyValueArray>
#include <vismodule/Vector3>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new HydrogenVolumeData class.
 *  @param  resolution [in] volume resolution
 */
/*===========================================================================*/
HydrogenVolumeData::HydrogenVolumeData( const vismodule::Vector3ui resolution )
{
    SuperClass::setResolution( resolution );
    this->exec();
}

/*===========================================================================*/
/**
 *  @brief  Destructs the HydrogenVolumeData class.
 */
/*===========================================================================*/
HydrogenVolumeData::~HydrogenVolumeData( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new HydrogenVolumeData class.
 */
/*===========================================================================*/
HydrogenVolumeData::HydrogenVolumeData( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Main routine of the hydrogen volume data generator.
 *  @return pointer to the structured volume object
 */
/*===========================================================================*/
HydrogenVolumeData::SuperClass* HydrogenVolumeData::exec( void )
{
    const vismodule::UInt64 dim1 = SuperClass::resolution().x();
    const vismodule::UInt64 dim2 = SuperClass::resolution().y();
    const vismodule::UInt64 dim3 = SuperClass::resolution().z();

    const vismodule::Real64 kr1 = 32.0 / dim1;
    const vismodule::Real64 kr2 = 32.0 / dim2;
    const vismodule::Real64 kr3 = 32.0 / dim3;
    const vismodule::Real64 kd = 6.0;

    vismodule::AnyValueArray values;
    if ( !values.allocate<vismodule::UInt8>( static_cast<size_t>( dim1 * dim2 * dim3 ) ) )
    {
        visModuleMessageError("Cannot allocate memory for the value.");
        return( this );
    }

    vismodule::UInt8* pvalues = values.pointer<vismodule::UInt8>();
    vismodule::UInt64 index = 0;
    for ( vismodule::UInt64 z = 0; z < dim3; ++z )
    {
        const vismodule::Real64 dz = kr3 * ( z - ( dim3 / 2.0 ) );
        for ( vismodule::UInt64 y = 0; y < dim2; ++y )
        {
            const vismodule::Real64 dy = kr2 * ( y - ( dim2 / 2.0 ) );
            for ( vismodule::UInt64 x = 0; x < dim1; ++x )
            {
                const vismodule::Real64 dx = kr1 * ( x - ( dim1 / 2.0 ));
                const vismodule::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz );
                const vismodule::Real64 cos_theta = dz / r;
                const vismodule::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );

                vismodule::Real64 c = phi * phi;
                if ( c > 255.0 ) { c = 255.0; }

                pvalues[ index++ ] = static_cast<vismodule::UInt8>( c );
            }
        }
    }

    SuperClass::setGridType( vismodule::StructuredVolumeObject::Uniform );
    SuperClass::setVeclen( 1 );
    SuperClass::setValues( values );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();

    return( this );
}

} // end of namespace vismodule

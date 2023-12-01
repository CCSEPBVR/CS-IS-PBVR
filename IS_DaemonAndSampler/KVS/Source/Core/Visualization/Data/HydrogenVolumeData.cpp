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
#include <kvs/AnyValueArray>
#include <kvs/Vector3>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new HydrogenVolumeData class.
 *  @param  resolution [in] volume resolution
 */
/*===========================================================================*/
HydrogenVolumeData::HydrogenVolumeData( const kvs::Vector3ui resolution )
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
    const kvs::UInt64 dim1 = SuperClass::resolution().x();
    const kvs::UInt64 dim2 = SuperClass::resolution().y();
    const kvs::UInt64 dim3 = SuperClass::resolution().z();

    const kvs::Real64 kr1 = 32.0 / dim1;
    const kvs::Real64 kr2 = 32.0 / dim2;
    const kvs::Real64 kr3 = 32.0 / dim3;
    const kvs::Real64 kd = 6.0;

    kvs::AnyValueArray values;
    if ( !values.allocate<kvs::UInt8>( static_cast<size_t>( dim1 * dim2 * dim3 ) ) )
    {
        kvsMessageError("Cannot allocate memory for the value.");
        return( this );
    }

    kvs::UInt8* pvalues = values.pointer<kvs::UInt8>();
    kvs::UInt64 index = 0;
    for ( kvs::UInt64 z = 0; z < dim3; ++z )
    {
        const kvs::Real64 dz = kr3 * ( z - ( dim3 / 2.0 ) );
        for ( kvs::UInt64 y = 0; y < dim2; ++y )
        {
            const kvs::Real64 dy = kr2 * ( y - ( dim2 / 2.0 ) );
            for ( kvs::UInt64 x = 0; x < dim1; ++x )
            {
                const kvs::Real64 dx = kr1 * ( x - ( dim1 / 2.0 ));
                const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz );
                const kvs::Real64 cos_theta = dz / r;
                const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );

                kvs::Real64 c = phi * phi;
                if ( c > 255.0 ) { c = 255.0; }

                pvalues[ index++ ] = static_cast<kvs::UInt8>( c );
            }
        }
    }

    SuperClass::setGridType( kvs::StructuredVolumeObject::Uniform );
    SuperClass::setVeclen( 1 );
    SuperClass::setValues( values );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();

    return( this );
}

} // end of namespace kvs

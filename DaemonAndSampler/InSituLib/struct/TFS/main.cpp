#include <kvs/Message>
#include <kvs/StructuredVolumeObject>
//#include <kvs/StructuredVolumeImporter>
//#include <kvs/RayCastingRenderer>
#include <kvs/HydrogenVolumeData>
//#include <kvs/glut/Application>
//#include <kvs/glut/Screen>
#include "TrilinearInterpolator.h"
#include <kvs/TrilinearInterpolator>
#include <kvs/Vector3>

int main( int argc, char** argv )
{
    //kvs::glut::Application app( argc, argv );

    kvs::StructuredVolumeObject* object = new kvs::HydrogenVolumeData( kvs::Vector3ui( 32, 32, 32 ) );

    TFS::TrilinearInterpolator interp( object );

    float p_x[SIMDW], p_y[SIMDW], p_z[SIMDW];

    for( int i = 0; i < SIMDW; i++ )
    {
        float s = i==0 ? 1 : i==SIMDW-1 ? SIMDW-2 : i;
        p_x[i] = s / 2.0;
        p_y[i] = 10;
        p_z[i] = 16;
    }

    interp.attachPoint( p_x, p_y, p_z );

    float scalars[SIMDW];

    interp.scalar<kvs::UInt8>( scalars );

    float g_x[SIMDW];
    float g_y[SIMDW];
    float g_z[SIMDW];

    interp.gradient<kvs::UInt8>( g_x, g_y, g_z );

    for(int i = 0; i < SIMDW; i++ )
    {
        std::cout<<scalars[i]<<":"<<g_x[i]<<","<<g_y[i]<<","<<g_z[i]<<std::endl;
    }

    return 0;
}


/*****************************************************************************/
/**
 *  @file   main.cpp
 *  @brief  Example program Support OpenXR.
 *  @author Keisuke Tajiri
 */
 /*****************************************************************************/
#include <kvs/qt/Application>
#include <kvs/openxr/Screen>
#include <kvs/PolygonImporter>
#include <kvs/PolygonObject>
#include <kvs/PointImporter>
#include <kvs/PolygonRenderer>
#include <kvs/PointRenderer>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticRenderingCompositor>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/LineObject>
#include <kvs/Streamline>
#include <kvs/TornadoVolumeData>
#include <kvs/Isosurface>
#include <kvs/HydrogenVolumeData>
#include <kvs/Tubeline>
#include <kvs/ExternalFaces>
#include <kvs/Message>
#include <cstdlib>

kvs::PointObject* CreatePointObject()
{
    const size_t N = 4;
    kvs::Real32 CoordArray[N * 3] = {
        1.0, 0.0, 2.0,
        2.0, 0.0, 2.0,
        1.5, 0.8, 1.7,
        1.5, 0.0, 1.1
    };

    kvs::UInt8 ColorArray[N * 3] = {
        255,   0,   0,
        0, 255,   0,
        0,   0, 255,
        255, 255,   0
    };

    kvs::ValueArray<kvs::Real32> coords( CoordArray, N * 3 );
    kvs::ValueArray<kvs::UInt8> colors( ColorArray, N * 3 );


    kvs::PointObject* object = new kvs::PointObject();
    object->setCoords( coords );
    object->setColors( colors );
    object->setSize( 10 );

    return object;
}

kvs::StructuredVolumeObject* CreateStructuredVolumeObject()
{
    const size_t dimx = 3;
    const size_t dimy = 3;
    const size_t dimz = 3;
    const size_t veclen = 1;

    kvs::UInt8 ValueArray[dimx * dimy * dimz * veclen] = {
        0,  10,    0,
        50, 255, 200,
        0,  100,   0,

        50,  0, 150,
        0,   0,   0,
        150, 0,  50,

        0, 255,  0,
        50, 10, 50,
        0, 255,  0
    };

    kvs::ValueArray<kvs::UInt8> values( ValueArray, sizeof( ValueArray ) );
    kvs::StructuredVolumeObject* object = new kvs::StructuredVolumeObject();
    object->setGridType(kvs::StructuredVolumeObject::Uniform);
    object->setVeclen( veclen );
    object->setResolution( kvs::Vector3ui( dimx, dimy, dimz ) );
    object->setValues( kvs::AnyValueArray( values ) );
    return object;
}

kvs::UnstructuredVolumeObject* CreateUnstructuredVolumeObject()
{
    const size_t nnodes = 7;
    const size_t ncells = 5;
    const size_t veclen = 1;

    kvs::Real32 CoordArray[nnodes * 3] = {
        0.0,  7.0,  7.0,
        5.0, 14.0,  7.0,
        9.0,  7.0,  7.0,
        4.0,  9.0, 14.0,
        4.0,  9.0,  0.0,
        5.0,  4.0,  4.0,
        4.0,  0.0,  9.0
    };

    kvs::Real32 ValueArray[nnodes * veclen] = {
        0.8,
        0.3,
        0.4,
        0.4,
        0.2,
        0.5,
        0.3
    };

    kvs::UInt32 ConnectionArray[ncells * 4] = {
        0, 1, 2, 3,
        0, 1, 2, 4,
        0, 2, 4, 5,
        0, 2, 5, 6,
        0, 2, 3, 6
    };

    kvs::ValueArray<kvs::Real32> coords( CoordArray, sizeof( CoordArray ) / sizeof( kvs::Real32 ) );
    kvs::ValueArray<kvs::UInt32> connections( ConnectionArray, sizeof( ConnectionArray ) / sizeof( kvs::UInt32 ) );
    kvs::ValueArray<kvs::Real32> values( ValueArray, sizeof( ValueArray ) / sizeof( kvs::Real32 ) );

    kvs::UnstructuredVolumeObject* object = new kvs::UnstructuredVolumeObject();
    object->setCellType( kvs::UnstructuredVolumeObject::Tetrahedra );
    object->setVeclen( veclen );
    object->setNumberOfNodes( nnodes );
    object->setNumberOfCells( ncells );
    object->setCoords( coords );
    object->setConnections( connections );
    object->setValues( values );

    return object;
}


/*===========================================================================*/
/**
 *  @brief  Main function.
 *  @param  argc [i] argument count
 *  @param  argv [i] argument values
 *  @return true, if the main process is done succesfully
 */
 /*===========================================================================*/
int main(int argc, char** argv)
{
    kvs::qt::Application app(argc, argv);
    kvs::openxr::Screen screen(&app);
    screen.setGeometry(0, 0, 512, 512);
    screen.setTitle("KVS OpenXR Sample Application");

    const std::string filename = "bunny.ply";

#if 1
    // PolygonRenderer
    {
        auto* object = new kvs::PolygonImporter( filename );
        auto* renderer = new kvs::PolygonRenderer();
        screen.registerObject( object, renderer);
        screen.reset();
    }
#endif

#if 0 
    // PointRenderer
    {
        auto* object = CreatePointObject();
        auto* renderer = new kvs::PointRenderer();
        screen.registerObject( object, renderer );
        screen.reset();
    }
#endif

#if 0
    // StocasticPolygonRenderer
    {
       kvs::StochasticRenderingCompositor compositor( screen.scene() );
       compositor.setRepetitionLevel( 4 );
       compositor.setLODControlEnabled( true );
       screen.setEvent( &compositor );
      
        kvs::PolygonObject* object = new kvs::PolygonImporter( filename );
        auto* renderer = new kvs::StochasticPolygonRenderer();
        screen.registerObject( object, renderer );
        screen.reset();
    }
#endif

#if 0
    // Isosurface
    {
        auto* volume = new kvs::HydrogenVolumeData( { 64, 64, 64 } );
        const auto i = ( volume->maxValue() + volume->minValue() ) * 0.5;
        const auto n = kvs::PolygonObject::VertexNormal;
        const auto d = false;
        const auto t = kvs::TransferFunction( 256 );
        auto* object = new kvs::Isosurface( volume, i, n, d, t );
        delete volume;
        screen.registerObject( object );
        screen.reset();
    }
#endif

#if 0
    // StreamLine
    {
        auto* volume = new kvs::TornadoVolumeData( { 32, 32, 32 } );
        const kvs::Vec3i min_coord( 15, 15, 0 );
        const kvs::Vec3i max_coord( 20, 20, 30 );
        auto* point = new kvs::PointObject;
        point->setCoords([&]
            {
                std::vector<kvs::Real32> v;
                for (int k = min_coord.z(); k < max_coord.z(); k++)
                {
                    for (int j = min_coord.y(); j < max_coord.y(); j++)
                    {
                        for (int i = min_coord.x(); i < max_coord.x(); i++)
                        {
                            v.push_back(static_cast<kvs::Real32>(i));
                            v.push_back(static_cast<kvs::Real32>(j));
                            v.push_back(static_cast<kvs::Real32>(k));
                        }
                    }
                }
                return kvs::ValueArray<kvs::Real32>(v);
            } ());

        const auto tfunc = kvs::TransferFunction(256);
        auto* object = new kvs::Streamline( volume, point, tfunc );
        delete point;
        delete volume;

        screen.registerObject( object );
        screen.reset();
    }
#endif

#if 0
    // TubeLine
    {
        auto* volume = new kvs::TornadoVolumeData({ 16, 16, 16 });
        const kvs::Vec3i min_coord(7, 7, 0);
        const kvs::Vec3i max_coord(10, 10, 15);
        auto* point = new kvs::PointObject;
        point->setCoords([&]
            {
                std::vector<kvs::Real32> v;
                for (int k = min_coord.z(); k < max_coord.z(); k++)
                {
                    for (int j = min_coord.y(); j < max_coord.y(); j++)
                    {
                        for (int i = min_coord.x(); i < max_coord.x(); i++)
                        {
                            v.push_back(static_cast<kvs::Real32>(i));
                            v.push_back(static_cast<kvs::Real32>(j));
                            v.push_back(static_cast<kvs::Real32>(k));
                        }
                    }
                }
                return kvs::ValueArray<kvs::Real32>(v);
            } ());

        const auto tfunc = kvs::TransferFunction(256);
        auto* line = new kvs::Streamline(volume, point, tfunc);
        line->setSize(0.2);
        delete point;
        delete volume;

        const size_t ndivisions = 20;
        auto* object = new kvs::Tubeline(line, ndivisions);
        delete line;

        screen.registerObject(object);
        screen.reset();
    }
#endif

#if 0
    // StructuredVolumeObject
    {
        auto* volume = CreateStructuredVolumeObject();
        auto* object = new kvs::ExternalFaces(volume);
        delete volume;
        screen.registerObject(object);
    }
#endif

#if 0
    // UnStructuredVolumeObject
    {
        auto* volume = CreateUnstructuredVolumeObject();
        auto* object = new kvs::ExternalFaces(volume);
        delete volume;
        screen.registerObject(object);
        screen.reset();
    }
#endif

    screen.create();
    return app.run();
}

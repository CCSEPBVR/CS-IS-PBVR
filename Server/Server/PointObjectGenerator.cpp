#include <sys/stat.h>
#include "PointObjectGenerator.h"
//#include <sys/time.h>
#include "TransferFunction.h"
#include "UnstructuredVolumeObject.h"
#include "UnstructuredVolumeImporter.h"
#include "CellByCellUniformSampling.h"
#include "CellByCellRejectionSampling.h"
#include "CellByCellMetropolisSampling.h"
//#include "CellByCellLayeredSampling.h"
#include <kvs/Camera>
#include "CellByCellUniformSampling.h"
#include "CellByCellRejectionSampling.h"
#include "CellByCellMetropolisSampling.h"
#if 0 //TEST_DELETE
#include <kvs/TestVolume>
#include <kvs/FrontSTRFileReader>
#endif
#include <kvs/AVSUcd>
#include "common.h"
#include <kvs/ValueArray>
#include <kvs/File>

#include "Argument.h"
#ifdef KMATH
#include <kmath_random.h>
#endif

#include "timer_simple.h"

using namespace pbvr;
#ifdef KMATH
extern KMATH_Random km_random;
#endif

void PointObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera, const size_t subpixel_level, const float sampling_step )
{
//FJ_TIMER_KAWAMURA
    PBVR_TIMER_STA( 260 );
//FJ_TIMER_KAWAMURA

    // modify by @hira at 2016/12/01 : main.cppから移動
    struct stat st;
    if ( stat( param.m_input_data.c_str(), &st ) )
    {
        std::cout << "Error. read failed:" << param.m_input_data << std::endl;
        exit( 1 );
    }

    delete m_object;
    pbvr::UnstructuredVolumeObject* volume;
    volume = new pbvr::UnstructuredVolumeImporter( param.m_input_data );
    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
    }

//FJ_TIMER_KAWAMURA
    PBVR_TIMER_END( 260 );
//FJ_TIMER_KAWAMURA

    volume->setMinMaxValues( m_fi->m_min_value, m_fi->m_max_value );
    volume->setMinMaxObjectCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    try
    {
        m_object = sampling( param, camera, volume, subpixel_level, sampling_step );
    }
    catch ( const std::runtime_error& e )
    {
#ifdef _DEBUG		// debug by @hira
        printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
        m_object = NULL;
        delete volume;
        throw e;
    }

    delete volume;
}

void PointObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl )
{
    PBVR_TIMER_STA( 260 );
    delete m_object;
    pbvr::UnstructuredVolumeObject* volume;

    // volume = new pbvr::UnstructuredVolumeImporter( param.input_data );
    kvs::File ifpx( m_fi->m_file_path );
    std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();

    volume = new pbvr::UnstructuredVolumeImporter( path_base, m_fi->m_file_type, st, vl );
    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
    }

    PBVR_TIMER_END( 260 );

    volume->setMinMaxValues( m_fi->m_min_value, m_fi->m_max_value );
    volume->setMinMaxObjectCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    try
    {
        m_object = sampling( param, camera, volume, subpixel_level, sampling_step );
    }
    catch ( const std::runtime_error& e )
    {
#ifdef _DEBUG		// debug by @hira
        printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
        m_object = NULL;
        delete volume;
        throw e;
    }

    delete volume;
}

std::string PointObjectGenerator::getErrorMessage( const size_t maxMemory ) const
{
    std::string errorMessage( "" );
    const size_t totalMemory = sizeof( float ) * m_object->nvertices() + sizeof( float ) * m_object->nnormals() + sizeof( char ) * m_object->ncolors();
    if ( totalMemory > maxMemory )
    {
        char ms[512];
        sprintf( ms, "Memory Error: TotalMemory = %zd, MaxMemory = %zd\n", totalMemory, maxMemory );
        errorMessage += ms;
    }
    return errorMessage;
}

pbvr::PointObject* PointObjectGenerator::sampling( const Argument& param, const kvs::Camera& camera, pbvr::VolumeObjectBase* volume, const size_t subpixel_level, const float sampling_step )
{
    pbvr::TransferFunction tf = param.m_transfer_function;

//    tf.setRange(volume);
//    param.m_transfunc_synthesizer->transfunc_map()["t1"] = tf;
//    param.m_transfunc_synthesizer->create();

    // volume calculate test.
#if 0 //TEST_DELETE
    if ( param.m_test_volume )
    {
        return new kvs::TestVolume( &camera, volume, m_subpixel_level, sampling_step, tf );
    }
#endif

#ifdef CPU_SAMPLING_TIME
    std::cout << std::endl << "CPU - ";
#else
    std::cout << std::endl << "GPU - ";
#endif
    switch ( param.m_sampling_method )
    {
    case 'u':
        std::cout << "Uniform sampling" << std::endl;
        return new pbvr::CellByCellUniformSampling( camera, *volume, subpixel_level, sampling_step, tf,
                                                    param.m_transfunc_synthesizer, param.m_normal_ingredient, param.m_crop,
                                                    param.m_particle_density, param.m_batch );
    case 'r':
        std::cout << "Rejection sampling" << std::endl;
        return new pbvr::CellByCellRejectionSampling( camera, *volume, subpixel_level, sampling_step, tf,
                                                      param.m_transfunc_synthesizer, param.m_crop,
                                                      param.m_particle_density, param.m_batch );
    case 'm':
        std::cout << "Metropolis sampling" << std::endl;
        return new pbvr::CellByCellMetropolisSampling( camera, *volume, subpixel_level, sampling_step, tf,
                                                       param.m_transfunc_synthesizer, param.m_normal_ingredient, param.m_crop,
                                                       param.m_particle_density, param.m_batch );
    default:
        std::cout << "Unknown sampling method:" << param.m_sampling_method << std::endl;
        exit( 1 );
        return 0;
    }
}

#include <vismodule/PointObjectGenerator>
//#include <sys/time.h>
#include <vismodule/TransferFunction>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
//#include "CellByCellLayeredSampling.h"
#include <vismodule/Camera>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellHistogram>
#if 0 //TEST_DELETE
#include <vismodule/TestVolume>
#include <vismodule/FrontSTRFileReader>
#endif
#include <vismodule/AVSUcd>
#include <vismodule/timer_simple>
#include <vismodule/ValueArray>
#include <vismodule/File>

#include <vismodule/FileChecker>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/StructuredVolumeImporter>

#include <vismodule/Argument>

#include <vismodule/timer_simple>

using namespace vismodule;

void PointObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const size_t subpixel_level, const float sampling_step )
{
//FJ_TIMER_KAWAMURA
    VIS_MODULE_TIMER_STA( 260 );
//FJ_TIMER_KAWAMURA

    delete m_object;

    // add by shimomura 2023/0407
    vismodule::VolumeObjectBase* volume = nullptr;
    if ( vismoduleview::FileChecker::ImportableStructuredVolume( param.m_input_data ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new vismodule::StructuredVolumeImporter( param.m_input_data ); 
        int id = param.m_subvolume_id;
        // change by shimomura 20240730
        volume->updateMinMaxValues();
        //volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
        volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[id], m_mvp->m_max_subvolume_coord[id] );
        volume->setMinMaxExternalCoords( m_mvp->m_min_subvolume_coord[id], m_mvp->m_max_subvolume_coord[id] );

    } 
    else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( param.m_input_data))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );  
        
        // change by shimomura 20240730
        volume->updateMinMaxValues();
        //volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
        volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
        volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
    }
    else 
    {
        visModuleMessageError("%s is not volume data.", param.m_input_data.c_str());
    }

    //vismodule::UnstructuredVolumeObject* volume;
    //volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );
    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
        volume->setCoordSynthesizerTokens( m_coord_synthesizer_tokens );
    }

//FJ_TIMER_KAWAMURA
    VIS_MODULE_TIMER_END( 260 );
//FJ_TIMER_KAWAMURA
    
    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

   try
    {
        m_object = sampling( param, camera, *volume, subpixel_level, sampling_step );
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

void PointObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl )
{
    VIS_MODULE_TIMER_STA( 260 );
    delete m_object;

    size_t found_kvsml = param.m_input_data_base.find(".kvsml");
    size_t found_vtm = param.m_input_data_base.find(".vtm");
    size_t found_vtu = param.m_input_data_base.find(".vtu");
    size_t found_vti = param.m_input_data_base.find(".vti");
    size_t found_inp = param.m_input_data_base.find(".inp");

    vismodule::VolumeObjectBase* volume = nullptr;

    if ( found_kvsml != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );
    
        vismodule::File ifpx( m_mvp->m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
    
        volume = new vismodule::UnstructuredVolumeImporter( path_base, m_mvp->m_file_type, st, vl );
    }
#ifdef EXTEND_FILE_FORMAT
    else if ( found_vtm != std::string::npos )
    {
        // structured
        if( m_mvp->m_file_type == 3 )
        {
            volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
        }
        // unstructured
        if( m_mvp->m_file_type == 4 )
        {
            volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
        }
    }
    else if ( found_vtu != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
    }
    else if ( found_vti != std::string::npos )
    {
        volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
    }
    else if ( found_inp != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
    }
#endif

    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
        volume->setCoordSynthesizerTokens( m_coord_synthesizer_tokens );
    }

    VIS_MODULE_TIMER_END( 260 );

    volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
    volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    try
    {
        m_object = sampling( param, camera, *volume, subpixel_level, sampling_step );
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

vismodule::PointObject* PointObjectGenerator::sampling( const Argument& param, const vismodule::Camera& camera, vismodule::VolumeObjectBase& volume, const size_t subpixel_level, const float sampling_step )
{
#ifndef CPU_VER
    int rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
#endif

    vismodule::TransferFunction tf = param.m_transfer_function;
    std::vector<vismodule::TransferFunction> tf_array = param.m_transfunc_array;


    // volume calculate test.
#if 0 //TEST_DELETE
    if ( param.m_test_volume )
    {
        return new vismodule::TestVolume( &camera, volume, m_subpixel_level, sampling_step, tf );
    }
#endif

#ifdef CPU_SAMPLING_TIME
    std::cout << std::endl << "CPU - ";
#else
    std::cout << std::endl << "GPU - ";
#endif
    switch ( param.m_sampling_method )
    {
// CO by shimomura 2022/12/21
    case 'u':
        std::cout << "Uniform sampling" << std::endl;
        return new vismodule::CellByCellUniformSampling( camera, volume, subpixel_level, sampling_step, tf, tf_array,
                                                    param.m_transfunc_synthesizer, param.m_normal_ingredient, param.m_crop,
                                                    param.m_particle_density, param.m_batch );
    case 'r':
        std::cout << "Rejection sampling" << std::endl;
        return new vismodule::CellByCellRejectionSampling( camera, volume, subpixel_level, sampling_step, tf, tf_array,
                                                      param.m_transfunc_synthesizer, param.m_crop,
                                                      param.m_particle_density, param.m_batch );
    case 'm':
        std::cout << "Metropolis sampling" << std::endl;
        return new vismodule::CellByCellMetropolisSampling( camera, volume, subpixel_level, sampling_step, tf, tf_array,
                                                       param.m_transfunc_synthesizer, param.m_normal_ingredient, param.m_crop,
                                                       param.m_particle_density, param.m_batch );
    case 'h':
        std::cout << "Histogram " << std::endl;
        return new vismodule::CellByCellHistogram( camera, volume, subpixel_level, sampling_step, tf, tf_array,
                                                       param.m_transfunc_synthesizer, param.m_normal_ingredient, param.m_crop,
                                                       param.m_particle_density, param.m_batch );

    default:
        std::cout << "Unknown sampling method:" << param.m_sampling_method << std::endl;
        exit( 1 );
        return 0;
    }
}

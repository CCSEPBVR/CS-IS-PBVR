#include <vismodule/CS_PointObjectGenerator>
//#include <sys/time.h>
#include <vismodule/TransferFunction>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/UnstructuredVolumeImporter>
//#include "CellByCellLayeredSampling.h"
#include <vismodule/Camera>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellHistogram>
#include <vismodule/CellByCellParticleGenerator>
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


vismodule::PointObject* CS_PointObjectGenerator::run( const Argument& param, const vismodule::Camera& camera, const int timeStep,  const int st )
{
    this ->setFilterInfo( m_mvp );
    this ->setCoordSynthTS( st );

    struct stat s;
    if ( stat( param.m_input_data.c_str(), &s ) )
    {
        std::cout << "Error. read failed:" << param.m_input_data << std::endl;
        exit( 1 );
    }
    this->createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step );

    vismodule::PointObject* po = this->getPointObject();
    return po;
}

vismodule::PointObject*  CS_PointObjectGenerator::run( const Argument& param, const vismodule::Camera& camera, const int timeStep, const int st, const int vl)
{
    this->setFilterInfo( m_mvp );
    this->setCoordSynthTS( st );
    this->createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step, st, vl );
    vismodule::PointObject* po = this->getPointObject();
    return po;
}


void CS_PointObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const size_t subpixel_level, const float sampling_step )
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

void CS_PointObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl )
{
    VIS_MODULE_TIMER_STA( 260 );
    delete m_object;

    size_t found_kvsml = param.m_input_data_base.find(".kvsml");
    size_t found_vtm   = param.m_input_data_base.find(".vtm");
    size_t found_vtu   = param.m_input_data_base.find(".vtu");
    size_t found_vti   = param.m_input_data_base.find(".vti");
    size_t found_inp   = param.m_input_data_base.find(".inp");
    size_t found_pvtu  = param.m_input_data_base.find(".pvtu");
    size_t found_case  = param.m_input_data_base.find(".case");

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
    else if ( found_vtu  != std::string::npos ||
              found_inp  != std::string::npos ||
              found_pvtu != std::string::npos ||
              found_case != std::string::npos
            )
    {
        volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
    }
    else if ( found_vti != std::string::npos )
    {
        volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
    }
#endif

    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
        volume->setCoordSynthesizerTokens( m_coord_synthesizer_tokens );
    }

    VIS_MODULE_TIMER_END( 260 );

    // .vtm .pvtu .case file format
    if ( ( found_vtm != std::string::npos ) || ( found_pvtu != std::string::npos ) || ( found_case != std::string::npos ) )
    {
        // Structured Volume Data
        if ( m_mvp->m_file_type == 3 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );
            volume->setMinMaxExternalCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );
        }

        // Unstructured Volume Data
        else if ( m_mvp->m_file_type == 4 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
            volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
        }
    }
    else
    {
        volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
        volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
        volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
    }

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    try
    {
        m_object = sampling( param, camera, *volume, subpixel_level, sampling_step );
        std::cout << __LINE__ <<__FUNCTION__ << std::endl;
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

        std::cout << __LINE__ <<__FUNCTION__ << std::endl;
    delete volume;
}

std::string CS_PointObjectGenerator::getErrorMessage( const size_t maxMemory ) const
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

vismodule::PointObject* CS_PointObjectGenerator::sampling( const Argument& param, const vismodule::Camera& camera, vismodule::VolumeObjectBase& volume, const size_t subpixel_level, const float sampling_step )
{
#ifndef CPU_VER
    int rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
#endif

    vismodule::TransferFunction tf = param.m_transfer_function;
    std::vector<vismodule::TransferFunction> tf_array = param.m_transfunc_array;
    vismodule::VolumeObjectBase::VolumeType voltype = volume.volumeType();
    float max_opacity;
    float max_density;
    float sampling_volume_inverse;

    CellByCellParticleGenerator::CalculateDensityConstaint(
            camera,
            volume,
            static_cast<float>( subpixel_level ),
            sampling_step,
            &sampling_volume_inverse,
            &max_opacity,
            &max_density );

    param.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
    param.m_transfunc_synthesizer->setMaxDensity( max_density );
    param.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

        //詰め替え処理
        vismodule::AnyValueArray valueArray; 
        valueArray = volume.values(); 
        int nnodes = volume.nnodes();
        int nvariables = volume.veclen();

        // ここで変数の値をfloatでまとめることで粒子生成のテンプレート化を回避
        std::unique_ptr<std::unique_ptr<Type[]>[]> values(new std::unique_ptr<Type[]>[nvariables]);

        // 実行時型分岐で呼び出す
        const std::type_info& type = volume.values().typeInfo()->type();
        if (type == typeid(vismodule::Int8))
        {
            copy_values<vismodule::Int8>(valueArray, values, nvariables, nnodes);
        }  
        else if ( type == typeid( vismodule::Int16  ) )
        {
            copy_values<vismodule::Int16>(valueArray, values, nvariables, nnodes);
        } 
        else if ( type == typeid( vismodule::Int32  ) )
        {
            copy_values<vismodule::Int32>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::Int64  ) )
        {
            copy_values<vismodule::Int64>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::UInt8  ) )
        {
            copy_values<vismodule::UInt8>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::UInt16 ) )
        {
            copy_values<vismodule::UInt16>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::UInt32 ) )
        {
            copy_values<vismodule::UInt32>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::UInt64 ) )
        {
            copy_values<vismodule::UInt64>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::Real32 ) )
        {
            copy_values<vismodule::Real32>(valueArray, values, nvariables, nnodes);
        }
        else if ( type == typeid( vismodule::Real64 ) )
        {
            copy_values<vismodule::Real64>(valueArray, values, nvariables, nnodes);
        }
        else 
        {
            throw std::runtime_error("Unsupported type");
        }


    if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Unstructured)
    {
        //詰め替え処理
        std::vector<float> coordinates; 
        int ncoords;
        std::vector<unsigned int> connections ;
        int ncells; 
        vismodule::VolumeObjectBase::CellType celltype;

        const vismodule::UnstructuredVolumeObject* uvo_p = static_cast<const vismodule::UnstructuredVolumeObject*>( &volume );
       
        coordinates.assign( (float * )volume.coords().begin(),(float * )volume.coords().end()); 
        ncoords =  volume.nnodes();
        connections.assign((unsigned int*)uvo_p->connections().begin(), (unsigned int*)uvo_p->connections().end());
        ncells = uvo_p->ncells();
        celltype = uvo_p->cellType();

        // 一時的に raw pointer の配列を作る
        std::vector<float*> raw_values(nvariables);
        for (int j = 0; j < nvariables; ++j) 
        {
            raw_values[j] = values[j].get();
        }
    
        domain_parameters_unstruct dom={
         volume.minObjectCoord().x()
        ,volume.minObjectCoord().y()
        ,volume.minObjectCoord().z()
        ,volume.maxObjectCoord().x()
        ,volume.maxObjectCoord().y()
        ,volume.maxObjectCoord().z()
        };


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
        return new vismodule::CellByCellUniformSampling( camera, dom, raw_values.data(), nvariables,
                   coordinates.data(), ncoords, connections.data(), ncells, celltype, subpixel_level, sampling_step, tf, tf_array,
                                                      param.m_transfunc_synthesizer,
                                                      param.m_particle_density , &m_coord_synthesizer_strings);

    case 'r':
        std::cout << "Rejection sampling" << std::endl;
        return new vismodule::CellByCellRejectionSampling( camera, dom, raw_values.data(), nvariables,
                coordinates.data(), ncoords, connections.data(), ncells, celltype, subpixel_level, sampling_step, tf, tf_array,
                                                      param.m_transfunc_synthesizer,
                                                      param.m_particle_density,&m_coord_synthesizer_strings );
    case 'm':
        std::cout << "Metropolis sampling" << std::endl;
        return new vismodule::CellByCellMetropolisSampling( camera, dom, raw_values.data(), nvariables,
                coordinates.data(), ncoords, connections.data(), ncells, celltype, subpixel_level, sampling_step, tf, tf_array,
                                                      param.m_transfunc_synthesizer,
                                                      param.m_particle_density,&m_coord_synthesizer_strings );

    case 'h':
        std::cout << "Histogram " << std::endl;
        return new vismodule::CellByCellHistogram( camera, dom, raw_values.data(), nvariables,
                coordinates.data(), ncoords, connections.data(), ncells, celltype, subpixel_level, sampling_step, tf, tf_array,
                                                      param.m_transfunc_synthesizer,
                                                      param.m_particle_density,&m_coord_synthesizer_strings );


    default:
        std::cout << "Unknown sampling method:" << param.m_sampling_method << std::endl;
        exit( 1 );
        return 0;
    }
    }

    if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Structured)
    {
# if 1
        // 詰め替え処理
        const vismodule::StructuredVolumeObject* svo_p = static_cast<const vismodule::StructuredVolumeObject*>( &volume );
        
        // 一時的に raw pointer の配列を作る
        std::vector<float*> raw_values(nvariables);
        for (int j = 0; j < nvariables; ++j) 
        {
            raw_values[j] = values[j].get();
        }

        int resol[3] = { svo_p->resolution().x(), svo_p->resolution().y(), svo_p->resolution().z()};
        domain_parameters_struct dom={
         volume.minObjectCoord().x()
        ,volume.minObjectCoord().y()
        ,volume.minObjectCoord().z()
        ,volume.maxObjectCoord().x()
        ,volume.maxObjectCoord().y()
        ,volume.maxObjectCoord().z()
        ,resol
        ,1.f
        };


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
        return new vismodule::CellByCellUniformSampling( camera, dom, raw_values.data(), nvariables, 
                                                    subpixel_level, sampling_step, tf, tf_array,
                                                    param.m_transfunc_synthesizer, 
                                                    param.m_particle_density , &m_coord_synthesizer_strings);
    case 'r':
        std::cout << "Rejection sampling" << std::endl;
        return new vismodule::CellByCellRejectionSampling( camera, dom, raw_values.data(), nvariables, 
                                                    subpixel_level, sampling_step, tf, tf_array,
                                                    param.m_transfunc_synthesizer, 
                                                    param.m_particle_density , &m_coord_synthesizer_strings);

    case 'm':
        std::cout << "Metropolis sampling" << std::endl;
        return new vismodule::CellByCellMetropolisSampling( camera, dom, raw_values.data(), nvariables, 
                                                    subpixel_level, sampling_step, tf, tf_array,
                                                    param.m_transfunc_synthesizer, 
                                                    param.m_particle_density , &m_coord_synthesizer_strings);
    case 'h':
        std::cout << "Histogram " << std::endl;
        return new vismodule::CellByCellHistogram( camera, dom, raw_values.data(), nvariables, 
                                                    subpixel_level, sampling_step, tf, tf_array,
                                                    param.m_transfunc_synthesizer, 
                                                    param.m_particle_density , &m_coord_synthesizer_strings);


    default:
        std::cout << "Unknown sampling method:" << param.m_sampling_method << std::endl;
        exit( 1 );
        return 0;
    }
#endif
    }

}

template <typename T>
void CS_PointObjectGenerator::copy_values(vismodule::AnyValueArray& valueArray, std::unique_ptr<std::unique_ptr<Type[]>[]>& values, int nvariables, int nnodes) 
{
    for (int j = 0; j < nvariables; j++) 
    {
        values[j] = std::make_unique<Type[]>(nnodes);
        for (int i = 0; i < nnodes; i++) 
        {
            int it = j * nnodes + i;
            values[j][i] = valueArray.at<T>(it);
        }
    }
}

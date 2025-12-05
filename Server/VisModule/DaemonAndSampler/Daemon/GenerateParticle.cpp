#include <vismodule/GenerateParticle>
#include <vismodule/VariableRange>
#include <vismodule/Calculate>
#include <vismodule/SetDefaultTransferFunction>
#include <vismodule/PointObjectGenerator>
#include <vismodule/FileChecker>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/ParticleMonitor>

bool SetParticleParameterCS(
    const std::string file_name,
    const int time_step,
    vismodule::Camera* camera,
    ParticleProperty& param,
    MultiVolumePropertyList& mvpl
)
{
    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    std::cout << "time_step = " << time_step << std::endl;

    param.m_time_step                = time_step;
    param.m_level_index              = 1;
    param.m_repeat_level             = 1;
    param.m_sampling_method          = 'u';
    param.m_camera                   = camera;
    // param.m_x_synthesis              = clntMes.m_x_synthesis;
    // param.m_y_synthesis              = clntMes.m_y_synthesis;
    // param.m_z_synthesis              = clntMes.m_z_synthesis;
    param.m_particle_data_size_limit = 10;
    param.filepath                   = file_name;
    param.m_particle_limit           = 10000000;
    param.m_particle_density         = 1;
    param.m_latency_threshold        = -1.0;
    param.m_job_id_pack_size         = 1;

    // 初回通信の場合, どのように判断するのかは保留
    if ( true )
    {
        mvpl.searchFile(param);

        if ( mvpl.m_total_start_steps > 0 )
        {
            param.m_time_step = mvpl.m_total_start_steps;
        }

        if ( mvpl.m_list.size() <= 0 )
        {
            if ( rank == 0 )
            {
                std::cerr << "Error: pfifile doesn't exist(rank:" << rank << ")" << std::endl;
            }
            return false;
        }
    }

    // 初回通信の場合, どのように判断するのかは保留
    if ( true )
    {
        std::cout << "default parameter " << std::endl;
        VariableRange range = Calculate_minmax( param, mvpl );
        setDefalutTransferFunctionToArgument( param, range, mvpl.m_total_number_ingredients );
    }
    else
    {
        /* 一旦保留
        std::cout << "user define parameter " << std::endl;
        transfunc_creator.setProtocol( clntMes );
        setClientTransferFunctionToArgument( &param, clntMes );
        */
    }

    /*
    param.m_transfunc_synthesizer = transfunc_creator.create();
    param.m_transfunc_array.resize( transfunc_creator.transfunc().size() );

    for(int i = 0; i < transfunc_creator.transfunc().size(); i++ )
    {
        param.m_transfunc_array[i] = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
    }
    */

    param.m_sampling_step  = CalculateSamplingStep( mvpl );
    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *param.m_camera );

    return true;
}

void GenerateParticleCS(
    ParticleProperty& param,
    MultiVolumePropertyList& mvpl,
    std::unique_ptr<kvs::PointObject>& point_object
    // jpv::ParticleTransferServer pts,
    // jpv::ServerMode server_mode,
    // jpv::InitializeParameter init_param
)
{
    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    int st, vl, wid = 0;
    int tf_number;
    VariableRange vr; // jobcollectorで使用
    JobDispatcher jd;
    bool nan_error = false;

#ifndef CPU_VER
    JobCollector jc;
#endif

    tf_number = param.m_transfunc_array.size();

    std::cout << "param.m_time_step:" << param.m_time_step << std::endl;

    /* 一時保留
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity

    for ( size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
    }

    for ( int i = 0; i < (tf_number * 2); i++ )
    {
        tmp_max[i] = FLT_MIN;
        tmp_min[i] = FLT_MAX;
    }
    */

    jd.initialize(
        param.m_time_step,
        param.m_time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        param.m_latency_threshold,
        param.m_job_id_pack_size
    );

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        vismodule::PointObject* recv_obj = new vismodule::PointObject;
        vismodule::PointObject* send_obj = nullptr;

        // make point object and histgram and range
        if ( ( rank > 0 ) || ( mpi_size == 1 ) )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( param.filepath, st, xvl );

            // generate point object start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                vismodule::PointObjectGenerator point_object_generator;

                /* 保留
                point_object_generator.setCoordSynthStr(
                    param.m_x_synthesis,
                    param.m_y_synthesis,
                    param.m_z_synthesis
                );
                */

                // generate volume object
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    generate_volume( param, mvp, st, xvl, volume );
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( param, mvp, st, xvl, volume );
                }
                else // filetype: kvsml
                {
                    generate_volume( param, mvp, st, volume );
                }

                if ( !volume )
                {
                    throw std::runtime_error("Failed to generate volume object.");
                }

                std::unique_ptr<std::unique_ptr<Type[]>[]> values;
                int nvariables = 0;
                int ncoords = 0;
                vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();

                if( voltype == vismodule::VolumeObjectBase::VolumeType::Unstructured )
                {
                    domain_parameters_unstruct dom;
                    std::unique_ptr<float[]> coordinates;
                    std::unique_ptr<unsigned int[]> connections;
                    int ncells = 0;
                    vismodule::VolumeObjectBase::CellType celltype;

                    store_volume_in_variables_array_unstruct( volume, dom, values, nvariables, coordinates, ncoords, connections, ncells, celltype );

                    float max_opacity;
                    float max_density;
                    float sampling_volume_inverse;
                        
                    vismodule::CellByCellParticleGenerator::CalculateDensityConstaint(
                        *param.m_camera,
                        *volume,
                        static_cast<float>( param.m_subpixel_level ),
                        param.m_sampling_step,
                        &sampling_volume_inverse,
                        &max_opacity,
                        &max_density
                    );

                    param.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
                    param.m_transfunc_synthesizer->setMaxDensity( max_density );
                    param.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }

                    // generate particle
                    send_obj = point_object_generator.GenerateParticleUnstruct( param, dom, raw_pointers_vector.data(), nvariables, coordinates.get(), ncoords, connections.get(), ncells, celltype );
                }
                else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                {
                    domain_parameters_struct dom; 

                    store_volume_in_variables_array_struct( volume, dom, values, nvariables, ncoords );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }                        

                    // generate particle
                    send_obj = point_object_generator.GenerateParticleStruct( param, dom, raw_pointers_vector.data(), nvariables );
                }

                delete volume;
            }
            catch ( const std::runtime_error& e )
            {
#ifdef _DEBUG // debug by @hira
                printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                std::cerr << e.what();
                nan_error = true;
            }
            // generate point object end

            /* 一時保留
            MakeHistgram( send_obj, tf_number, tmp_c_bins, tmp_o_bins );
            MakeParticleMinMax( param.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );
            */
        } // make point object and histgram and range

#ifndef CPU_VER
        if ( mpi_size > 1 ) {
            if ( rank == 0 )
            {
                jc.jobCollect( recv_obj, &vr, &nan_error, &wid );
            }
            else
            {
                jc.jobCollect( send_obj, &vr, &nan_error, &wid );
            }
        }
#else
        size_t nmemb = send_obj->nvertices() * 3;
        vismodule::ValueArray<vismodule::Real32> vismodule_coords ( send_obj->coords().pointer() , nmemb );
        vismodule::ValueArray<vismodule::UInt8>  vismodule_colors ( send_obj->colors().pointer() , nmemb );
        vismodule::ValueArray<vismodule::Real32> vismodule_normals( send_obj->normals().pointer(), nmemb );

        recv_obj->clear();
        recv_obj->setCoords( vismodule_coords );
        recv_obj->setColors( vismodule_colors );
        recv_obj->setNormals( vismodule_normals );
#endif

        kvs::ValueArray<kvs::Real32> kvs_coords;
        kvs::ValueArray<kvs::UInt8>  kvs_colors;
        kvs::ValueArray<kvs::Real32> kvs_normals;

        if ( point_object->coords().size() > 0 )
        {
            const size_t ncoords  = point_object->coords().size() + recv_obj->coords().size();
            const size_t ncolors  = point_object->colors().size() + recv_obj->colors().size();
            const size_t nnormals = point_object->normals().size() + recv_obj->normals().size();

            kvs_coords.allocate( ncoords );
            kvs_coords.allocate( ncolors );
            kvs_coords.allocate( nnormals );

            kvs::Real32* pcoords  = kvs_coords.data();
            kvs::UInt8* pcolors  = kvs_colors.data();
            kvs::Real32* pnormals = kvs_normals.data();

            memcpy( pcoords, point_object->coords().data(), point_object->coords().byteSize() );
            memcpy( pcoords + point_object->coords().size(), recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
            memcpy( pcolors, point_object->colors().data(), point_object->colors().byteSize() );
            memcpy( pcolors + point_object->colors().size(), recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
            memcpy( pnormals, point_object->normals().data(), point_object->normals().byteSize() );
            memcpy( pnormals + point_object->normals().size(), recv_obj->normals().pointer(), recv_obj->normals().byteSize() );
        }
        else
        {
            const size_t ncoords = recv_obj->coords().size();
            const size_t ncolors  = recv_obj->colors().size();
            const size_t nnormals = recv_obj->normals().size();

            kvs_coords.allocate( ncoords );
            kvs_coords.allocate( ncolors );
            kvs_coords.allocate( nnormals );

            kvs::Real32* pcoords  = kvs_coords.data();
            kvs::UInt8* pcolors  = kvs_colors.data();
            kvs::Real32* pnormals = kvs_normals.data();

            memcpy( pcoords, recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
            memcpy( pcolors, recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
            memcpy( pnormals, recv_obj->normals().pointer(), recv_obj->normals().byteSize() );
        }

        delete send_obj;
        delete recv_obj;
    } // end of while(DispatchNext)

#ifndef CPU_VER
    /* // 一時保留
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, ( DEFAULT_NBINS * tf_number ), MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, ( DEFAULT_NBINS * tf_number ), MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
    */
#endif

    // vr = setVariablerange2( tmp_max, tmp_min, ( tf_number * 2 ) ); // 一時保留

    nan_error = false;
    /*  一時保留
    delete tmp_min;
    delete tmp_max;
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    */
}

void GenerateParticleIS(
    ParticleProperty &param,
    MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string file_name,
    std::unique_ptr<kvs::PointObject> point_object
    // jpv::ParticleTransferServer pts,
    // jpv::ServerMode server_mode,
    // jpv::InitializeParameter init_param
)
{
    int tf_number;
    ParticleMonitor pm;
    VariableRange vr;

    std::cout << "param.m_time_step:" << param.m_time_step << std::endl;

    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_particle( pm.particleStatusFile().getLatestTimeStep() );
    }
    else
    {
        pm.setTimeStep_particle(0);
    }
    pm.readParticleHistoryFile();

    tf_number = pm.particleHistoryFile().colorHistogramArray().size();

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];

    for ( size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
    }

    vismodule::PointObject* originalObject = new vismodule::PointObject;

    // get point object
    pm.readParticleFile();
    pm.getParticle( originalObject );

    // get histgram start
    int c_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            tmp_c_bins[c_count] = pm.particleHistoryFile().colorHistogramArray()[tf][res];
            c_count++;
        }
    }

    int o_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            tmp_o_bins[o_count] = pm.particleHistoryFile().opacityHistogramArray()[tf][res];
            o_count++;
        }
    }
    // get histgram end

    vr = pm.particleHistoryFile().variableRange();
    vr.show();

    delete originalObject;
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

void generate_volume(
    const ParticleProperty& param,
    const MultiVolumeProperty& mvp,
    const int time_step,
    vismodule::VolumeObjectBase*& volume
)
{
    struct stat s;
    if ( stat( param.filepath.c_str(), &s ) )
    {
        std::cout << "Error. read failed:" << param.filepath << std::endl;
        exit( 1 );
    }

    if ( vismoduleview::FileChecker::ImportableStructuredVolume( param.filepath ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new vismodule::StructuredVolumeImporter( param.filepath ); 
        int id = 1;
        volume->updateMinMaxValues();
        volume->setMinMaxObjectCoords( mvp.m_min_subvolume_coord[id], mvp.m_max_subvolume_coord[id] );
        volume->setMinMaxExternalCoords( mvp.m_min_subvolume_coord[id], mvp.m_max_subvolume_coord[id] );

    } 
    else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( param.filepath))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new vismodule::UnstructuredVolumeImporter( param.filepath );  
        volume->updateMinMaxValues();
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
    }
    else 
    {
        visModuleMessageError("%s is not volume data.", param.filepath.c_str());
    }

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    return;
}

void generate_volume(
    const ParticleProperty &param,
    const MultiVolumeProperty& mvp,
    const int time_step,
    const int sub_volume_id,
    vismodule::VolumeObjectBase*& volume
)
{
    size_t found_kvsml = param.filepath.find(".kvsml");
    size_t found_vtm   = param.filepath.find(".vtm");
    size_t found_vtu   = param.filepath.find(".vtu");
    size_t found_vti   = param.filepath.find(".vti");
    size_t found_inp   = param.filepath.find(".inp");
    size_t found_pvtu  = param.filepath.find(".pvtu");
    size_t found_case  = param.filepath.find(".case");

    if ( found_kvsml != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( param.filepath );
    
        vismodule::File ifpx( mvp.m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
    
        volume = new vismodule::UnstructuredVolumeImporter( path_base, mvp.m_file_type, time_step, sub_volume_id );
    }
#ifdef EXTEND_FILE_FORMAT
    else if ( found_vtm != std::string::npos )
    {
        // structured
        if( mvp.m_file_type == 3 )
        {
            volume = new vismodule::StructuredVolumeImporter( mvp.m_file_path, time_step, sub_volume_id );
        }
        // unstructured
        if( mvp.m_file_type == 4 )
        {
            volume = new vismodule::UnstructuredVolumeImporter( mvp.m_file_path, mvp.m_file_type, mvp.m_elem_type, time_step, sub_volume_id );
        }
    }
    else if ( found_vtu  != std::string::npos ||
              found_inp  != std::string::npos ||
              found_pvtu != std::string::npos ||
              found_case != std::string::npos
            )
    {
        volume = new vismodule::UnstructuredVolumeImporter( mvp.m_file_path, mvp.m_file_type, mvp.m_elem_type, time_step, sub_volume_id );
    }
    else if ( found_vti != std::string::npos )
    {
        volume = new vismodule::StructuredVolumeImporter( mvp.m_file_path, time_step, sub_volume_id );
    }
#endif

    // CS only
    if ( volume )
    {
        std::string xss = param.m_x_synthesis;
        std::string yss = param.m_y_synthesis;
        std::string zss = param.m_z_synthesis;
        vismodule::CoordSynthesizerStrings css( 0, xss, yss, zss );
        volume->setCoordSynthesizerStrings( css );
    }

    // .vtm .pvtu .case file format
    if ( ( found_vtm != std::string::npos ) || ( found_pvtu != std::string::npos ) || ( found_case != std::string::npos ) )
    {
        // Structured Volume Data
        if ( mvp.m_file_type == 3 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( mvp.m_min_subvolume_coord[sub_volume_id], mvp.m_max_subvolume_coord[sub_volume_id] );
            volume->setMinMaxExternalCoords( mvp.m_min_subvolume_coord[sub_volume_id], mvp.m_max_subvolume_coord[sub_volume_id] );
        }

        // Unstructured Volume Data
        else if ( mvp.m_file_type == 4 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
            volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        }
    }
    else
    {
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
    }

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;    
}

void store_volume_in_variables_array_common(
    const vismodule::VolumeObjectBase* volume,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
)
{
    //詰め替え処理
    vismodule::AnyValueArray valueArray;
    valueArray = volume->values(); 
    ncoords    = volume->nnodes();
    nvariables = volume->veclen();

    // ここで変数の値をfloatでまとめることで粒子生成のテンプレート化を回避
    // std::unique_ptr<std::unique_ptr<Type[]>[]> tmp_values(new std::unique_ptr<Type[]>[nvariables]);
    values = std::make_unique<std::unique_ptr<Type[]>[]>( nvariables );

    // 実行時型分岐で呼び出す
    const std::type_info& type = volume->values().typeInfo()->type();
    if (type == typeid( vismodule::Int8 ) )
    {
        copy_values<vismodule::Int8>( valueArray, values, nvariables, ncoords );
    }  
    else if ( type == typeid( vismodule::Int16 ) )
    {
        copy_values<vismodule::Int16>( valueArray, values, nvariables, ncoords );
    } 
    else if ( type == typeid( vismodule::Int32 ) )
    {
        copy_values<vismodule::Int32>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::Int64 ) )
    {
        copy_values<vismodule::Int64>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt8 ) )
    {
        copy_values<vismodule::UInt8>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        copy_values<vismodule::UInt16>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        copy_values<vismodule::UInt32>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
        copy_values<vismodule::UInt64>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        copy_values<vismodule::Real32>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        copy_values<vismodule::Real64>( valueArray, values, nvariables, ncoords );
    }
    else 
    {
        throw std::runtime_error( "Unsupported type" );
    }
}

void store_volume_in_variables_array_struct(
    const vismodule::VolumeObjectBase* volume,
    domain_parameters_struct& dom,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
)
{
    store_volume_in_variables_array_common( volume, values, nvariables, ncoords );

    const vismodule::StructuredVolumeObject* svo_p = static_cast<const vismodule::StructuredVolumeObject*>( volume );

    int resol[3] = {
        static_cast<int>(svo_p->resolution().x()),
        static_cast<int>(svo_p->resolution().y()),
        static_cast<int>(svo_p->resolution().z())
    };

    dom = {
        volume->minObjectCoord().x(),
        volume->minObjectCoord().y(),
        volume->minObjectCoord().z(),
        volume->maxObjectCoord().x(),
        volume->maxObjectCoord().y(),
        volume->maxObjectCoord().z(),
        volume->minObjectCoord().x(),
        volume->minObjectCoord().y(),
        volume->minObjectCoord().z(),
        volume->maxObjectCoord().x(),
        volume->maxObjectCoord().y(),
        volume->maxObjectCoord().z(),
        resol,
        1.f
    };
}

void store_volume_in_variables_array_unstruct(
    const vismodule::VolumeObjectBase* volume,
    domain_parameters_unstruct& dom,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    std::unique_ptr<float[]>& coordinates,
    int& ncoords,
    std::unique_ptr<unsigned int[]>& connections,
    int& ncells,
    vismodule::VolumeObjectBase::CellType& celltype
)
{
    store_volume_in_variables_array_common( volume, values, nvariables, ncoords );

    const vismodule::UnstructuredVolumeObject* uvo_p = static_cast<const vismodule::UnstructuredVolumeObject*>( volume );

    std::vector<float> tmp_coordinates;
    tmp_coordinates.assign( (float*)uvo_p->coords().begin(), (float*)uvo_p->coords().end() );
    coordinates = std::make_unique<float[]>( tmp_coordinates.size() );
    std::copy( tmp_coordinates.begin(), tmp_coordinates.end(), coordinates.get() );

    std::vector<unsigned int> tmp_connections;
    tmp_connections.assign( (unsigned int*)uvo_p->connections().begin(), (unsigned int*)uvo_p->connections().end() );
    connections = std::make_unique<unsigned int[]>( tmp_connections.size() );
    std::copy( tmp_connections.begin(), tmp_connections.end(), connections.get() );

    ncells   = uvo_p->ncells();
    celltype = uvo_p->cellType();

    dom = {
        volume->minObjectCoord().x(),
        volume->minObjectCoord().y(),
        volume->minObjectCoord().z(),
        volume->maxObjectCoord().x(),
        volume->maxObjectCoord().y(),
        volume->maxObjectCoord().z()
    };
}

template <typename T>
void copy_values(
    vismodule::AnyValueArray& valueArray,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int nvariables,
    int nnodes
)
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

void MakeHistgram(
    const vismodule::PointObject* point_object,
    const int tf_number,
    vismodule::UInt64* c_bins,
    vismodule::UInt64* o_bins
)
{
    int c_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        int c_nbins = point_object->getNbins();
        for ( int res = 0; res < c_nbins; res++ )
        {
            c_bins[ c_count ] += point_object->getCHistogram()[ c_count ];
            c_count++;
        }
    }

    int o_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        int o_nbins = point_object->getNbins();
        for ( int res = 0; res < o_nbins; res++ )
        {
            o_bins[ o_count ] += point_object->getOHistogram()[ o_count ];
            o_count++;
        }
    }
}

void MakeParticleMinMax(
    const TransferFunctionSynthesizer* transfer_function_synthesizer,
    const int tf_number,
    float* max_array,
    float* min_array
)
{
    for( int i = 0; i < tf_number; i++ )
    {
        max_array[2 * i + 1] = vismodule::Math::Max( max_array[2 * i + 1], transfer_function_synthesizer->m_c_max[i] );
        min_array[2 * i + 1] = vismodule::Math::Min( min_array[2 * i + 1], transfer_function_synthesizer->m_c_min[i] );
        max_array[2 * i    ] = vismodule::Math::Max( max_array[2 * i    ], transfer_function_synthesizer->m_o_max[i] );
        min_array[2 * i    ] = vismodule::Math::Min( min_array[2 * i    ], transfer_function_synthesizer->m_o_min[i] );
    }
}
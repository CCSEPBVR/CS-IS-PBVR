#include <vismodule/JobDispatcher>
#include <vismodule/GenerateParticle>
#include <vismodule/Calculate>
#include <vismodule/VariableRange>
#include <vismodule/PointObjectGenerator>
#include <vismodule/FileChecker>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>

#ifndef CPU_VER
    #include <vismodule/JobCollector>
#endif

void GenerateParticleCS(
    std::string& file_path,
    const int time_step, 
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    std::unique_ptr<kvs::PointObject>& point_object
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
    char tmp_sampling_method;

#ifndef CPU_VER
    JobCollector jc( &jd );
#endif

    tf_number = particle_property.m_transfunc_array.size();

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    std::fill_n( tmp_c_bins, DEFAULT_NBINS * tf_number, 0 );
    std::fill_n( tmp_o_bins, DEFAULT_NBINS * tf_number, 0 );

    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity
    std::fill_n( tmp_max, tf_number * 2, FLT_MIN );
    std::fill_n( tmp_min, tf_number * 2, FLT_MAX );

    // サンプリングメソッドをMinMax用に一時的に変更
    tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method ='x';

    jd.initialize(
        time_step,
        time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        particle_property.m_latency_threshold,
        particle_property.m_job_id_pack_size
    );

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        // calculate min max
        if ( ( rank > 0 ) || ( mpi_size == 1 ) )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( file_path, st, xvl );

            // generate point object start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                vismodule::PointObjectGenerator point_object_generator;
                vismodule::PointObject* tmp_obj = nullptr;

                // generate volume object
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else // filetype: kvsml
                {
                    generate_volume( file_path, mvp, volume );
                }

                if ( !volume )
                {
                    throw std::runtime_error("Failed to generate volume object.");
                }

                float max_opacity;
                float max_density;
                float sampling_volume_inverse;

                if ( particle_property.m_camera == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_camera is NULL." << std::endl;
                }

                vismodule::CellByCellParticleGenerator::CalculateDensityConstaint(
                    *particle_property.m_camera,
                    *volume,
                    static_cast<float>( particle_property.m_subpixel_level ),
                    particle_property.m_sampling_step,
                    &sampling_volume_inverse,
                    &max_opacity,
                    &max_density
                );

                if ( particle_property.m_transfunc_synthesizer == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_transfunc_synthesizer is NULL." << std::endl;
                }

                particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
                particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
                particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

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

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }

                    // generate particle
                    tmp_obj = point_object_generator.GenerateParticleUnstruct( particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(), ncoords, connections.get(), ncells, celltype, ServerMode::CS );
                }
                else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                {
                    domain_parameters_struct dom; 
                    std::array<int, 3> resolution;

                    store_volume_in_variables_array_struct( volume, dom, resolution, values, nvariables, ncoords );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }                        

                    // generate particle
                    tmp_obj = point_object_generator.GenerateParticleStruct( particle_property, dom, raw_pointers_vector.data(), nvariables, ServerMode::CS );
                }

                delete volume;
                delete tmp_obj;
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

            MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );
        } // make point object and histgram and range
    } // end of while(DispatchNext)

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    // min,maxの更新
    MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );

    vr = setVariablerange2( tmp_max, tmp_min, tf_number );
    vr.show();

    for( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream ss; 
        ss << (i + 1); 
        const std::string idxbuf = ss.str();

        float color_variable_min   = vr.min( "t" + idxbuf + "_var_c" );
        float color_variable_max   = vr.max( "t" + idxbuf + "_var_c" );
        float opacity_variable_min = vr.min( "t" + idxbuf + "_var_o" );
        float opacity_variable_max = vr.max( "t" + idxbuf + "_var_o" );

        // MinMaxの格納
        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        // ColorMap,OpacityMapの更新
        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max);
        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
    }

    // 本命の粒子生成のためjob dispatch のパラメータ初期化 
    wid = 0; st = 0; vl = 0; 

    // 一時的に変更していたサンプリングメソッドを元に戻す
    particle_property.m_sampling_method = tmp_sampling_method;

    jd.initialize(
        time_step,
        time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        particle_property.m_latency_threshold,
        particle_property.m_job_id_pack_size
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
            mvp.setFilePath( file_path, st, xvl );

            // generate point object start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                vismodule::PointObjectGenerator point_object_generator;

                // generate volume object
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
                else // filetype: kvsml
                {
                    generate_volume( file_path, mvp, volume );
                }

                if ( !volume )
                {
                    throw std::runtime_error("Failed to generate volume object.");
                }

                float max_opacity;
                float max_density;
                float sampling_volume_inverse;

                if ( particle_property.m_camera == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_camera is NULL." << std::endl;
                }
                    
                vismodule::CellByCellParticleGenerator::CalculateDensityConstaint(
                    *particle_property.m_camera,
                    *volume,
                    static_cast<float>( particle_property.m_subpixel_level ),
                    particle_property.m_sampling_step,
                    &sampling_volume_inverse,
                    &max_opacity,
                    &max_density
                );

                if ( particle_property.m_transfunc_synthesizer == nullptr )
                {
                    std::cout << "ERROR: particle_property.m_transfunc_synthesizer is NULL." << std::endl;
                }

                particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
                particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
                particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

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

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }

                    // generate particle
                    send_obj = point_object_generator.GenerateParticleUnstruct( particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(), ncoords, connections.get(), ncells, celltype, ServerMode::CS );
                }
                else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                {
                    domain_parameters_struct dom; 
                    std::array<int, 3> resolution;

                    store_volume_in_variables_array_struct( volume, dom, resolution, values, nvariables, ncoords );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }                        

                    // generate particle
                    send_obj = point_object_generator.GenerateParticleStruct( particle_property, dom, raw_pointers_vector.data(), nvariables, ServerMode::CS );
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
            
            MakeHistgram( send_obj, tf_number, tmp_c_bins, tmp_o_bins );
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
            kvs_colors.allocate( ncolors );
            kvs_normals.allocate( nnormals );

            kvs::Real32* pcoords  = kvs_coords.data();
            kvs::UInt8* pcolors   = kvs_colors.data();
            kvs::Real32* pnormals = kvs_normals.data();

            memcpy( pcoords, point_object->coords().data(), point_object->coords().byteSize() );
            memcpy( pcoords + point_object->coords().size(), recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
            memcpy( pcolors, point_object->colors().data(), point_object->colors().byteSize() );
            memcpy( pcolors + point_object->colors().size(), recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
            memcpy( pnormals, point_object->normals().data(), point_object->normals().byteSize() );
            memcpy( pnormals + point_object->normals().size(), recv_obj->normals().pointer(), recv_obj->normals().byteSize() );

            point_object->setCoords( kvs_coords );
            point_object->setColors( kvs_colors );
            point_object->setNormals( kvs_normals );
        }
        else
        {
            const size_t ncoords = recv_obj->coords().size();
            const size_t ncolors  = recv_obj->colors().size();
            const size_t nnormals = recv_obj->normals().size();

            kvs_coords.allocate( ncoords );
            kvs_colors.allocate( ncolors );
            kvs_normals.allocate( nnormals );

            kvs::Real32* pcoords  = kvs_coords.data();
            kvs::UInt8*  pcolors  = kvs_colors.data();
            kvs::Real32* pnormals = kvs_normals.data();

            memcpy( pcoords, recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
            memcpy( pcolors, recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
            memcpy( pnormals, recv_obj->normals().pointer(), recv_obj->normals().byteSize() );

            point_object->setCoords( kvs_coords );
            point_object->setColors( kvs_colors );
            point_object->setNormals( kvs_normals );
        }

        delete send_obj;
        delete recv_obj;
    } // end of while(DispatchNext)

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, ( DEFAULT_NBINS * tf_number ), MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, ( DEFAULT_NBINS * tf_number ), MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    }
#endif

    // histogramの格納
    for( int i = 0; i < tf_number; i++ )
    {
        // histogram
        std::copy( tmp_c_bins + ( DEFAULT_NBINS * i ), tmp_c_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_color_histogram );
        std::copy( tmp_o_bins + ( DEFAULT_NBINS * i ), tmp_o_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_opacity_histogram );
    }

    nan_error = false;

    delete[] tmp_min;
    delete[] tmp_max;
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

void GenerateParticleIS(
    const int time_step,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    std::unique_ptr<kvs::PointObject>& point_object
)
{
    int tf_number;
    ParticleMonitor pm;
    VariableRange vr;

    std::cout << "time_step:" << time_step << std::endl;

    pm.check();

    if( pm.stepExisted() )
    {
        // pm.setTimeStep_particle( pm.particleStatusFile().getLatestTimeStep() );
        pm.setTimeStep_particle( time_step );
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
    std::fill_n( tmp_c_bins, DEFAULT_NBINS * tf_number, 0 );
    std::fill_n( tmp_o_bins, DEFAULT_NBINS * tf_number, 0 );

    vismodule::PointObject* vismodule_point_object = new vismodule::PointObject;

    // get point object
    pm.readParticleFile();
    pm.getParticle( vismodule_point_object );

    kvs::ValueArray<kvs::Real32> kvs_coords;
    kvs::ValueArray<kvs::UInt8>  kvs_colors;
    kvs::ValueArray<kvs::Real32> kvs_normals;

    const size_t ncoords  = vismodule_point_object->coords().size();
    const size_t ncolors  = vismodule_point_object->colors().size();
    const size_t nnormals = vismodule_point_object->normals().size();

    kvs_coords.allocate( ncoords );
    kvs_colors.allocate( ncolors );
    kvs_normals.allocate( nnormals );

    kvs::Real32* pcoords  = kvs_coords.data();
    kvs::UInt8*  pcolors  = kvs_colors.data();
    kvs::Real32* pnormals = kvs_normals.data();

    memcpy( pcoords, vismodule_point_object->coords().pointer(), vismodule_point_object->coords().byteSize() );
    memcpy( pcolors, vismodule_point_object->colors().pointer(), vismodule_point_object->colors().byteSize() );
    memcpy( pnormals, vismodule_point_object->normals().pointer(), vismodule_point_object->normals().byteSize() );

    point_object->setCoords( kvs_coords );
    point_object->setColors( kvs_colors );
    point_object->setNormals( kvs_normals );

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

    // histogram, minmaxの格納
    for( int i = 0; i < tf_number; i++ )
    {
        // histogram
        std::copy( tmp_c_bins + ( DEFAULT_NBINS * i ), tmp_c_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_color_histogram );
        std::copy( tmp_o_bins + ( DEFAULT_NBINS * i ), tmp_o_bins + ( DEFAULT_NBINS * ( i + 1 ) ), particle_property.m_transfunc_array[i].m_opacity_histogram );

        // minmax
        std::stringstream ss; 
        ss << (i + 1); 
        const std::string idxbuf = ss.str();
        particle_property.m_transfunc_array[i].m_server_color_variable_min   = vr.min( "t" + idxbuf + "_var_c" );
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = vr.max( "t" + idxbuf + "_var_c" );
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = vr.min( "t" + idxbuf + "_var_o" );
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = vr.max( "t" + idxbuf + "_var_o" );
    }

    delete vismodule_point_object;
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

void generate_volume(
    const std::string& file_path,
    const MultiVolumeProperty& mvp,
    vismodule::VolumeObjectBase*& volume
)
{
    struct stat s;
    if ( stat( file_path.c_str(), &s ) )
    {
        std::cout << "Error. read failed:" << file_path << std::endl;
        exit( 1 );
    }

    if ( vismoduleview::FileChecker::ImportableStructuredVolume( file_path ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new vismodule::StructuredVolumeImporter( file_path ); 
        int id = 1;

    } 
    else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( file_path))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new vismodule::UnstructuredVolumeImporter( file_path );  
        volume->updateMinMaxValues();
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
    }
    else 
    {
        visModuleMessageError("%s is not volume data.", file_path.c_str());
    }

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    return;
}

void generate_volume(
    const std::string& file_path,
    const MultiVolumeProperty& mvp,
    const int time_step,
    const int sub_volume_id,
    vismodule::VolumeObjectBase*& volume
)
{
    size_t found_kvsml = file_path.find(".kvsml");
    size_t found_vtm   = file_path.find(".vtm");
    size_t found_vtu   = file_path.find(".vtu");
    size_t found_vti   = file_path.find(".vti");
    size_t found_inp   = file_path.find(".inp");
    size_t found_pvtu  = file_path.find(".pvtu");
    size_t found_case  = file_path.find(".case");

    if ( found_kvsml != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( file_path );
    
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
    std::array<int, 3>& resolution,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
)
{
    store_volume_in_variables_array_common( volume, values, nvariables, ncoords );

    const vismodule::StructuredVolumeObject* svo_p = static_cast<const vismodule::StructuredVolumeObject*>( volume );

    resolution[0] = static_cast<int>( svo_p->resolution().x() );
    resolution[1] = static_cast<int>( svo_p->resolution().y() );
    resolution[2] = static_cast<int>( svo_p->resolution().z() );

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
        resolution.data(),
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

    coordinates = std::make_unique<float[]>( uvo_p->coords().size() );
    std::memcpy( coordinates.get(), uvo_p->coords().begin(), uvo_p->coords().size() * sizeof( float ) );

    connections = std::make_unique<unsigned int[]>( uvo_p->connections().size() );
    std::memcpy( connections.get(), uvo_p->connections().begin(), uvo_p->connections().size() * sizeof( unsigned int ) );

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

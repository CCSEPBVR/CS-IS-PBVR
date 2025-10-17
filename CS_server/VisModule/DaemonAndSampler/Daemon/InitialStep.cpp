#include <vismodule/InitialStep>

void initial_step(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    int32_t server_mode
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
    jpv::ParticleTransferServerMessage servMes;
    std::vector<vismodule::CS_PointObjectGenerator> point_generator_lst;
    const int tf_number = mvpl.m_list[0].m_number_ingredients;

    jd.initialize(
        mvpl.m_total_start_steps,
        mvpl.m_total_start_steps,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        param.m_latency_threshold,
        param.m_job_id_pack_size
    );

    if( rank == 0 )
    {
        // make sub volume num server message start
        strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
        servMes.m_camera = param.m_camera;
        servMes.m_server_status = 0;
        servMes.m_time_step = param.m_time_step;
        servMes.m_level_index = param.m_level_index;
        servMes.m_repeat_level = param.m_repeat_level;
        servMes.m_number_particle = 0;
        servMes.m_number_glyph = 0;
        servMes.m_flag_send_bins = 1;
        servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
        servMes.m_transfer_function_count = 0;
        servMes.m_start_step = mvpl.m_total_start_steps;
        servMes.m_last_step = mvpl.m_total_last_step;
        servMes.m_number_step = mvpl.m_total_number_steps;
        servMes.m_min_object_coord[0] = mvpl.m_total_min_object_coord[0];
        servMes.m_min_object_coord[1] = mvpl.m_total_min_object_coord[1];
        servMes.m_min_object_coord[2] = mvpl.m_total_min_object_coord[2];
        servMes.m_max_object_coord[0] = mvpl.m_total_max_object_coord[0];
        servMes.m_max_object_coord[1] = mvpl.m_total_max_object_coord[1];
        servMes.m_max_object_coord[2] = mvpl.m_total_max_object_coord[2];
        servMes.m_min_value = mvpl.m_total_min_value;
        servMes.m_max_value = mvpl.m_total_max_value;
        servMes.m_number_nodes = mvpl.m_total_number_nodes;
        servMes.m_number_elements = mvpl.m_total_number_elements;
        servMes.m_element_type = mvpl.m_list[0].m_elem_type;
        servMes.m_file_type = mvpl.m_list[0].m_file_type;
        servMes.m_number_ingredients = mvpl.m_list[0].m_number_ingredients;
        servMes.m_opacity_transfer_function_synthesis = "O1";
        servMes.m_color_transfer_function_synthesis = "C1";
        servMes.m_particle_limit = param.m_particle_limit;
        servMes.m_particle_density = param.m_particle_density;
        servMes.m_subpixel_level = param.m_subpixel_level;
        servMes.m_server_side_variable_range = param.m_server_side_variable_range;
        // make sub volume num server message end

        // send sub volume num server message
        std::cout << "INFO: send sub volume num server message" << std::endl;
        servMes.m_message_size = servMes.byteSize();
        servMes.show();
        pts.sendMessage( servMes );            
    }

    if ( server_mode == jpv::ServerMode::CS )
    {
        point_generator_lst.clear();
        point_generator_lst.resize(mvpl.m_list.size());
    }

    int c_bins_size = 0;
    int o_bins_size = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        c_bins_size += DEFAULT_NBINS;
        o_bins_size += DEFAULT_NBINS;
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];

    for ( int tf = 0; tf < c_bins_size; tf++ )
    {
        tmp_c_bins[tf] = 0;
    }

    for ( int tf = 0; tf < o_bins_size; tf++ )
    {
        tmp_o_bins[tf] = 0;
    }

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        if ( server_mode == jpv::ServerMode::CS )
        {
            vismodule::PointObject* tmp_obj = NULL;
            vismodule::PointObject* originalObject = new vismodule::PointObject;

            // make point object and histgram
            if ( ( rank > 0 ) || ( mpi_size == 1 ) )
            {
                int xvl, fidx;
                fidx = mvpl.getFileIndex( vl, &xvl );
                MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                mvp.setFilePath( param.m_input_data, st, xvl );
                point_generator_lst[fidx].setFilterInfo( &mvp );
                param.m_subvolume_id = xvl;

                point_generator_lst[fidx].setCoordSynthStr(
                    param.m_x_synthesis,
                    param.m_y_synthesis,
                    param.m_z_synthesis
                );

                // generate point object start            
                try
                {
                    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                        tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, st, xvl);
                    }
                    else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                    {
                        tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, st, xvl);
                    }
                    else     // filetype: kvsml
                    {
                        tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, st );
                    }
                }
                catch ( const std::runtime_error& e )
                {
#ifdef _DEBUG
                    // debug by @hira
                    printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                    std::cerr << e.what();
                    nan_error = true;
                }

                // make histgram start
                int c_count = 0;
                for ( int tf = 0; tf < tf_number; tf++ )
                {
                    int c_nbins = tmp_obj->getNbins();
                    for ( int res = 0; res < c_nbins; res++ )
                    {
                        tmp_c_bins[ c_count ] += tmp_obj->getCHistogram()[ c_count ];
                        c_count++;
                    }
                }
                int o_count = 0;
                for ( int tf = 0; tf < tf_number; tf++ )
                {
                    int o_nbins = tmp_obj->getNbins();
                    for ( int res = 0; res < o_nbins; res++ )
                    {
                        tmp_o_bins[ o_count ] += tmp_obj->getOHistogram()[ o_count ];
                        o_count++;
                    }
                }
                // make histgram end
            } // make point object and histgram

#ifndef CPU_VER
            if ( mpi_size > 1 ) {
                if ( rank == 0 )
                {
                    jc.jobCollect( originalObject, &vr, &nan_error, &wid );
                }
                else
                {
                    jc.jobCollect( tmp_obj, &vr, &nan_error, &wid );
                }
            }
#endif

            delete tmp_obj;
            delete originalObject;
        } // server_mode == jpv::ServerMode::CS
        else // server_mode == jpv::ServerMode::IS
        {
            ParticleMonitor pm;
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
        } // server_mode == jpv::ServerMode::IS
    } // end of while(DispatchNext)

#ifndef CPU_VER
    if (mpi_size > 1) {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    }
#endif

    if ( rank == 0 ) // send histgram
    {
        // make histgram server message start
        servMes.m_number_particle = 0;
        servMes.m_flag_send_bins = 1;
        servMes.m_transfer_function_count = tf_number;
        servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
        servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];
        servMes.m_color_bins.resize( tf_number );
        servMes.m_opacity_bins.resize( tf_number );

        for ( int tf = 0; tf < tf_number; tf++ )
        {
            servMes.m_color_nbins[tf] = DEFAULT_NBINS;
            servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
            servMes.m_color_bins[tf]   =  new vismodule::UInt64[ DEFAULT_NBINS ];
            servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ DEFAULT_NBINS ];
            for ( int res = 0; res < DEFAULT_NBINS; res++ )
            {
                servMes.m_color_bins[tf][res] = 0;
            }
            for ( int res = 0; res < DEFAULT_NBINS; res++ )
            {
                servMes.m_opacity_bins[tf][res] = 0;
            }
        }

        servMes.setColorHistogramBins(                                                     
                param.m_transfunc_array.size(),
                DEFAULT_NBINS,
                tmp_c_bins
        );
        servMes.setOpacityHistogramBins(
                param.m_transfunc_array.size(),
                DEFAULT_NBINS,
                tmp_o_bins
        );
        // make histgram server message end

        // TEST START 2015.1.14
        if ( nan_error )
        {
        // TEST START 2015.1.14
            strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
            servMes.m_server_status = 1;
            servMes.m_number_particle = 0;
            servMes.m_number_glyph = 0 ;
            servMes.m_flag_send_bins = 1;
            std::cout << "!!!!!!!!!!!! Send serverStatus = 1 " << std::endl;
        }

        // send histgram server message
        std::cout << "INFO: send histgram server message" << std::endl;
        servMes.m_message_size = servMes.byteSize();
        servMes.show();
        pts.sendMessage( servMes );

        // TEST START 2015.1.14
        servMes.m_server_status = 0;
        // TEST END 2015.1.14

        for ( int tf = 0; tf < tf_number; tf++ )
        {
            delete[] servMes.m_color_bins[tf];
            delete[] servMes.m_opacity_bins[tf];
        }

        delete[] servMes.m_color_nbins;
        delete[] servMes.m_opacity_nbins;
    } // send histgram

    nan_error = false;

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

void initial_step_worker(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    TransferFunctionSynthesizerCreator transfunc_creator,
    int& timer_count
)
{
#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    int rank = 0;
	int mpi_size = 1;
#endif
    jpv::ParticleTransferServerMessage servMes;
    int st, vl, wid = 0;
    std::vector<vismodule::CS_PointObjectGenerator> point_generator_lst;
    VariableRange vr;

    point_generator_lst.clear();
    point_generator_lst.resize(mvpl.m_list.size());
    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

    jd.initialize(
        mvpl.m_total_start_steps,
        mvpl.m_total_start_steps,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        param.m_latency_threshold,
        param.m_job_id_pack_size
    );

    param.m_sampling_step = CalculateSamplingStep( mvpl );
    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *param.m_camera );
    // param.m_particle_limit_pre = param.m_particle_limit; 削除予定
    
    int c_bins_size = 0;
    int o_bins_size = 0;
    int c_nbins = DEFAULT_NBINS;
    int o_nbins = DEFAULT_NBINS;
    for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
    {
        c_bins_size += c_nbins;
        o_bins_size += o_nbins;
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];

    float*  tmp_max;
    float*  tmp_min;
    tmp_max = new float[transfunc_creator.transfunc().size() * 2]; 
    tmp_min = new float[transfunc_creator.transfunc().size() * 2]; 

    for ( int tf = 0; tf < (transfunc_creator.transfunc().size() * 2); tf++ )
    {
        tmp_max[tf] = FLT_MIN;
        tmp_min[tf] = FLT_MAX;
    }

    for ( int tf = 0; tf < c_bins_size; tf++ )
    {
        tmp_c_bins[tf] = 0;
    }

    for ( int tf = 0; tf < o_bins_size; tf++ )
    {
        tmp_o_bins[tf] = 0;
    }

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        int xvl, fidx;
        vismodule::PointObject* tmp_obj = NULL;
        fidx = mvpl.getFileIndex( vl, &xvl );
        MultiVolumeProperty& mvp = mvpl.m_list[fidx];
        mvp.setFilePath(param.m_input_data, st, xvl);
        point_generator_lst[fidx].setFilterInfo( &mvp );
        point_generator_lst[fidx].setCoordSynthStr( param.m_x_synthesis, param.m_y_synthesis, param.m_z_synthesis );
        int timeStep = 1;

        try
        {
            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, timeStep, st, xvl );
            }
            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, timeStep, st, xvl);
            }
            else     // filetype: kvsml
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, timeStep, st );
            }
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            std::cerr << e.what();
            nan_error = true;
        }
#ifndef CPU_VER
        jc.jobCollect( tmp_obj, &vr, &nan_error, &wid );
#endif
        if ( nan_error )
        {
            nan_error = false;
            continue;
        }
        
        int c_count = 0;
        for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
        {
            c_nbins = tmp_obj->getNbins();
            //add by shimomura 2023/06/14
            tmp_max[2*tf+1] = vismodule::Math::Max( tmp_max[2*tf+1] ,param.m_transfunc_synthesizer-> m_c_max[tf]);
            tmp_min[2*tf+1] = vismodule::Math::Min( tmp_min[2*tf+1] ,param.m_transfunc_synthesizer-> m_c_min[tf]);
            for ( int res = 0; res < c_nbins; res++ )
            {
                tmp_c_bins[c_count] += tmp_obj->getCHistogram()[ c_count ] ;
                c_count++;
            }
        }

        int o_count = 0;
        for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
        {
            o_nbins = tmp_obj->getNbins();
            //add by shimomura 2023/06/14
            tmp_max[2*tf] = vismodule::Math::Max( tmp_max[2*tf] ,param.m_transfunc_synthesizer-> m_c_max[tf]);
            tmp_min[2*tf] = vismodule::Math::Min( tmp_min[2*tf] ,param.m_transfunc_synthesizer-> m_c_min[tf]);
            for ( int res = 0; res < o_nbins; res++ )
            {
                tmp_o_bins[o_count] += tmp_obj->getOHistogram()[ o_count ] ;
                o_count++;
            }
        }
    } // end of while(DispatchNext)
#ifndef CPU_VER
    MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_max, (transfunc_creator.transfunc().size() * 2), MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_min, (transfunc_creator.transfunc().size() * 2), MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
#endif
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    //add by shimomura 20240603
    delete[] tmp_max;
    delete[] tmp_min;
    delete param.m_transfunc_synthesizer;
}

void initial_step_IS(
    Argument &param,
    MultiVolumePropertyList& mvpl, 
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts
)
{
    jpv::ParticleTransferServerMessage servMes;
    MultiVolumeProperty mvp;
    VariableRange vr;
    const int tf_number = mvpl.m_list[0].m_number_ingredients;

    // Using environment variables, the constructor of the ParticleMonitor class
    // set particle file, glyph file, plot over line file, status file, history file,
    // and the min/max coordinates of the object.
    ParticleMonitor pm;
    pm.check();


    if( pm.stepExisted() )
    {
        pm.setTimeStep_particle( mvpl.m_total_last_step );
    }
    else
    {
        pm.setTimeStep_particle(0);
        std::cout << "WARN:particle status file does not exist" << std::endl;
    }
    pm.readParticleHistoryFile();

    vr = pm.particleHistoryFile().variableRange();

#if 0
    std::cout << "particle monitor variable range" << std::endl;
    std::cout << "tf_number:" << tf_number << std::endl;
    std::cout << "tfname, MIN, MAX" << std::endl;
    for( int i = 0; i < tf_number; i++ )
    {
        std::stringstream tt;
        float c_min, c_max, o_min, o_max;

        tt << "t" << i + 1;
        c_min = vr.min( tt.str() + "_var_c" );
        c_max = vr.max( tt.str() + "_var_c" );
        o_min = vr.min( tt.str() + "_var_o" );
        o_max = vr.max( tt.str() + "_var_o" );

        std::cout << "C" << i << ":" << c_min << ", " << c_max << std::endl;
        std::cout << "C" << i << ":" << o_min << ", " << o_max << std::endl;
    }
#endif

    int c_bins_size = 0;
    int o_bins_size = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        c_bins_size += DEFAULT_NBINS;
        o_bins_size += DEFAULT_NBINS;
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];

    for ( int tf = 0; tf < c_bins_size; tf++ )
    {
        tmp_c_bins[tf] = 0;
    }

    for ( int tf = 0; tf < o_bins_size; tf++ )
    {
        tmp_o_bins[tf] = 0;
    }

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

    // make server message
    strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
    servMes.m_camera = param.m_camera;
    servMes.m_server_status = 0;
    servMes.m_time_step = param.m_time_step;
    servMes.m_level_index = param.m_level_index;
    servMes.m_repeat_level = param.m_repeat_level;
    servMes.m_number_particle = 0;
    servMes.m_number_glyph = 0;
    servMes.m_flag_send_bins = 1;
    servMes.m_number_volume_divide = 1;
    servMes.m_transfer_function_count = 0;
    servMes.m_start_step = mvpl.m_total_start_steps;
    servMes.m_last_step = mvpl.m_total_last_step;
    servMes.m_number_step = mvpl.m_total_number_steps;
    servMes.m_min_object_coord[0] = mvpl.m_total_min_object_coord[0];
    servMes.m_min_object_coord[1] = mvpl.m_total_min_object_coord[1];
    servMes.m_min_object_coord[2] = mvpl.m_total_min_object_coord[2];
    servMes.m_max_object_coord[0] = mvpl.m_total_max_object_coord[0];
    servMes.m_max_object_coord[1] = mvpl.m_total_max_object_coord[1];
    servMes.m_max_object_coord[2] = mvpl.m_total_max_object_coord[2];
    // servMes.m_min_value = mvpl.m_total_min_value;
    // servMes.m_max_value = mvpl.m_total_max_value;
    servMes.m_number_nodes = mvpl.m_total_number_nodes;
    servMes.m_number_elements = mvpl.m_total_number_elements;
    servMes.m_element_type = mvpl.m_list[0].m_elem_type;
    servMes.m_file_type = mvpl.m_list[0].m_file_type;
    servMes.m_number_ingredients = mvpl.m_list[0].m_number_ingredients;
    servMes.m_color_transfer_function_synthesis = param.m_color_transfer_function_synthesis;
    servMes.m_opacity_transfer_function_synthesis = param.m_opacity_transfer_function_synthesis;
    servMes.m_particle_limit = param.m_particle_limit;
    servMes.m_particle_density = param.m_particle_density;
    servMes.m_subpixel_level = param.m_subpixel_level;
    servMes.m_server_side_variable_range = vr;

    servMes.m_transfer_function.clear();
    servMes.m_transfer_function.resize( tf_number );
    for( int i = 0; i < tf_number; i++ )
    {
        servMes.m_transfer_function[i] = param.m_named_transfunc_array[i];
        /*
        servMes.m_transfer_function[i].m_name = param.m_named_transfunc_array[i].m_name;
        servMes.m_transfer_function[i].m_color_variable = param.m_named_transfunc_array[i].m_color_variable;
        servMes.m_transfer_function[i].m_color_variable_min = param.m_named_transfunc_array[i].m_color_variable_min;
        servMes.m_transfer_function[i].m_color_variable_max = param.m_named_transfunc_array[i].m_color_variable_max;
        servMes.m_transfer_function[i].m_opacity_variable = param.m_named_transfunc_array[i].m_opacity_variable;
        servMes.m_transfer_function[i].m_opacity_variable_min = param.m_named_transfunc_array[i].m_opacity_variable_min;
        servMes.m_transfer_function[i].m_opacity_variable_max = param.m_named_transfunc_array[i].m_opacity_variable_max;
        servMes.m_transfer_function[i].m_resolution = param.m_named_transfunc_array[i].m_resolution;
        servMes.m_transfer_function[i].m_equation_red = param.m_named_transfunc_array[i].m_equation_red;
        servMes.m_transfer_function[i].m_equation_green = param.m_named_transfunc_array[i].m_equation_green;
        servMes.m_transfer_function[i].m_equation_blue = param.m_named_transfunc_array[i].m_equation_blue;
        servMes.m_transfer_function[i].m_equation_opacity = param.m_named_transfunc_array[i].m_equation_opacity;
        servMes.m_transfer_function[i].setColorMap( param.m_named_transfunc_array[i].colorMap() );
        servMes.m_transfer_function[i].setOpacityMap( param.m_named_transfunc_array[i].opacityMap() );
        servMes.m_transfer_function[i].m_selection = param.m_named_transfunc_array[i].m_selection;
        */
    }

    servMes.m_transfer_function_count = tf_number; // TF_COUNT
    servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
    servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];
    servMes.m_color_bins.resize( tf_number );
    servMes.m_opacity_bins.resize( tf_number );

    for ( int tf = 0; tf < tf_number; tf++ )
    {
        servMes.m_color_nbins[tf] = DEFAULT_NBINS;
        servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
        servMes.m_color_bins[tf]   =  new vismodule::UInt64[ DEFAULT_NBINS ];
        servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ DEFAULT_NBINS ];
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            servMes.m_color_bins[tf][res] = 0;
        }
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            servMes.m_opacity_bins[tf][res] = 0;
        }
    }

    // add by shimomura 2022/12/16
    servMes.setColorHistogramBins
    (
        param.m_transfunc_array.size(),
        DEFAULT_NBINS,
        tmp_c_bins
    );
    servMes.setOpacityHistogramBins
    (
        param.m_transfunc_array.size(),
        DEFAULT_NBINS,
        tmp_o_bins
    );

    servMes.m_message_size = servMes.byteSize();
    servMes.show();
    pts.sendMessage( servMes );

    servMes.m_server_status = 0;
    servMes.m_transfer_function_count = 0;

    for ( int tf = 0; tf < tf_number; tf++ )
    {
        delete[] servMes.m_color_bins[tf];
        delete[] servMes.m_opacity_bins[tf];
    }
    delete[] servMes.m_color_nbins;
    delete[] servMes.m_opacity_nbins;

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

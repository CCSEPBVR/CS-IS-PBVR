#include <vismodule/InitialStep>

void initial_step_master(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
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

    strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
    servMes.m_camera = param.m_camera;
    servMes.m_server_status = 0;
    servMes.m_time_step = param.m_time_step;
    servMes.m_level_index = param.m_level_index;
    servMes.m_repeat_level = param.m_repeat_level;
    servMes.m_number_particle = 0;
    servMes.m_number_glyph = 0;
    servMes.m_flag_send_bins = 0;
    servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
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
    servMes.m_message_size = servMes.byteSize();

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

    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
    {
        VIS_MODULE_TIMER_STA( 470 );
    }

    param.m_sampling_step = CalculateSamplingStep( mvpl );
    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *servMes.m_camera );

    // 関数の領域確保、初期化を行う : by @hira 2016/12/01
    servMes.initializeTransferFunction(transfunc_creator.transfunc().size(), DEFAULT_NBINS);

    int c_bins_size = 0;
    int o_bins_size = 0;
    for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
    {
        c_bins_size += servMes.m_color_nbins[tf];
        o_bins_size += servMes.m_opacity_nbins[tf];
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];

    //add by shimomura 2023/06/14
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
        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_STA( 471 );
        }

        vismodule::PointObject* originalObject = new vismodule::PointObject;
        
        if (mpi_size == 1)
        {
            int xvl, fidx;
            vismodule::PointObject* tmp_obj = NULL;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath(param.m_input_data, st, xvl);
            point_generator_lst[fidx].setFilterInfo( &mvp );
            point_generator_lst[fidx].setCoordSynthStr( param.m_x_synthesis, param.m_y_synthesis, param.m_z_synthesis );
            int timeStep = 1;
            servMes.m_flag_send_bins = 0;

            try
            {
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    tmp_obj = point_generator_lst[fidx].run( param, *servMes.m_camera, timeStep, st, xvl);
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    tmp_obj = point_generator_lst[fidx].run( param, *servMes.m_camera, timeStep, st, xvl);
                }
                else     // filetype: kvsml
                {
                    tmp_obj = point_generator_lst[fidx].run( param, *servMes.m_camera, timeStep, st );
                }

                // modify by @hira at 2016/12/01  
                int c_count = 0;
                for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
                {
                    int c_nbins = tmp_obj->getNbins();
                    //changed by shimomura 2023/07/24
                    tmp_max[2 * tf + 1] = vismodule::Math::Max( tmp_max[2 * tf + 1], param.m_transfunc_synthesizer->m_c_max[tf] );
                    tmp_min[2 * tf + 1] = vismodule::Math::Min( tmp_min[2 * tf + 1], param.m_transfunc_synthesizer->m_c_min[tf] );
                    for ( int res = 0; res < c_nbins; res++ )
                    {
                        tmp_c_bins[ c_count ] += tmp_obj->getCHistogram()[ c_count ];
                        c_count++;
                    }
                }
                int o_count = 0;
                for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
                {
                    int o_nbins = tmp_obj->getNbins();
                    //changed by shimomura 2023/07/24
                    tmp_max[2 * tf] = vismodule::Math::Max( tmp_max[2 * tf], param.m_transfunc_synthesizer->m_c_max[tf] );
                    tmp_min[2 * tf] = vismodule::Math::Min( tmp_min[2 * tf], param.m_transfunc_synthesizer->m_c_min[tf] );
                    for ( int res = 0; res < o_nbins; res++ )
                    {
                        tmp_o_bins[ o_count ] += tmp_obj->getOHistogram()[ o_count ];
                        o_count++;
                    }
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
        } // if (mpi_size == 1)

#ifndef CPU_VER
        if (mpi_size > 1) {
            jc.jobCollect( originalObject, &vr, &nan_error, &wid );
        }
#endif

        // add by shimomura 2023/06/14
        vismodule::PointObject* object = originalObject;
        if ( originalObject != object ) delete originalObject;
        // 初回通信なので粒子数は0
        // servMes.m_number_particle = object->coords().size() / 3;
        printf(" %zu perticles generated\n", object->coords().size() / 3);

        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_END( 471 );
        }
        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_STA( 472 );
        }

        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_END( 472 );
        }
        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_STA( 473 );
        }
        delete object;
        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_END( 473 );
        }
    } // end of while(DispatchNext)

#ifndef CPU_VER
    if (mpi_size > 1) {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, (transfunc_creator.transfunc().size() * 2), MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, (transfunc_creator.transfunc().size() * 2), MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
    }
#endif
    //add by shimomura 2023/06/14
    vr = setVariablerange2( tmp_max, tmp_min, servMes.m_transfer_function_count );
    servMes.m_server_side_variable_range = vr;
    transfunc_creator.setTransferFunction(&servMes, vr);

    std::stringstream tt;
    tt << "t1";
    servMes.m_glyph_color_min  = vr.min( tt.str() + "_var_c" );
    servMes.m_glyph_color_max  = vr.max( tt.str() + "_var_c" ); 
    servMes.m_glyph_size_min   = vr.min( tt.str() + "_var_c" );
    servMes.m_glyph_size_max   = vr.max( tt.str() + "_var_c" );

    // add by shimomura 2022/12/16
    servMes.setColorHistogramBins(                                                     
            param.m_transfunc_array.size(),
            DEFAULT_NBINS,
            tmp_c_bins);//,
    servMes.setOpacityHistogramBins(
            param.m_transfunc_array.size(),
            DEFAULT_NBINS,
            tmp_o_bins); // change by shimomura 2022/12/26

    // TEST START 2015.1.14
    if ( nan_error )
    {
    // TEST START 2015.1.14
        strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
        servMes.m_server_status = 1;
        servMes.m_number_particle = 0;
        servMes.m_number_glyph = 0 ;
        servMes.m_flag_send_bins = 1; // histogram
        std::cout << "!!!!!!!!!!!! Send serverStatus = 1 " << std::endl;
        nan_error = false;
    }



    servMes.m_flag_send_bins = 1; // histogram
    servMes.m_subpixel_level = param.m_subpixel_level;
    servMes.m_message_size = servMes.byteSize();
    // TEST START 2015.1.14
    pts.sendMessage( servMes );
    // TEST START 2015.1.14
    servMes.m_server_status = 0;
    // TEST END 2015.1.14

    for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
    {
        delete[] servMes.m_color_bins[tf];
        delete[] servMes.m_opacity_bins[tf];
    }
    delete[] servMes.m_color_nbins;
    delete[] servMes.m_opacity_nbins;
    servMes.m_transfer_function_count = 0;
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    //add by shimomura 20240603
    delete[] tmp_max;
    delete[] tmp_min;
    delete param.m_transfunc_synthesizer;

    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
    {
        VIS_MODULE_TIMER_END( 470 );
    }
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
    jpv::ParticleTransferServerMessage& servMes,
    jpv::ParticleTransferClientMessage& clntMes,
    Argument &param,
    ParticleMonitor& pm,
    MultiVolumePropertyList& mvpl, 
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    std::string particlePath,
    std::string tfFilePath_old
)
{
    // read minmax & histrogram
    strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
    servMes.m_camera = clntMes.m_camera;
    servMes.m_server_status = 0;
    servMes.m_number_particle = 0;
    servMes.m_number_glyph = 0;
    servMes.m_flag_send_bins = 1;
    servMes.m_number_volume_divide = 1;
    servMes.m_transfer_function_count = 0;
    servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
    servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
    servMes.m_number_ingredients = pm.particleHistoryFile().nVariables();
    servMes.m_subpixel_level = pm.getSubpixelLevel();
    servMes.m_particle_limit = pm.particleHistoryFile().ParticleLimit();
    servMes.m_particle_density = pm.particleHistoryFile().ParticleDensity();

    std::string filename( particlePath );
    filename.append( "_pfi_coords_minmax.txt" );
    vismodule::File f( filename.c_str()  );
    if ( f.isExisted() )
    {
        // ファイルがある
        FILE* fp = NULL;
        fp = fopen( filename.c_str(), "r" );
        fscanf( fp, "%f %f %f %f %f %f",
                &servMes.m_min_object_coord[0],
                &servMes.m_min_object_coord[1],
                &servMes.m_min_object_coord[2],
                &servMes.m_max_object_coord[0],
                &servMes.m_max_object_coord[1],
                &servMes.m_max_object_coord[2]);
        if ( fp != NULL ) fclose( fp );
    }
    else
    {
        servMes.m_min_object_coord[0]=0.f;
        servMes.m_min_object_coord[1]=0.f;
        servMes.m_min_object_coord[2]=0.f;
        servMes.m_max_object_coord[0]=0.1;
        servMes.m_max_object_coord[1]=0.1;
        servMes.m_max_object_coord[2]=0.1;
    }

    if( pm.stepExisted() )
    {
        pm.setTimeStep_particle(pm.particleStatusFile().getLatestTimeStep());
    }
    else
    {
        pm.setTimeStep_particle(0);
    }
    
    pm.readParticleHistoryFile();
    servMes.m_server_side_variable_range = pm.particleHistoryFile().variableRange();

    const int tf_number = pm.particleHistoryFile().colorHistogramArray().size();
    servMes.m_transfer_function_count = tf_number; // TF_COUNT
    servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
    servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];
    servMes.m_color_bins.resize( tf_number );
    servMes.m_opacity_bins.resize( tf_number );

    for ( int tf = 0; tf < tf_number; tf++ )
    {
        servMes.m_color_nbins[tf] = DEFAULT_NBINS;
        servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
        servMes.m_color_bins[tf]   =  new vismodule::UInt64[ servMes.m_color_nbins[tf] ];
        servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ servMes.m_opacity_nbins[tf] ];
        for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
        {
            servMes.m_color_bins[tf][res] = 0;
        }
        for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
        {
            servMes.m_opacity_bins[tf][res] = 0;
        }
    }

    for ( int tf = 0; tf < tf_number; tf++ )
    {
        servMes.m_color_nbins[tf] = pm.particleHistoryFile().colorHistogramArray()[ tf ].size();
        for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
        {
            servMes.m_color_bins[tf][res] = pm.particleHistoryFile().colorHistogramArray()[ tf ][res];
        }
    }

    for ( int tf = 0; tf < tf_number; tf++ )
    {
        servMes.m_opacity_nbins[tf] = pm.particleHistoryFile().opacityHistogramArray()[ tf ].size();
        for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
        {
            servMes.m_opacity_bins[tf][res] = pm.particleHistoryFile().opacityHistogramArray()[ tf ][ res ];
        }
    }

    // servMes.m_min_value = mvpl.m_total_min_value;
    // servMes.m_max_value = mvpl.m_total_max_value;
    // servMes.m_number_nodes = mvpl.m_total_number_nodes;
    // servMes.m_number_elements = mvpl.m_total_number_elements;

    // 20181226 start  環境変数で指定したパスおよび名前でファイル参照を行う
    //初期化 : jupiter_old.tfを読む
    ParameterFileReader ppr;
    //ppr.readParameterFile("jupiter_old.tf");
    ppr.readParameterFile( tfFilePath_old.c_str() );
    NameListFile nm = ppr.getNameListFile();
    //最初の送信(daemon->client)
    //jupiter_old.tfの内容をクライアントに送信
    std::ifstream file(tfFilePath_old.c_str());
    if(file)
    {
        ppr.outputMessage( &servMes );
    }
    else
    { 
        setDefalutTransferFunction(&servMes, pm.particleHistoryFile().colorHistogramArray().size() );
    }
    file.close();

    servMes.show();
    servMes.m_message_size = servMes.byteSize();
    pts.sendMessage( servMes );
    delete servMes.m_camera;
}

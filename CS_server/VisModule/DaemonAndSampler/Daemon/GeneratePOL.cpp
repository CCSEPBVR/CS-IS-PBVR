#include <vismodule/GeneratePOL>

void generate_plot_over_line(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    jpv::ServerMode server_mode
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
    const int tf_number  = mvpl.m_list[0].m_number_ingredients;
    const int resolution = param.m_sampling_size;

    if( rank == 0 )
    {
        SetServerMessageParameter( param, mvpl, servMes );

        // send sub volume num server message
        std::cout << "INFO: send sub volume num server message" << std::endl;
        servMes.m_message_size = servMes.byteSize();
        servMes.show();
        pts.sendMessage( servMes );     
    }

    if ( server_mode == jpv::ServerMode::CS )
    {
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
            vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
            std::vector<float> tmp_values( resolution, 0 );
            std::vector<int>   tmp_mask( resolution, 0 );
            std::vector<float> tmp_axis( resolution, 0 );
        
            // make plot over line
            if ( ( rank > 0 ) || ( mpi_size == 1 ) )
            {
                int xvl, fidx;
                fidx = mvpl.getFileIndex( vl, &xvl );
                MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                mvp.setFilePath( param.m_input_data, st, xvl );
                param.m_subvolume_id = xvl;
            
                // generate plot over line start
                try
                {
                    vismodule::VolumeObjectBase* volume = nullptr;
                    PlotOverLineGenerator pol_generator;

                    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                    }
#ifdef EXTEND_FILE_FORMAT
                    else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                    {
                        generate_volume( param, mvp, st, xvl, volume );
                    }
#endif
                    else // filetype: kvsml
                    {
                        generate_volume( param, mvp, st, volume );
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

                        std::vector<Type*> raw_pointers_vector( nvariables );
                        for ( size_t i = 0; i < nvariables; ++i )
                        {
                            raw_pointers_vector[i] = values.get()[i].get();
                        }

                        // generate particle
                        pol_generator.GeneratePOLUnstruct(
                            param,
                            raw_pointers_vector.data(),
                            nvariables,
                            coordinates.get(),
                            ncoords,
                            connections.get(),
                            ncells,
                            celltype,
                            tmp_obj
                        );
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
                        pol_generator.GeneratePOLStruct(
                            param,
                            dom,
                            raw_pointers_vector.data(),
                            nvariables,
                            tmp_obj
                        );
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
                // generate plot over line end

                // make parameter
                MakePlotOverLine( tmp_obj, resolution, tmp_values, tmp_mask, tmp_axis );
            } // make plot over line

#ifndef CPU_VER
            if ( mpi_size > 1 ) {
                jc.jobCollect_pol( tmp_axis, tmp_mask, tmp_values, &nan_error, &wid );
            }
#endif

            // send plot over line
            if ( rank == 0 )
            {
                SendPlotOverLineServerMessage( tmp_values, tmp_mask, tmp_axis, resolution, nan_error, pts, servMes );
            } // send plot over line

            delete tmp_obj;
        } // end of while(DispatchNext)
    } // server_mode == jpv::ServerMode::CS
    else // server_mode == jpv::ServerMode::IS
    {
        vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
        std::vector<float> tmp_values( resolution, 0 );
        std::vector<int>   tmp_mask( resolution, 0 );
        std::vector<float> tmp_axis( resolution, 0 );

        ParticleMonitor pm;
        pm.check();

        if( pm.stepExisted() )
        {
            pm.setTimeStep_pol( pm.particleStatusFile().getLatestTimeStep() );
        }
        else
        {
            pm.setTimeStep_pol(0);
        }
        
        // get plot over line
        pm.readPlotOverLineFile();
        pm.getPlotOverLine( tmp_obj );

        // make parameter
        MakePlotOverLine( tmp_obj, resolution, tmp_values, tmp_mask, tmp_axis );

        // send plot over line
        SendPlotOverLineServerMessage( tmp_values, tmp_mask, tmp_axis, resolution, nan_error, pts, servMes );

        delete tmp_obj;
    } // server_mode == jpv::ServerMode::IS

    nan_error = false;
}

void MakePlotOverLine(
    const vismodule::KVSMLObjectPlotOverLine* pol_object,
    const int resolution,
    std::vector<float>& values_on_line,
    std::vector<int>& mask,
    std::vector<float>& x_axis
)
{
    for( size_t i = 0; i < resolution; i++ )
    { 
        x_axis[i] = pol_object->x_axis()[i];
        if (pol_object->mask()[i])
        {
            mask[i]           = 1;
            values_on_line[i] = pol_object->values_on_line()[i];
        }
    }
}

void SendPlotOverLineServerMessage(
    const std::vector<float>& values_on_line,
    const std::vector<int>& mask,
    const std::vector<float>& x_axis,
    const int resolution,
    const bool& nan_error,
    jpv::ParticleTransferServer pts,
    jpv::ParticleTransferServerMessage& servMes
)
{
    servMes.m_flag_send_bins = 3;
    servMes.m_resolution     = resolution;
    servMes.m_xAxis.resize( resolution );
    servMes.m_mask.resize( resolution );
    servMes.m_line_values.resize( resolution );

    for ( int i = 0; i < resolution; ++i )
    {
        servMes.m_xAxis[i]       = x_axis[i];   
        servMes.m_line_values[i] = values_on_line[i];   
        servMes.m_mask[i]        = mask[i];
    }

    // TEST START 2015.1.14
    if ( nan_error )
    {
        strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
        servMes.m_server_status   = 1;
        servMes.m_number_particle = 0;
        servMes.m_number_glyph    = 0;
        servMes.m_flag_send_bins  = 1;
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
}

#if 0

void generate_plot_over_line_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, 
#ifndef CPU_VER
                         JobCollector& jc, 
#endif
                         JobDispatcher& jd, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param)
{
#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
	int mpi_size = 1;
#endif
    
    char* buf;
    int bsz = 0;
    int st, vl, wid = 0;


                if ( clntMes.m_time_parameter == 0 )
                {
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                }
                else
                {
                    timer_count++;
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_input_data_base = clntMes.m_input_directory;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;

                    mvpl.searchFile(param);

                    //VariableRange range = Calculate_minmax_glyph( param, mvpl, clntMes); 
                    Calculate_minmax_glyph(param, mvpl, clntMes);
                    transfunc_creator.setFilterInfo( mvpl.m_list[0] );
                   transfunc_creator.setProtocol( clntMes );
                    transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                    param.m_transfunc_synthesizer = transfunc_creator.create();
//
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

//                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size );

                    param.m_sampling_step = CalculateSamplingStep( mvpl );
                    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );
                    param.m_particle_limit_pre = param.m_particle_limit;
                    
                    int cnt = 2 ;

                    const int resolution = clntMes.m_sampling_size;
                    std::vector<float> tmp_values(resolution); 
                    std::vector<int> tmp_mask(resolution,0); 
                    std::vector<float> tmp_axis(resolution); 

                    while ( jd.dispatchNext( wid, &st, &vl ) )
                    {
                        PlotOverLineGenerator pol_generator;
                        int xvl, fidx;
                        fidx = mvpl.getFileIndex( vl, &xvl );
                        MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                        pol_generator.setFinlterInfo( &mvpl.m_list[fidx] );

                        mvp.setFilePath(param.m_input_data, st, xvl);
                        vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
//                                //object = glyph_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl );
//                                *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 
                            }
#ifdef EXTEND_FILE_FORMAT
                            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                            {
                                pol_generator.run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, tmp_obj, st, xvl );                         
                            }
#endif
                            else     // filetype: kvsml
                            {
                                pol_generator.run( param, *clntMes.m_camera, clntMes, timeStep, mvpl.m_total_number_subvolumes , tmp_obj, st );
                            }
                           
                            for(int i =0; i < resolution; i++)
                            { 
                                tmp_axis[i] = tmp_obj->x_axis()[i];
                                if (tmp_obj->mask()[i]) 
                                {
                                    //tmp_mask[i] = tmp_obj ->mask()[i];
                                    tmp_mask[i] = 1;
                                    tmp_values[i] = tmp_obj->values_on_line()[i];
                                }
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
                        jc.jobCollect_pol( tmp_axis, tmp_mask, tmp_values, &nan_error, &wid );
#endif
                        if ( nan_error )
                        {
                            nan_error = false;
                            continue;
                        }

                    } // end of while(DispatchNext)

                    if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 1 );
                        VIS_MODULE_TIMER_FIN();
                    }
                    delete param.m_transfunc_synthesizer;
                }


}


void generate_plot_over_line_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, ParticleMonitor& pm, vismodule::Timer& timer,
                         std::string particlePath, std::string plotOverLineParameterPath, std::string plotOverLineParameterPath_old,
                         int& timer_count , const jpv::InitializeParameter init_param )
{
    int bsz = 0;
    int st, vl, wid = 0;
    timer_count++;
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                       VIS_MODULE_TIMER_STA( 461 );
                    }

                    std::cout << "initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                    if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
                    {

                        std::cout << "sampling method = " << clntMes.m_sampling_method << std::endl;
                        std::cout << "subpixel level = " << clntMes.m_subpixel_level << std::endl;
                        std::cout << "repeat level = " << clntMes.m_repeat_level << std::endl;
                    }
                    std::cout << "timeParam = " << clntMes.m_time_parameter << std::endl;

                    if ( clntMes.m_time_parameter == 0 )
                    {
                        std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                    }
                    else if ( clntMes.m_time_parameter == 1 )
                    {
                        std::cout << "beginTime = " << clntMes.m_begin_time << std::endl;
                        std::cout << "endTime = " << clntMes.m_last_time << std::endl;
                        std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        std::cout << "step = " << clntMes.m_step << std::endl;
                    }
                    std::cout << "transParam = " << clntMes.m_trans_parameter << std::endl;
                    if ( clntMes.m_trans_parameter == 1 )
                    {
                        std::cout << "levelIndex = " << clntMes.m_level_index << std::endl;
                    }
                    if ( clntMes.m_time_parameter == 0 )
                    {
                        strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                        servMes.m_time_step = clntMes.m_step;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        servMes.m_flag_send_bins = 1;

                        servMes.m_message_size = servMes.byteSize();

                        std::cout<<"main.cpp:L509"<<std::endl;
                        clntMes.show();

                        pts.sendMessage( servMes );
                    }
                    else if ( clntMes.m_time_parameter == 1 )
                    {

                        strncpy( servMes.m_header, "JPTP /1.0 130 OK\r\n", 18 );
                        servMes.m_time_step = clntMes.m_step;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        servMes.m_flag_send_bins = 1;

                        servMes.m_message_size = servMes.byteSize();

                        std::cout<<"main.cpp:L526"<<std::endl;
                        clntMes.show();

                        pts.sendMessage( servMes );
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        TimerInitialize();
                        TimerStart( 10 );
                        strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        param.m_sampling_method = clntMes.m_sampling_method;
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                       
                        mvpl.m_total_number_subvolumes=1;
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size );
                            servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_STA( 470 );
                        }

                        param.m_sampling_step = CalculateSamplingStep( mvpl );

                        VariableRange vr;

                        pm.check();
                        servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                        servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                        if( pm.stepExisted() )
                        {
                            if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step )
                            {
                                servMes.m_time_step = clntMes.m_step;
                            }
                            else
                            {
                                servMes.m_time_step = pm.particleStatusFile().getLatestTimeStep();
                                clntMes.m_step = servMes.m_time_step;
                            }
                        }
                        else
                        {
                            std::cout << " no step !!!!!!!!!!!" << std::endl;
                            clntMes.m_step = -1;
                        }

                        std::cout<<"main.cpp:L1319"<<std::endl;
                        //clntMes.show();

                        pts.sendMessage( servMes );


                        timer.start();

                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 471 );
                            }

//jupiter start
//                          TimerInitialize();
                            TimerStart( 1 );
                            ParameterFileWriter ppw;
                            ParameterFileReader ppr;

                            //clntMes.show();
                            // 20181226 start　環境変数で指定したパスおよび名前でファイル参照を行う
                            ppw.inputPlotOverLineParameterMessage( clntMes );
                            ppr.readParameterFile( plotOverLineParameterPath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();

                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( plotOverLineParameterPath.c_str() );
                            }


                            vismodule::KVSMLObjectPlotOverLine* originalObject = new vismodule::KVSMLObjectPlotOverLine;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
//                            std::string filename( plotOverLineFilePath );
                            std::string filename( particlePath );
                            // 20181226 end
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

                            TimerStop( 2 );
                            if( pm.setTimeStep_pol( clntMes.m_step ) || pm.stepExisted() ) servMes.m_flag_send_bins = 3; //plot over line
                            else                                 servMes.m_flag_send_bins = 1;
                            if( servMes.m_flag_send_bins == 3)
                            {
                                pm.readPlotOverLineFile();
                                pm.getPlotOverLine( originalObject );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            servMes.m_subpixel_level = pm.getSubpixelLevel();
                            
                            int resolution = originalObject->x_axis().size();
                            servMes.m_resolution = resolution;
                            servMes.m_xAxis.clear();
                            servMes.m_line_values.clear();
                            servMes.m_mask.clear();
                            for ( int i = 0; i < resolution; ++i )
                            {
                                servMes.m_xAxis.push_back(  originalObject->x_axis()[i]);
                                servMes.m_line_values.push_back(  originalObject->values_on_line()[i]);
                                if(originalObject->mask()[i]) servMes.m_mask.push_back( 1 );
                                else servMes.m_mask.push_back( 0 );
                            }

                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 471 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 472 );
                            }
                            servMes.m_message_size = servMes.byteSize();
                            TimerStart( 4 );

                            std::cout<<"main.cpp:L1497"<<std::endl;

                            servMes.show();
                            pts.sendMessage( servMes );
                            TimerStop( 4 );
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 472 );
                            }
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                               VIS_MODULE_TIMER_STA( 473 );
                            }
                            delete originalObject;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 473 );
                            }
                            TimerStop( 1 );
                        } // end of while(DispatchNext)
                        servMes.m_flag_send_bins = 1;
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        TimerStart( 11 );
                        pts.sendMessage( servMes );
                        TimerStop( 11 );
                        timer.stop();
                        std::cout << "Particle File: " << timer.sec() << " [sec/step]" << std::endl;

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_END( 470 );
                        }
                        TimerStop( 10 );
                        TimerFinish( servMes.m_time_step );
                    } // end of timeParam == 2
                    else
                    {
                        //break;
                        return;
                    }
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 461 );
                    }
                    if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 1 );
                        VIS_MODULE_TIMER_FIN();
                    }
                    


}
#endif
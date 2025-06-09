void generate_plot_over_line_master(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, std::vector<PointObjectCreator>& point_creator_lst ,JobCollector& jc, JobDispatcher& jd,  jpv::ParticleTransferServer pts,
                         bool& useAllNodes, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param )
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

                    timer_count++;
//                    std::vector<POLObjectGenerator> pol_generator_lst;
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_STA( 461 );
                    }

                    // send cltMes to all worker process >>
                    bsz = clntMes.byteSize();
#ifndef CPU_VER
                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                    buf = new char[bsz];
                    clntMes.pack( buf );
#ifndef CPU_VER
                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                    delete[] buf;
                    // send cltMes to all worker process <<

                    std::cout << "Recieve message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
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
                        servMes.m_number_glyph = 0 ;
                        servMes.m_flag_send_bins = 1;

                        servMes.m_message_size = servMes.byteSize();
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
                        pts.sendMessage( servMes );
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_time_step = clntMes.m_step;
                        servMes.m_level_index = clntMes.m_level_index;
                        servMes.m_repeat_level = clntMes.m_repeat_level;
                        param.m_sampling_method = clntMes.m_sampling_method;
                        param.m_component_Id = clntMes.m_rendering_id;
                        param.m_crop.setEnable( clntMes.m_enable_crop_region );
                        param.m_crop.set( clntMes.m_crop_region );
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

                     param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

                        if ( clntMes.m_node_type == 'a' )
                        {
                            useAllNodes = true;
                        }
                        else if ( clntMes.m_node_type == 's' )
                        {
                            useAllNodes = false;
                        }
                        else
                        {
                            assert( false );
                        }
                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                        if ( param.m_crop.isEnabled() )
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size,
                                           param.m_crop.getMinCoord(),
                                           param.m_crop.getMaxCoord() );
                            servMes.m_number_volume_divide = jd.getCountVolumes();
                        }
                        else
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                           mvpl.m_total_min_subvolume_coord,
                                           mvpl.m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size );
                            servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                        }

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_STA( 470 );
                        }

                        param.m_sampling_step = CalculateSamplingStep( mvpl );

                        param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );

                        VariableRange vr;
                        pts.sendMessage( servMes );

                        //add by shimomura 2023/06/14
                        int cnt = 2;

                        // 関数の領域確保、初期化を行う : by @hira 2016/12/01
                        servMes.initializeTransferFunction(clntMes.m_transfer_function.size(), DEFAULT_NBINS);
 
                        const int resolution = clntMes.m_sampling_size;
                        vismodule::KVSMLObjectPlotOverLine* originalGlyph = new vismodule::KVSMLObjectPlotOverLine;
                        servMes.m_resolution = resolution;
                        servMes.m_xAxis.resize(resolution);
                        servMes.m_mask.resize(resolution);
                        servMes.m_line_values.resize(resolution);
                        std::vector<float> tmp_values(resolution); 
                        std::vector<int> tmp_mask(resolution,0); 
                        std::vector<float> tmp_axis(resolution); 
                         for (int i =0; i < resolution; ++i)
                         {
                             servMes.m_mask[i] = 0;
                         } 

                        originalGlyph -> setResolution(resolution);
                        originalGlyph ->mask().fill(false);
                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
                            POLObjectGenerator pol_generator;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_STA( 471 );
                            }

                            if (mpi_size == 1) 
                            {
                            int xvl, fidx;
                            fidx = mvpl.getFileIndex( vl, &xvl );
                            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                            pol_generator.setFinlterInfo( &mvpl.m_list[fidx] );

                            mvp.setFilePath(param.m_input_data, st, xvl);
                            vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
                            param.m_subvolume_id = xvl ;
                            int timeStep = 1;
                            servMes.m_flag_send_bins = 2;
                            try
                            {
                                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                                {
                                    // run()で得られるKVSMLObjectglyphとtmp_objは異なるメモリ領域を指しているため,ポインタコピーではなくオペレータを呼び出す必要がある
                                }
#ifdef EXTEND_FILE_FORMAT
                                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                                {
                                    pol_generator.run( param, *clntMes.m_camera, clntMes, servMes.m_number_volume_divide, tmp_obj, st, xvl );
                                }
#endif
                                else     // filetype: kvsml
                                {
                                    pol_generator.run( param, *clntMes.m_camera, clntMes, timeStep,servMes.m_number_volume_divide , tmp_obj, st );
                                }

                                //集約処理
                                
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

                            } //end if mpi_size == 1
#if 1
#ifndef CPU_VER          // MPI並列については一旦保留, collectorの内容がわかるまで
                            if (mpi_size > 1) {
                                //jc.jobCollect( originalObject, &vr, &nan_error, &wid );
                                jc.jobCollect_pol( tmp_axis, tmp_mask, tmp_values, &nan_error, &wid );
                            }
#endif
#endif
//							printf(" %zu glyphs generated\n", object->coords().size() / 3);

                            for (int i =0; i < resolution; ++i)
                            {
                                servMes.m_xAxis[i] = tmp_axis[i];   
                                servMes.m_line_values[i] = tmp_values[i];   
                                servMes.m_mask[i]  = tmp_mask[i];
                            }

//                           //add by shimomura 2023/06/14
                            //if ( originalGlyph != object ) delete originalGlyph;

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
                            //delete object;
                            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                            {
                                VIS_MODULE_TIMER_END( 473 );
                            }
                        } // end of while(DispatchNext)
                           
                        // TEST START 2015.1.14
                        if ( nan_error )
                        {
                            strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                            servMes.m_server_status = 1;
                            servMes.m_number_particle = 0;
                            servMes.m_number_glyph = 0 ;
                            servMes.m_flag_send_bins = 1;
                            std::cout << "!!!!!!!!!!!! Send serverStatus = 1 " << std::endl;
                            nan_error = false;
                        }

                        servMes.m_flag_send_bins = 3;
                        servMes.m_subpixel_level = param.m_subpixel_level;
                        servMes.m_message_size = servMes.byteSize();
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
                        servMes.m_flag_send_bins = 1;

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_END( 470 );
                        }
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

void generate_plot_over_line_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, std::vector<PointObjectCreator>& point_creator_lst ,JobCollector& jc, JobDispatcher& jd,  
                         bool& useAllNodes, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param)
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
//                  param.m_transfer_function = vismodule::TransferFunction(); // *( clntMes.m_transfer_function );
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_component_Id = clntMes.m_rendering_id;
                    param.m_crop.setEnable( clntMes.m_enable_crop_region );
                    param.m_crop.set( clntMes.m_crop_region );
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
                    if ( param.m_crop.isEnabled() )
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size,
                                       param.m_crop.getMinCoord(),
                                       param.m_crop.getMaxCoord() );
                    }
                    else
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size );
                    }

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
                        POLObjectGenerator pol_generator;
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
                        //jc.jobCollect_pol( tmp_obj, &nan_error, &wid );
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

#if 0
void generate_pol()
{

                if ( clntMes.m_time_parameter == 0 )
                {
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                }
                else
                {
                    timer_count++;
//                  param.m_transfer_function = vismodule::TransferFunction(); // *( clntMes.m_transfer_function );
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_component_Id = clntMes.m_rendering_id;
                    param.m_crop.setEnable( clntMes.m_enable_crop_region );
                    param.m_crop.set( clntMes.m_crop_region );
                    param.m_input_data_base = clntMes.m_input_directory;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;

                    size_t found_pfl  = param.m_input_data_base.find(".pfl");
                    size_t found_pfi  = param.m_input_data_base.find(".pfi");
                    size_t found_vtm  = param.m_input_data_base.find(".vtm");
                    size_t found_vtu  = param.m_input_data_base.find(".vtu");
                    size_t found_vti  = param.m_input_data_base.find(".vti");
                    size_t found_inp  = param.m_input_data_base.find(".inp");
                    size_t found_pvtu = param.m_input_data_base.find(".pvtu");
                    size_t found_case = param.m_input_data_base.find(".case");

                    if ( found_pfl != std::string::npos )
                    {
                        std::string pflfile = param.m_input_data_base;
                        std::cout << "pflファイルが選択されました" << std::endl;
                        vismodule::File pfl( pflfile );
                        if ( pfl.isExisted() )
                        {
                            mvpl.loadPFL( pflfile );
                        }
                     }
                    else if ( found_pfi != std::string::npos )
                    {
                        std::string pfifile = param.m_input_data_base;
                        std::cout << "pfiファイルが選択されました" << std::endl;
                        vismodule::File pfi( pfifile );
                        if ( pfi.isExisted() )
                        {
                            mvpl.loadPFL( pfifile );
                        }                        
                    }
#ifdef EXTEND_FILE_FORMAT
                    else if ( found_vtm != std::string::npos )
                    {
                        std::string vtmfile = param.m_input_data_base;
                        std::cout << ".vtmファイルが選択されました" << std::endl;
                        size_t found_asterisk = vtmfile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadVtm( vtmfile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesVtm( vtmfile );
                        }
                    }
                    else if ( found_vtu != std::string::npos )
                    {
                        std::string vtufile = param.m_input_data_base;
                        std::cout << ".vtuファイルが選択されました" << std::endl;
                        size_t found_asterisk = vtufile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadVtu( vtufile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesVtu( vtufile );
                        }
                    }
                    else if ( found_vti != std::string::npos )
                    {
                        std::string vtifile = param.m_input_data_base;
                        std::cout << ".vtiファイルが選択されました" << std::endl;
                        size_t found_asterisk = vtifile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadVti( vtifile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesVti( vtifile );
                        }
                    }
                    else if ( found_inp != std::string::npos )
                    {
                        std::string inpfile = param.m_input_data_base;
                        std::cout << ".inpファイルが選択されました" << std::endl;
                        size_t found_asterisk = inpfile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadInp( inpfile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            std::cout << ".inpファイルは連番ファイルに対応していません" << std::endl;
                        }
                    }
                    else if ( found_pvtu != std::string::npos )
                    {
                        std::string pvtufile = param.m_input_data_base;
                        std::cout << ".pvtuファイルが選択されました" << std::endl;
                        size_t found_asterisk = pvtufile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadPvtu( pvtufile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesPvtu( pvtufile );
                        }
                    }
                    else if ( found_case != std::string::npos )
                    {
                        std::string casefile = param.m_input_data_base;
                        std::cout << ".caseファイルが選択されました" << std::endl;
                        size_t found_asterisk = casefile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadEnsightGold( casefile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            std::cout << ".caseファイルは連番ファイルに対応していません" << std::endl;
                        }
                    }                    
#endif
                    else
                    {
                        std::cout << "このファイルは現在対応していません" << std::endl;
                    }

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
                    if ( param.m_crop.isEnabled() )
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size,
                                       param.m_crop.getMinCoord(),
                                       param.m_crop.getMaxCoord() );
                    }
                    else
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size );
                    }

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
                        POLObjectGenerator pol_generator;
                        int xvl, fidx;
                        fidx = mvpl.getFileIndex( vl, &xvl );
                        MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                        pol_generator.setFinlterInfo( &mvpl.m_list[fidx] );

                        size_t found_pfi  = mvp.m_file_path.find(".pfi");
                        size_t found_vtm  = mvp.m_file_path.find(".vtm");
                        size_t found_vtu  = mvp.m_file_path.find(".vtu");
                        size_t found_vti  = mvp.m_file_path.find(".vti");
                        size_t found_inp  = mvp.m_file_path.find(".inp");
                        size_t found_pvtu = mvp.m_file_path.find(".pvtu");
                        size_t found_case = mvp.m_file_path.find(".case");
                        
                        if ( found_pfi != std::string::npos )
                        {
                            std::stringstream suffix;
                            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                            << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                            << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                            vismodule::File ifpx( mvp.m_file_path );
                            param.m_input_data = ifpx.pathName() + ifpx.Separator()
                            + ifpx.baseName() + suffix.str() + ".kvsml";
                        }
#ifdef EXTEND_FILE_FORMAT
                        else if ( found_vtm  != std::string::npos ||
                                  found_vtu  != std::string::npos ||
                                  found_vti  != std::string::npos ||
                                  found_inp  != std::string::npos ||
                                  found_pvtu != std::string::npos ||
                                  found_case != std::string::npos 
                                )
                        {
                            param.m_input_data = mvp.m_file_path;
                        }
#endif
                        else
                        {
                            std::cout << "このファイルは現在対応していません" << std::endl;
                        }                        
                        
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
                        //jc.jobCollect_pol( tmp_obj, &nan_error, &wid );
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
#endif

#include <vismodule/InitialStep>

void initial_step_master(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, 
#ifndef CPU_VER
                         JobCollector& jc, 
#endif
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count )
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
    
    int bsz = 0;
    int st, vl, wid = 0;
    std::vector<vismodule::PointObjectGenerator> point_generator_lst;

                    param.m_input_data_base = clntMes.m_input_directory;

                    bool open_flag = true; 
                    std::ifstream fin( param.m_input_data_base, std::ios::in);
                    /*
                    if (!fin.is_open()) 
                    {
                        std::cout << "ファイルを開けませんでした: " << param.m_input_data_base << std::endl;
                        open_flag = false;
                    }
                    */
      
                    bool ExtendFileFormat_flag = true;
                    bool pfi_flag = true;
#ifndef EXTEND_FILE_FORMAT 
                    ExtendFileFormat_flag = false;
                    
                    {          
                        pfi_flag = false;
                        size_t found_pfl = param.m_input_data_base.find(".pfl");
                        size_t found_pfi = param.m_input_data_base.find(".pfi");
                        if (found_pfl != std::string::npos) pfi_flag = true;
                        if (found_pfi != std::string::npos) pfi_flag = true;
                    }
#endif
                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    // ADD by FEAST 2015.12.24
                    //servMes.m_server_status = 0;
                    // ADD END 2015.12.24
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;
                    servMes.m_transfer_function_count = 0;
                    servMes.m_message_size = servMes.byteSize();
                    std::cout << "open_flag = " << open_flag << ", ExtendFileFormat_flag = " << ExtendFileFormat_flag << ", pfi_flag = " << pfi_flag << std::endl; 
                    if (open_flag == true && pfi_flag == true) servMes.m_file_enable_flag = jpv::FileEnableFlag::Enable_VTK ;
                    if (open_flag == true && ExtendFileFormat_flag == false && pfi_flag == false) servMes.m_file_enable_flag = jpv::FileEnableFlag::NotEnable_VTK;
                    if (open_flag == false) servMes.m_file_enable_flag = jpv::FileEnableFlag::NoFile ;
                    pts.sendMessage( servMes );
 
                    if(servMes.m_file_enable_flag == jpv::FileEnableFlag::NotEnable_VTK || servMes.m_file_enable_flag == jpv::FileEnableFlag::NoFile) 
                    //if( warning_flag == 2) 
                    {
                        if ( rank == 0 ) std::cerr << "Error: pfifile doesn't exist" << std::endl;
                        bsz = -1;
#ifndef CPU_VER
                        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
 
//#ifndef CPU_VER               // 開けなくても停止しないよう変更  予定 
//                        MPI_Finalize();
//#endif
//                        return 0;
                     
                        //continue;
                        return;
                    }
                   

#if 0
                    std::string pfifile = param.m_input_data_base + ".pfi";
                    vismodule::File pfi( pfifile );
                    std::string pflfile = param.m_input_data_base + ".pfl";
                    vismodule::File pfl( pflfile );
                    if ( pfl.isExisted() )
                    {
                        mvpl.loadPFL( pflfile );
                    }
                    else if ( pfi.isExisted() )
                    {
                        mvpl.loadPFL( pfifile );
                    }
#else
                    mvpl.searchFile(param);
#endif
                    if ( mvpl.m_list.size() > 0 )
                    {
                        point_generator_lst.clear();
                        for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                        {
                            vismodule::PointObjectGenerator point_generator;
                            point_generator.setFilterInfo( &mvpl.m_list[idx] );
                            point_generator.setCoordSynthStr( clntMes.m_x_synthesis,
                                                            clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                            point_generator_lst.push_back( point_generator );
                        }

                        transfunc_creator.setFilterInfo( mvpl.m_list[0] );

                        std::cout << " time step = "          << mvpl.m_total_number_steps
                                  << " subvolume division = " << mvpl.m_total_number_subvolumes
                                  << std::endl;

                    // send cltMes to all worker process >>
                    bsz = clntMes.byteSize();
#ifndef CPU_VER
                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                    char* buf;
                    buf = new char[bsz];
                    clntMes.pack( buf );
#ifndef CPU_VER
                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                    delete[] buf;
                    // send cltMes to all worker process <<

                    }
                    else
                    {
                        if ( rank == 0 ) std::cerr << "Error: pfifile doesn't exist" << std::endl;
                        bsz = -1;
#ifndef CPU_VER
                        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

                        
#ifndef CPU_VER               // 開けなくても停止しないよう変更  予定 
                        MPI_Finalize();
#endif
                        //return 0;
                        strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                        // ADD by FEAST 2015.12.24
                        //servMes.m_server_status = 0;
                        // ADD END 2015.12.24
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0 ;
                        servMes.m_flag_send_bins = 1;
                        servMes.m_message_size = servMes.byteSize();
                        pts.sendMessage( servMes );
                       
                        return;
                    }

                    int nvariable;
                    VariableRange range = Calculate_minmax( param, mvpl);
                    if( !clntMes.m_import_flag ) 
                    {
                        std::cout << "defalt parameter " << std::endl;
                        nvariable = mvpl.m_total_number_ingredients;
                        transfunc_creator.setInitialProtocol( nvariable, range );
                    }
                    else
                    {
                        std::cout << "user define parameter " << std::endl;
                        nvariable = clntMes.m_transfer_function.size();
                        transfunc_creator.setProtocol(clntMes);
                        servMes.m_glyph_color_min  =  clntMes.m_transfer_function[0].m_color_variable_min;
                        servMes.m_glyph_color_max  =  clntMes.m_transfer_function[0].m_color_variable_max; 
                        servMes.m_glyph_size_min   =  clntMes.m_transfer_function[0].m_color_variable_min;
                        servMes.m_glyph_size_max   =  clntMes.m_transfer_function[0].m_color_variable_max; 
                    }
#if 1
                    // generate_histogram
                    param.m_sampling_method = 'h';
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;

                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }
                    
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    jd.initialize( mvpl.m_total_start_steps, mvpl.m_total_start_steps, mvpl.m_total_number_subvolumes,
                            mvpl.m_total_min_subvolume_coord,
                            mvpl.m_total_max_subvolume_coord,
                            param.m_latency_threshold, param.m_job_id_pack_size );
                    servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;

                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_STA( 470 );
                    }

                    param.m_sampling_step = CalculateSamplingStep( mvpl );
                    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );

                    VariableRange vr;
//                    pts.sendMessage( servMes );

                    // 関数の領域確保、初期化を行う : by @hira 2016/12/01
                    servMes.initializeTransferFunction(nvariable, DEFAULT_NBINS);

                    //int tf_count = clntMes.m_transfer_function.size();
                    int tf_count = nvariable;
                    int c_bins_size = 0;
                    int o_bins_size = 0;
                    vismodule::UInt64* tmp_c_bins;
                    vismodule::UInt64* tmp_o_bins;
                    float*  tmp_max;
                    float*  tmp_min;
                    for ( int tf = 0; tf < tf_count; tf++ )
                    {
                        c_bins_size += servMes.m_color_nbins[tf];
                        o_bins_size += servMes.m_opacity_nbins[tf];
                    }

                    tmp_c_bins = new vismodule::UInt64[c_bins_size];
                    tmp_o_bins = new vismodule::UInt64[o_bins_size];

                    //add by shimomura 2023/06/14
                    int cnt = 2* servMes.m_transfer_function_count ;
                    tmp_max = new float[cnt]; 
                    tmp_min = new float[cnt];

                    for ( int tf = 0; tf < cnt; tf++ )
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

                    std::cout << "servMes.m_server_status = " << servMes.m_server_status <<std::endl;
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
                            fidx = mvpl.getFileIndex( vl, &xvl );
                            MultiVolumeProperty& mvp = mvpl.m_list[fidx];

                            mvp.setFilePath(param.m_input_data, st, xvl);
                            vismodule::PointObject* tmp_obj = NULL;
                            param.m_subvolume_id = xvl;
                            int timeStep = 1;
                            try
                            {
                                point_generator_lst[fidx].setCoordSynthStr( clntMes.m_x_synthesis,
                                        clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                                {
                                    tmp_obj = point_generator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                                }
                                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                                {
                                    tmp_obj = point_generator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                                }
                                else     // filetype: kvsml
                                {
                                    tmp_obj = point_generator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st );
                                }

                                size_t nmemb = tmp_obj->nvertices() * 3;
                                // modify by @hira at 2016/12/01  
                                int c_count = 0;
                                for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
                                {
                                    int c_nbins = tmp_obj->getNbins();
                                    //changed by shimomura 2023/07/24
                                    tmp_max[2*tf+1] = vismodule::Math::Max(tmp_max[2*tf+1],param.m_transfunc_synthesizer-> m_c_max[tf]);
                                    tmp_min[2*tf+1] = vismodule::Math::Min(tmp_min[2*tf+1],param.m_transfunc_synthesizer-> m_c_min[tf]);
                                    for ( int res = 0; res < c_nbins; res++ )
                                    {
                                        tmp_c_bins[ c_count ] += tmp_obj->getCHistogram()[ c_count ] ;
                                        c_count++;

                                    }
                                }
                                int o_count = 0;
                                for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
                                {
                                    int o_nbins = tmp_obj->getNbins();
                                    //changed by shimomura 2023/07/24
                                    tmp_max[2*tf] = vismodule::Math::Max(tmp_max[2*tf],param.m_transfunc_synthesizer-> m_c_max[tf]);
                                    tmp_min[2*tf] = vismodule::Math::Min(tmp_min[2*tf],param.m_transfunc_synthesizer-> m_c_min[tf]);
                                    for ( int res = 0; res < o_nbins; res++ )
                                    {
                                        tmp_o_bins[o_count] += tmp_obj->getOHistogram()[ o_count ] ;
                                        o_count++;
                                    }
                                }

                            }
                            catch ( const std::runtime_error& e )
                            {
#ifdef _DEBUG          // debug by @hira
                                printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                                std::cerr << e.what();
                                nan_error = true;
                            }

                        }

#ifndef CPU_VER
                        if (mpi_size > 1) {
                            jc.jobCollect( originalObject, &vr, &nan_error, &wid );
                        }
#endif
                        //int nvertices = originalObject->coords().size() / 3;

                        vismodule::PointObject* object = originalObject;
                        printf(" %zu perticles generated\n", object->coords().size() / 3);

                        //                           //add by shimomura 2023/06/14
                        if ( originalObject != object ) delete originalObject;
                        servMes.m_number_particle = object->coords().size() / 3;
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
                        MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
                        MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
                    }
#endif
                    //add by shimomura 2023/06/14
                    vr = setVariablerange2( tmp_max,tmp_min, cnt/2 );
                    servMes.m_server_side_variable_range = vr;

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
                        servMes.m_flag_send_bins = 1;
                        std::cout << "!!!!!!!!!!!! Send serverStatus = 1 " << std::endl;
                        nan_error = false;
                    }
#endif
                    strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                    servMes.m_time_step = mvpl.m_total_start_steps;
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
                    transfunc_creator.setTransferFunction(&servMes, vr); 

#if 0
                    std::cout << "\n================== client parameter start ==================" << std::endl;
                    std::cout << "servMes.m_number_particle:" << servMes.m_number_particle << std::endl;
                    std::cout << "servMes.m_number_glyph:" << servMes.m_number_glyph << std::endl;
                    std::cout << "servMes.m_number_volume_divide:" << servMes.m_number_volume_divide << std::endl;
                    std::cout << "servMes.m_time_step:" << servMes.m_time_step << std::endl;
                    std::cout << "servMes.m_start_step:" << servMes.m_start_step << std::endl;
                    std::cout << "servMes.m_last_step:" << servMes.m_last_step << std::endl;
                    std::cout << "servMes.m_number_step:" << servMes.m_number_step << std::endl;
                    std::cout << "servMes.m_min_object_coord[0]:" << servMes.m_min_object_coord[0] << std::endl;
                    std::cout << "servMes.m_min_object_coord[1]:" << servMes.m_min_object_coord[1] << std::endl;
                    std::cout << "servMes.m_min_object_coord[2]:" << servMes.m_min_object_coord[2] << std::endl;
                    std::cout << "servMes.m_max_object_coord[0]:" << servMes.m_max_object_coord[0] << std::endl;
                    std::cout << "servMes.m_max_object_coord[1]:" << servMes.m_max_object_coord[1] << std::endl;
                    std::cout << "servMes.m_max_object_coord[2]:" << servMes.m_max_object_coord[2] << std::endl;
                    std::cout << "servMes.m_min_value:" << servMes.m_min_value << std::endl;
                    std::cout << "servMes.m_max_value:" << servMes.m_max_value << std::endl;
                    std::cout << "servMes.m_number_nodes:" << servMes.m_number_nodes << std::endl;
                    std::cout << "servMes.m_number_elements:" << servMes.m_number_elements << std::endl;
                    std::cout << "servMes.m_element_type:" << servMes.m_element_type << std::endl;
                    std::cout << "servMes.m_file_type:" << servMes.m_file_type << std::endl;
                    std::cout << "servMes.m_number_ingredients:" << servMes.m_number_ingredients << std::endl;
                    std::cout << "servMes.m_opacity_transfer_function_synthesis:" << servMes.m_opacity_transfer_function_synthesis << std::endl;
                    std::cout << "servMes.m_color_transfer_function_synthesis:" << servMes.m_color_transfer_function_synthesis << std::endl;
                    std::cout << "servMes.m_transfer_function.size():" << servMes.m_transfer_function.size() << std::endl;
                    std::cout << "================== client parameter end ==================\n" << std::endl;
#endif

                    servMes.m_flag_send_bins = 1;
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


void initial_step_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error,
#ifndef CPU_VER
                         JobCollector& jc, 
#endif
                         JobDispatcher& jd, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count )
{

    vismodule::PointObject* object = NULL;
#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
	int mpi_size = 1;
#endif
    
    int bsz = 0;
    int st, vl, wid = 0;
    std::vector<vismodule::PointObjectGenerator> point_generator_lst;


    timer_count++;
    param.m_sampling_method = 'h';
    param.m_input_data_base = clntMes.m_input_directory;
    param.m_particle_limit = clntMes.m_particle_limit;
    param.m_particle_density = clntMes.m_particle_density;

#if 0
    std::string pfifile, pflfile;
    pfifile = param.m_input_data_base + ".pfi";
    vismodule::File pfi( pfifile );
    pflfile = param.m_input_data_base + ".pfl";
    vismodule::File pfl( pflfile );
    if ( pfl.isExisted() )
    {
        mvpl.loadPFL( pflfile );
    }
    else if ( pfi.isExisted() )
    {
        mvpl.loadPFL( pfifile );
    }
#else
    mvpl.searchFile(param);
#endif

    point_generator_lst.clear();
    for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
    {
        vismodule::PointObjectGenerator point_generator;
        point_generator.setFilterInfo( &mvpl.m_list[idx] );
        point_generator.setCoordSynthStr( clntMes.m_x_synthesis,
                clntMes.m_y_synthesis, clntMes.m_z_synthesis );
        point_generator_lst.push_back( point_generator );
    }

    transfunc_creator.setFilterInfo( mvpl.m_list[0] );
    int nvariable;
    VariableRange range = Calculate_minmax( param, mvpl); 
    if( !clntMes.m_import_flag ) 
    {
        std::cout << "defalt parameter " << std::endl;
        nvariable = mvpl.m_total_number_ingredients;
        transfunc_creator.setInitialProtocol( nvariable, range );
    }
    else
    {
        std::cout << "user define parameter " << std::endl;
        nvariable = clntMes.m_transfer_function.size();
        transfunc_creator.setProtocol(clntMes);
    }
    param.m_transfunc_synthesizer = transfunc_creator.create();
    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());

    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
    {
        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
    }
    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
    jd.initialize( mvpl.m_total_start_steps, mvpl.m_total_start_steps, mvpl.m_total_number_subvolumes,
            mvpl.m_total_min_subvolume_coord,
            mvpl.m_total_max_subvolume_coord,
            param.m_latency_threshold, param.m_job_id_pack_size );

    param.m_sampling_step = CalculateSamplingStep( mvpl );
    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );
    param.m_particle_limit_pre = param.m_particle_limit;

    clntMes.show();
    int tf_count = nvariable;
    int c_bins_size = 0;
    int o_bins_size = 0;
    int c_nbins = DEFAULT_NBINS;
    int o_nbins = DEFAULT_NBINS;
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float*  tmp_max;
    float*  tmp_min;
    c_bins_size = 0;
    o_bins_size = 0;

    for ( int tf = 0; tf < tf_count; tf++ )
    {
        c_bins_size += c_nbins;
        o_bins_size += o_nbins;
    }

    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];
    //add by shimomura 2023/06/14
    int cnt = 2* tf_count ;
    tmp_max = new float[cnt]; 
    tmp_min = new float[cnt]; 

    for ( int tf = 0; tf < cnt; tf++ )
    {
        tmp_max[tf] = 0;
        tmp_min[tf] = 0;
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
        fidx = mvpl.getFileIndex( vl, &xvl );
        MultiVolumeProperty& mvp = mvpl.m_list[fidx];

        mvp.setFilePath(param.m_input_data, st, xvl);
        int timeStep = 1;
        try
        {
            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
            {
                object = point_generator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl );
            }
            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
            {
                object = point_generator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
            }
            else     // filetype: kvsml
            {
                object = point_generator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st );
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
        VariableRange* p_vr = &range;
        jc.jobCollect( object, p_vr, &nan_error, &wid );
#endif
        if ( nan_error )
        {
            nan_error = false;
            continue;
        }

        int c_count = 0;
        int o_count = 0;

        for ( int tf = 0; tf < object->getTfnumber(); tf++ )
        {
            c_nbins = object->getNbins();
            //add by shimomura 2023/06/14
            tmp_max[2*tf+1] = vismodule::Math::Max( tmp_max[2*tf+1] ,param.m_transfunc_synthesizer-> m_c_max[tf]);
            tmp_min[2*tf+1] = vismodule::Math::Min( tmp_min[2*tf+1] ,param.m_transfunc_synthesizer-> m_c_min[tf]);
            for ( int res = 0; res < c_nbins; res++ )
            {
                tmp_c_bins[c_count] += object->getCHistogram()[ c_count ] ;
                c_count++;
            }
        }

        for ( int tf = 0; tf < object->getTfnumber(); tf++ )
        {
            o_nbins = object->getNbins();
            //add by shimomura 2023/06/14
            tmp_max[2*tf] = vismodule::Math::Max( tmp_max[2*tf] ,param.m_transfunc_synthesizer-> m_c_max[tf]);
            tmp_min[2*tf] = vismodule::Math::Min( tmp_min[2*tf] ,param.m_transfunc_synthesizer-> m_c_min[tf]);
            for ( int res = 0; res < o_nbins; res++ )
            {
                tmp_o_bins[o_count] += object->getOHistogram()[ o_count ] ;
                o_count++;
            }
        }

    } // end of while(DispatchNext)
#ifndef CPU_VER

    MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    //add by shimomura 20240603
    delete[] tmp_max;
    delete[] tmp_min;
#endif
    if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
    {
        VIS_MODULE_TIMER_END( 1 );
        VIS_MODULE_TIMER_FIN();
    }
    delete param.m_transfunc_synthesizer;

}

void initial_step_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         std::string particlePath, std::string glyphFilePath, std::string plotOverLineFilePath, std::string statePath, std::string  historyPath, std::string tfFilePath_old,
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, int& timer_count )
{
 
    //jupiter start
//              param.input_data_base = clntMes.m_input_directory;
                param.m_input_data_base = "dummy";

                std::cout<<"main.cpp:L221"<<std::endl;
                //clntMes.show();
//jupiter end
                VariableRange range;

                strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                servMes.m_transfer_function_count = 0;
                servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                servMes.m_time_step = mvpl.m_total_start_steps;
                servMes.m_start_step = mvpl.m_total_start_steps;
                servMes.m_last_step = mvpl.m_total_last_step;
                servMes.m_number_step = mvpl.m_total_last_step;
                //servMes.m_number_volume_divide = mvpl.m_total_numSubVolumes;
                //servMes.m_time_step = mvpl.m_total_staSteps;
                //servMes.m_start_step = mvpl.m_total_staSteps;
                //servMes.m_last_step = mvpl.m_total_endSteps;
                //servMes.m_number_step = mvpl.m_total_numSteps;
//jupiter start
#if 0
                servMes.minObjectCoord[0] = mvpl.total_minObjectCoord[0];
                servMes.minObjectCoord[1] = mvpl.total_minObjectCoord[1];
                servMes.minObjectCoord[2] = mvpl.total_minObjectCoord[2];
                servMes.maxObjectCoord[0] = mvpl.total_maxObjectCoord[0];
                servMes.maxObjectCoord[1] = mvpl.total_maxObjectCoord[1];
                servMes.maxObjectCoord[2] = mvpl.total_maxObjectCoord[2];
#else

#if 0
                //clntMesに含まれる-vinオプションで指定されたパスを使用
                //デーモン(pbvr_server)をサンプラと同じディレクトリに配置
                //相対パスでOK  ./jupiter_particle_out/t
                jupiter_prefix = clntMes.m_input_directory;
                // 存在領域設定ファイルがあるか確認をする
                std::string filename( jupiter_prefix );
#else
                // 20181226 start
                // 環境変数の値を直接使用する
                std::string filename( particlePath );
                // 20181226 end
#endif
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

#endif

//jupiter end

//// read minmax & histrogram 

                ParticleMonitor pm( particlePath, glyphFilePath, plotOverLineFilePath, statePath.c_str(), historyPath.c_str() );
                pm.check();

                servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                if( pm.stepExisted() )
                {
                    pm.setTimeStep_particle(pm.particleStatusFile().getLatestTimeStep());
                    //pm.setTimeStep(0);
                }
                else
                {
                    pm.setTimeStep_particle(0);
                }
                pm.readParticleHistoryFile();
                range = pm.particleHistoryFile().variableRange();

                const int tf_number = pm.particleHistoryFile().colorHistogramArray().size();
                servMes.m_transfer_function_count =       tf_number;//TF_COUNT
                servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
                servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];

                servMes.m_color_bins.resize(   tf_number );
                servMes.m_opacity_bins.resize( tf_number );

                for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
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

                servMes.m_transfer_function_count = pm.particleHistoryFile().colorHistogramArray().size();
                for ( int tf = 0; tf < pm.particleHistoryFile().colorHistogramArray().size() && tf < servMes.m_transfer_function_count; tf++ )
                {
                    servMes.m_color_nbins[tf] = pm.particleHistoryFile().colorHistogramArray()[ tf ].size();
                    for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                    {
                        servMes.m_color_bins[tf][res] = pm.particleHistoryFile().colorHistogramArray()[ tf ][res];
                    }
                }

                for ( int tf = 0; tf < pm.particleHistoryFile().opacityHistogramArray().size() && tf < servMes.m_transfer_function_count; tf++ )
                {
                    servMes.m_opacity_nbins[tf] = pm.particleHistoryFile().opacityHistogramArray()[ tf ].size();
                    for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                    {
                        servMes.m_opacity_bins[tf][res] = pm.particleHistoryFile().opacityHistogramArray()[ tf ][ res ];
                    }
                }

                servMes.m_min_value = mvpl.m_total_min_value;
                servMes.m_max_value = mvpl.m_total_max_value;
                servMes.m_number_nodes = mvpl.m_total_number_nodes;
                servMes.m_number_elements = mvpl.m_total_number_elements;
                servMes.m_number_ingredients = pm.particleHistoryFile().nVariables();
                servMes.m_server_side_variable_range = range;
                servMes.m_flag_send_bins = 1;
                servMes.m_number_glyph = 0;
                servMes.m_particle_limit = pm.particleHistoryFile().ParticleLimit();
                servMes.m_particle_density = pm.particleHistoryFile().ParticleDensity();

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

                std::cout<<"main.cpp:571"<<std::endl;
                servMes.show();

//                servMes.m_server_status =0;
                servMes.m_message_size = servMes.byteSize();
                pts.sendMessage( servMes );
                delete servMes.m_camera;
                delete clntMes.m_camera;

}

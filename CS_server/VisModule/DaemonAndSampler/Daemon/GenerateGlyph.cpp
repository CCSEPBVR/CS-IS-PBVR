#include <vismodule/GenerateGlyph>
void generate_glyph_master(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, 
#ifndef CPU_VER
                         JobCollector& jc,
#endif
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts,
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

#if 1
    timer_count++;
    std::vector<vismodule::GlyphSeedGenerator> glyph_creator_lst;
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

        //                        transfunc_creator.setProtocol( clntMes );
        //                        transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
        //                        param.m_transfunc_synthesizer = transfunc_creator.create();

        Calculate_minmax_glyph( param, mvpl, clntMes );
        param.m_transfunc_array.resize( transfunc_creator.transfunc().size() );
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

        float*  tmp_max;
        float*  tmp_min;
        //add by shimomura 2023/06/14
        int cnt = 2;
        tmp_max = new float[cnt]; 
        tmp_min = new float[cnt];

        for ( int tf = 0; tf < cnt; tf++ )
        {
            tmp_max[tf] = FLT_MIN;
            tmp_min[tf] = FLT_MAX;
        }

        // 関数の領域確保、初期化を行う : by @hira 2016/12/01
        servMes.initializeTransferFunction(clntMes.m_transfer_function.size(), DEFAULT_NBINS);

        while ( jd.dispatchNext( wid, &st, &vl ) )
        {
            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_STA( 471 );
            }

            vismodule::KVSMLObjectGlyph* originalGlyph = new vismodule::KVSMLObjectGlyph;

            if (mpi_size == 1) {
                int xvl, fidx;
                fidx = mvpl.getFileIndex( vl, &xvl );
                MultiVolumeProperty& mvp = mvpl.m_list[fidx];

                mvp.setFilePath(param.m_input_data, st, xvl);
                vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;
                param.m_subvolume_id = xvl;
                int timeStep = 1;
                servMes.m_flag_send_bins = 2;

                // glyph_creator_lstの初期化
                glyph_creator_lst.clear();
                for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                {
                    //GlyphObjectCreator glyph_creator;
                    vismodule::GlyphSeedGenerator glyph_creator;
                    glyph_creator.setFilterInfo( &mvpl.m_list[idx] );
                    glyph_creator_lst.push_back( glyph_creator );
                }

                try
                {
                    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                        *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 
                        // run()で得られるKVSMLObjectglyphとtmp_objは異なるメモリ領域を指しているため,ポインタコピーではなくオペレータを呼び出す必要がある
                    }
#ifdef EXTEND_FILE_FORMAT
                    else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                    {
                        glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, servMes.m_number_volume_divide, timeStep , tmp_obj, st, xvl );
                    }                                
#endif
                    else     // filetype: kvsml
                    {
                        glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, servMes.m_number_volume_divide, timeStep , tmp_obj, st );
                    }
                    //                                size_t nmemb = tmp_obj->sizes().size();
                    originalGlyph->clear();
                    originalGlyph = tmp_obj;

                    for ( int tf = 0; tf < cnt/2; tf++ )
                    {
                        //changed by shimomura 2023/07/24
                        tmp_max[2*tf+1] = vismodule::Math::Max(tmp_max[2*tf+1],tmp_obj->colorMax());
                        tmp_min[2*tf+1] = vismodule::Math::Min(tmp_min[2*tf+1],tmp_obj->colorMin());
                        tmp_max[2*tf  ] = vismodule::Math::Max(tmp_max[2*tf  ],tmp_obj->sizeMax());
                        tmp_min[2*tf  ] = vismodule::Math::Min(tmp_min[2*tf  ],tmp_obj->sizeMin());
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

            }
#if 1
#ifndef CPU_VER          // MPI並列については一旦保留, collectorの内容がわかるまで
            if (mpi_size > 1) {
                //jc.jobCollect( originalObject, &vr, &nan_error, &wid );
                jc.jobCollect_glyph( originalGlyph, &nan_error, &wid );
            }
#endif
#endif
            vismodule::KVSMLObjectGlyph* object = originalGlyph;
            printf(" %zu glyphs generated\n", object->coords().size() / 3);

            //                           //add by shimomura 2023/06/14
            if ( originalGlyph != object ) delete originalGlyph;

            servMes.m_number_glyph = originalGlyph->coords().size() / 3;
            if ( servMes.m_number_glyph > 0 )
            {
                servMes.m_glyph_coords = std::make_unique<float[]>(3 * servMes.m_number_glyph);
                servMes.m_glyph_vectors = std::make_unique<float[]>(3 * servMes.m_number_glyph);
                servMes.m_glyph_colors = std::make_unique<unsigned char[]>(3 * servMes.m_number_glyph);
                servMes.m_glyph_sizes = std::make_unique<float[]>(servMes.m_number_glyph);
            }
            else
            {
                servMes.m_glyph_coords  = NULL;
                servMes.m_glyph_vectors = NULL;
                servMes.m_glyph_colors  = NULL;
                servMes.m_glyph_sizes   = NULL;
            }
            for ( int i = 0; i < servMes.m_number_glyph; ++i )
            {
                servMes.m_glyph_coords[3 * i + 0]  = object->coords()[3 * i + 0];
                servMes.m_glyph_coords[3 * i + 1]  = object->coords()[3 * i + 1];
                servMes.m_glyph_coords[3 * i + 2]  = object->coords()[3 * i + 2];
                servMes.m_glyph_vectors[3 * i + 0] = object->directions()[3 * i + 0];
                servMes.m_glyph_vectors[3 * i + 1] = object->directions()[3 * i + 1];
                servMes.m_glyph_vectors[3 * i + 2] = object->directions()[3 * i + 2];
                servMes.m_glyph_colors[3 * i + 0]  = object->colors()[3 * i + 0];
                servMes.m_glyph_colors[3 * i + 1]  = object->colors()[3 * i + 1];
                servMes.m_glyph_colors[3 * i + 2]  = object->colors()[3 * i + 2];
                servMes.m_glyph_sizes[i ] = object->sizes()[ i ];
            }

            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_END( 471 );
            }
            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_STA( 472 );
            }
            servMes.m_flag_send_bins = 2;
            servMes.m_message_size = servMes.byteSize();
            servMes.show();
            pts.sendMessage( servMes );
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

        if (mpi_size > 1) 
        {
            MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
        }
#endif

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

        servMes.m_glyph_color_min = tmp_min[1];
        servMes.m_glyph_color_max = tmp_max[1];
        servMes.m_glyph_size_min = tmp_min[0];
        servMes.m_glyph_size_max = tmp_max[0];
        std::cout << "m_glyph_min   = " << servMes.m_glyph_color_min << std::endl;
        std::cout << "m_glyph_max   = " << servMes.m_glyph_color_max << std::endl;
        servMes.m_flag_send_bins = 1;
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
        delete[] tmp_max;
        delete[] tmp_min;
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
#endif
}

//void generate_glytph_worker() 
void generate_glyph_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error,
#ifndef CPU_VER
                         JobCollector& jc,
#endif
                         JobDispatcher& jd,  
                         bool& useAllNodes, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param)
{

    int st, vl, wid = 0;
                    //std::vector<GlyphObjectCreator> glyph_creator_lst;
                    std::vector<vismodule::GlyphSeedGenerator> glyph_creator_lst;

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
                    glyph_creator_lst.clear();
                    for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                    {
                        //GlyphObjectCreator glyph_creator;
                        vismodule::GlyphSeedGenerator glyph_creator;
                        glyph_creator.setFilterInfo( &mvpl.m_list[idx] );

//                        point_creator.setFilterInfo( mvpl.m_list[idx] );
//                        glyph_creator.setCoordSynthStr( clntMes.m_x_synthesis,
//                                                        clntMes.m_y_synthesis, clntMes.m_z_synthesis );
//                        point_creator.setCoordSynthTkn( clntMes.x_synthesis_token,
//                                                        clntMes.y_synthesis_token, clntMes.z_synthesis_token );
                        glyph_creator_lst.push_back( glyph_creator );
                    }

                    transfunc_creator.setFilterInfo( mvpl.m_list[0] );
                    Calculate_minmax_glyph( param, mvpl, clntMes);
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
                    //param.m_sampling_step = 1;
                    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );
                    param.m_particle_limit_pre = param.m_particle_limit;
                    
                    float*  tmp_max;
                    float*  tmp_min;
                    int cnt = 2 ;
                    tmp_max = new float[cnt]; 
                    tmp_min = new float[cnt]; 

                    for ( int tf = 0; tf < cnt; tf++ )
                    {
                        tmp_max[tf] = FLT_MIN;
                        tmp_min[tf] = FLT_MAX;
                    }
 
                    while ( jd.dispatchNext( wid, &st, &vl ) )
                    {
                        int xvl, fidx;
                        fidx = mvpl.getFileIndex( vl, &xvl );
                        MultiVolumeProperty&mvp = mvpl.m_list[fidx];

                        mvp.setFilePath(param.m_input_data, st, xvl);
                        vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;
                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 

                            }
#ifdef EXTEND_FILE_FORMAT
                            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                            {
                                glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep , tmp_obj, st, xvl );
                            }                                
#endif
                            else     // filetype: kvsml
                            {
                                glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, tmp_obj, st );
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
//                        VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
//                        jc.jobCollect( tmp_obj, p_vr, &nan_error, &wid );
                        jc.jobCollect_glyph( tmp_obj, &nan_error, &wid );
#endif
                        if ( nan_error )
                        {
                            nan_error = false;
                            continue;
                        }
                        for ( int tf = 0; tf < cnt/2; tf++ )
                        {
                            //add by shimomura 2023/06/14
                            tmp_max[2*tf+1] = vismodule::Math::Max( tmp_max[2*tf+1] ,tmp_obj->colorMax());
                            tmp_min[2*tf+1] = vismodule::Math::Min( tmp_min[2*tf+1] ,tmp_obj->colorMin());
                            tmp_max[2*tf]   = vismodule::Math::Max( tmp_max[2*tf]   ,tmp_obj->sizeMax());
                            tmp_min[2*tf]   = vismodule::Math::Min( tmp_min[2*tf]   ,tmp_obj->sizeMin());
                        }



                    } // end of while(DispatchNext)
#ifndef CPU_VER

                    MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
                    MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
#endif
                    //add by shimomura 20250213
                    delete[] tmp_max;
                    delete[] tmp_min;

                    if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 1 );
                        VIS_MODULE_TIMER_FIN();
                    }
                    delete param.m_transfunc_synthesizer;
                }
               
}

void generate_glyph_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, ParticleMonitor& pm, vismodule::Timer& timer,
                         std::string particlePath, std::string glyphParameterPath, std::string glyphParameterPath_old,
                         bool& useAllNodes, int& timer_count , const jpv::InitializeParameter init_param )
{
	int mpi_size = 1;
    
    char* buf;
    int bsz = 0;
    int st, vl, wid = 0;


                        timer_count++;
                    if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_STA( 461 );
                    }

                    // send cltMes to all worker process >>
//                    bsz = clntMes.byteSize();
//#ifndef CPU_VER
//                    MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
//#endif
//                    buf = new char[bsz];
//                    clntMes.pack( buf );
//#ifndef CPU_VER
//                    MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
//#endif
//                    delete[] buf;
                    // send cltMes to all worker process <<

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
                        param.m_component_Id = clntMes.m_rendering_id;
                        param.m_crop.setEnable( clntMes.m_enable_crop_region );
                        param.m_crop.set( clntMes.m_crop_region );
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

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
                       
                        pm.findGlyphFile();
                        mvpl.m_total_number_subvolumes=1;
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

                        VariableRange vr;

                        pm.check();
                        servMes.m_start_step = pm.particleStatusFile().getStartTimeStep();
                        servMes.m_last_step = pm.particleStatusFile().getLatestTimeStep();
                        if( pm.stepExisted() )
                        {
                            //if( servMes.m_start_step <= clntMes.m_step && clntMes.m_step <= servMes.m_last_step && pm.getTimeStep() > -1 )
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
                            ppw.inputGlyphParameterMessage( clntMes );
                            ppr.readParameterFile( glyphParameterPath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();

                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( glyphParameterPath.c_str() );
                                // 20181226 end
                            }
                            vismodule::PointObject* originalObject = new vismodule::PointObject;
                            vismodule::KVSMLObjectGlyph* originalGlyph = new vismodule::KVSMLObjectGlyph;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
//                            std::string filename( glyphFilePath );
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
                            if( pm.setTimeStep_glyph( clntMes.m_step ) || pm.stepExisted() ) servMes.m_flag_send_bins = 2;
                            else                                 servMes.m_flag_send_bins = 1;
                            if( servMes.m_flag_send_bins == 2)
                            {
                                pm.readGlyphFile();
                                pm.getGlyph( originalGlyph );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            //servMes.m_subpixel_level = pm.getSubpixelLevel();
//                            servMes.m_subpixel_level = 1;
                            
                            servMes.m_number_glyph = originalGlyph->coords().size() / 3;
                            if ( servMes.m_number_glyph > 0 )
                            {
                                servMes.m_glyph_coords = std::make_unique<float[]>(3 * servMes.m_number_glyph);
                                servMes.m_glyph_vectors = std::make_unique<float[]>(3 * servMes.m_number_glyph);
                                servMes.m_glyph_colors = std::make_unique<unsigned char[]>(3 * servMes.m_number_glyph);
                                servMes.m_glyph_sizes = std::make_unique<float[]>(servMes.m_number_glyph);
                            }
                            else
                            {
                                servMes.m_glyph_coords  = NULL;
                                servMes.m_glyph_vectors = NULL;
                                servMes.m_glyph_colors  = NULL;
                                servMes.m_glyph_sizes   = NULL;
                            }
                            for ( int i = 0; i < servMes.m_number_glyph; ++i )
                            {
                                servMes.m_glyph_coords[3 * i + 0] = originalGlyph->coords()[3 * i + 0];
                                servMes.m_glyph_coords[3 * i + 1] = originalGlyph->coords()[3 * i + 1];
                                servMes.m_glyph_coords[3 * i + 2] = originalGlyph->coords()[3 * i + 2];
                                servMes.m_glyph_vectors[3 * i + 0] = originalGlyph->directions()[3 * i + 0];
                                servMes.m_glyph_vectors[3 * i + 1] = originalGlyph->directions()[3 * i + 1];
                                servMes.m_glyph_vectors[3 * i + 2] = originalGlyph->directions()[3 * i + 2];
                                servMes.m_glyph_colors[3 * i + 0] = originalGlyph->colors()[3 * i + 0];
                                servMes.m_glyph_colors[3 * i + 1] = originalGlyph->colors()[3 * i + 1];
                                servMes.m_glyph_colors[3 * i + 2] = originalGlyph->colors()[3 * i + 2];
                                servMes.m_glyph_sizes[i ] = originalGlyph->sizes()[ i ];
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

#include <vismodule/GenerateGlyph>

void generate_glyph(
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
    const int size_index  = 0; // for minmax array index
    const int color_index = 1; // for minmax array index

    if( rank == 0 )
    {
        SetServerMessageParameter( param, mvpl, servMes );

        // send sub volume num server message
        std::cout << "INFO: send sub volume num server message" << std::endl;
        servMes.m_message_size = servMes.byteSize();
        servMes.show();
        pts.sendMessage( servMes );     
    }

    int cnt = 2;
    float* tmp_max;
    float* tmp_min;
    tmp_max = new float[cnt]; 
    tmp_min = new float[cnt];

    for ( int tf = 0; tf < cnt; tf++ )
    {
        tmp_max[tf] = FLT_MIN;
        tmp_min[tf] = FLT_MAX;
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
            vismodule::KVSMLObjectGlyph* originalGlyph = new vismodule::KVSMLObjectGlyph;
            vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;

            // make glyph
            // make size and color minmax
            if ( ( rank > 0 ) || ( mpi_size == 1 ) )
            {
                int xvl, fidx;
                fidx = mvpl.getFileIndex( vl, &xvl );
                MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                mvp.setFilePath(param.m_input_data, st, xvl);
                param.m_subvolume_id = xvl;

                // generate glyph start
                try
                {
                    vismodule::VolumeObjectBase* volume = nullptr;
                    vismodule::GlyphSeedGenerator glyph_creator;

                    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                        generate_volume( param, mvp, st, xvl, volume );
                    }
#ifdef EXTEND_FILE_FORMAT
                    else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                    {
                        generate_volume( param, mvp, st, xvl, volume );
                    }                                
#endif
                    else     // filetype: kvsml
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
                    int number_of_divide = mvpl.m_total_number_subvolumes;

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
                        glyph_creator.GenerateGlyphUnstruct(
                            param,
                            number_of_divide,
                            raw_pointers_vector.data(),
                            nvariables,
                            coordinates.get(),
                            ncoords,
                            connections.get(),
                            ncells,
                            celltype,
                            server_mode,
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
                        glyph_creator.GenerateGlyphStruct(
                            param,
                            number_of_divide,
                            dom,
                            raw_pointers_vector.data(),
                            nvariables,
                            server_mode,
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
                // generate glyph end

                // get minmax
                tmp_max[size_index]  = vismodule::Math::Max(  tmp_max[size_index], tmp_obj->sizeMax()  );
                tmp_min[size_index]  = vismodule::Math::Min(  tmp_min[size_index], tmp_obj->sizeMin()  );
                tmp_max[color_index] = vismodule::Math::Max( tmp_max[color_index], tmp_obj->colorMax() );
                tmp_min[color_index] = vismodule::Math::Min( tmp_min[color_index], tmp_obj->colorMin() );
            }
            // make glyph
            // make size and color minmax

#ifndef CPU_VER
            if ( mpi_size > 1 ) {
                if ( rank == 0 )
                {
                    jc.jobCollect_glyph( originalGlyph, &nan_error, &wid );
                }
                else
                {
                    jc.jobCollect_glyph( tmp_obj, &nan_error, &wid );
                }
            }
#else
            originalGlyph->clear();
            originalGlyph->setCoords( tmp_obj->coords() );
            originalGlyph->setColors( tmp_obj->colors() );
            originalGlyph->setDirections( tmp_obj->directions() );
            originalGlyph->setSizes( tmp_obj->sizes() );
            originalGlyph->setColorMin( tmp_obj->colorMin() );
            originalGlyph->setColorMax( tmp_obj->colorMax() );
            originalGlyph->setSizeMin( tmp_obj->sizeMin() );
            originalGlyph->setSizeMax( tmp_obj->sizeMax() );
            printf(" %zu glyphs generated\n", tmp_obj->coords().size() / 3);
#endif
            delete tmp_obj;

            // send glyph
            if ( rank == 0 )
            {
                SendGlyphServerMessage( originalGlyph, pts, servMes );
            } // send glyph

            delete originalGlyph;
        } // end of while(DispatchNext)

#ifndef CPU_VER
        if ( mpi_size > 1 )
        {
            MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
        }
#endif
    } // server_mode == jpv::ServerMode::CS  
    else // server_mode == jpv::ServerMode::IS
    {
        vismodule::KVSMLObjectGlyph* originalGlyph = new vismodule::KVSMLObjectGlyph;

        ParticleMonitor pm;
        pm.check();

        if( pm.stepExisted() )
        {
            pm.setTimeStep_glyph( pm.particleStatusFile().getLatestTimeStep() );
        }
        else
        {
            pm.setTimeStep_glyph(0);
        }

        // get glyph
        pm.readGlyphFile();
        pm.getGlyph( originalGlyph );

        // get minmax
        tmp_max[size_index]  = vismodule::Math::Max(  tmp_max[size_index], originalGlyph->sizeMax()  );
        tmp_min[size_index]  = vismodule::Math::Min(  tmp_min[size_index], originalGlyph->sizeMin()  );
        tmp_max[color_index] = vismodule::Math::Max( tmp_max[color_index], originalGlyph->colorMax() );
        tmp_min[color_index] = vismodule::Math::Min( tmp_min[color_index], originalGlyph->colorMin() );

        SendGlyphServerMessage( originalGlyph, pts, servMes );

        delete originalGlyph;
    } // server_mode == jpv::ServerMode::IS  

    // send size and color minmax
    if ( rank == 0 )
    {
        servMes.m_flag_send_bins = 1;
        servMes.m_number_particle = 0;
        servMes.m_number_glyph = 0;

        // make minmax
        servMes.m_glyph_size_min  = tmp_min[size_index];
        servMes.m_glyph_size_max  = tmp_max[size_index];
        servMes.m_glyph_color_min = tmp_min[color_index];
        servMes.m_glyph_color_max = tmp_max[color_index];

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

        // send size and color minmax server message
        std::cout << "INFO: send sieze and color minmax server message" << std::endl;
        servMes.m_message_size = servMes.byteSize();
        servMes.show();
        pts.sendMessage( servMes );

        // TEST START 2015.1.14
        servMes.m_server_status = 0;
        // TEST END 2015.1.14
    } // send size and color minmax

    nan_error = false;

    delete[] tmp_min;
    delete[] tmp_max;
}

void SendGlyphServerMessage(
    const vismodule::KVSMLObjectGlyph* originalGlyph,
    jpv::ParticleTransferServer pts,
    jpv::ParticleTransferServerMessage& servMes    
)
{
    // make glyph server message start
    servMes.m_flag_send_bins = 2;
    // vismodule::KVSMLObjectGlyph* object = originalGlyph;
    // if ( originalGlyph != object ) delete originalGlyph;
    servMes.m_number_glyph = originalGlyph->coords().size() / 3;
    printf(" %zu glyphs generated\n", originalGlyph->coords().size() / 3);

    if ( servMes.m_number_glyph > 0 )
    {
        servMes.m_glyph_coords  = std::make_unique<float[]>(3 * servMes.m_number_glyph);
        servMes.m_glyph_vectors = std::make_unique<float[]>(3 * servMes.m_number_glyph);
        servMes.m_glyph_colors  = std::make_unique<unsigned char[]>(3 * servMes.m_number_glyph);
        servMes.m_glyph_sizes   = std::make_unique<float[]>(servMes.m_number_glyph);
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
        servMes.m_glyph_coords[3 * i + 0]  = originalGlyph->coords()[3 * i + 0];
        servMes.m_glyph_coords[3 * i + 1]  = originalGlyph->coords()[3 * i + 1];
        servMes.m_glyph_coords[3 * i + 2]  = originalGlyph->coords()[3 * i + 2];
        servMes.m_glyph_vectors[3 * i + 0] = originalGlyph->directions()[3 * i + 0];
        servMes.m_glyph_vectors[3 * i + 1] = originalGlyph->directions()[3 * i + 1];
        servMes.m_glyph_vectors[3 * i + 2] = originalGlyph->directions()[3 * i + 2];
        servMes.m_glyph_colors[3 * i + 0]  = originalGlyph->colors()[3 * i + 0];
        servMes.m_glyph_colors[3 * i + 1]  = originalGlyph->colors()[3 * i + 1];
        servMes.m_glyph_colors[3 * i + 2]  = originalGlyph->colors()[3 * i + 2];
        servMes.m_glyph_sizes[i]           = originalGlyph->sizes()[i];
    }
    // make glyph server message end

    // send glyph server message
    std::cout << "INFO: send glyph server message" << std::endl;
    servMes.m_message_size = servMes.byteSize();
    servMes.show();
    pts.sendMessage( servMes );
}

#if 0
void generate_glyph_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error,
#ifndef CPU_VER
                         JobCollector& jc,
#endif
                         JobDispatcher& jd,  
                         TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param)
{
    int st, vl, wid = 0;
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
        param.m_sampling_method = clntMes.m_sampling_method;
        param.m_input_data_base = clntMes.m_input_directory;
        param.m_particle_limit = clntMes.m_particle_limit;
        param.m_particle_density = clntMes.m_particle_density;

        mvpl.searchFile(param);
        glyph_creator_lst.clear();
        for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
        {
            vismodule::GlyphSeedGenerator glyph_creator;
            glyph_creator.setFilterInfo( &mvpl.m_list[idx] );

            glyph_creator_lst.push_back( glyph_creator );
        }

        transfunc_creator.setFilterInfo( mvpl.m_list[0] );
        Calculate_minmax_glyph( param, mvpl, clntMes);
        transfunc_creator.setProtocol( clntMes );
        transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
        param.m_transfunc_synthesizer = transfunc_creator.create();
        param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
        for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
        {
            param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
        }

        jd.initialize( 
            clntMes.m_step,
            clntMes.m_step,
            mvpl.m_total_number_subvolumes,
            mvpl.m_total_min_subvolume_coord,
            mvpl.m_total_max_subvolume_coord,
            param.m_latency_threshold,
            param.m_job_id_pack_size );

        param.m_sampling_step = CalculateSamplingStep( mvpl );
        param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );
                    
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
#ifdef _DEBUG // debug by @hira
                printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                std::cerr << e.what();
                nan_error = true;
            }
#ifndef CPU_VER
            // VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
            // jc.jobCollect( tmp_obj, p_vr, &nan_error, &wid );
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
                         int& timer_count , const jpv::InitializeParameter init_param )
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
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        param.m_sampling_method = clntMes.m_sampling_method;
                        param.m_particle_limit = clntMes.m_particle_limit;
                        param.m_particle_density = clntMes.m_particle_density;

                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                       
                        pm.findGlyphFile();
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

std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;
                        pts.sendMessage( servMes );

                        timer.start();
               
                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;
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

std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;

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

std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;

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

std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;
                            std::cout << "clntMes.m_step:" << clntMes.m_step << std::endl;
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
std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;                            
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

std::cout << __FILE__ << ", " << __func__ << ", " << __LINE__ << std::endl;                            
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
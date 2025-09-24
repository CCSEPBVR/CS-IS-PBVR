#include <vismodule/GenerateParticle>

void generate_particle_master(
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
    pts.sendMessage( servMes );
    
    point_generator_lst.clear();
    point_generator_lst.resize(mvpl.m_list.size());

    jd.initialize(
        param.m_time_step,
        param.m_time_step,
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
    servMes.initializeTransferFunction( transfunc_creator.transfunc().size(), DEFAULT_NBINS );
    int tf_count = transfunc_creator.transfunc().size();

    int c_bins_size = 0;
    int o_bins_size = 0;
    for ( int tf = 0; tf < tf_count; tf++ )
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
    tmp_max = new float[tf_count * 2]; 
    tmp_min = new float[tf_count * 2];

    for ( int tf = 0; tf < (tf_count * 2); tf++ )
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

        if (mpi_size == 1) {
            int xvl, fidx;
            vismodule::PointObject* tmp_obj = NULL;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( param.m_input_data, st, xvl );
            point_generator_lst[fidx].setFilterInfo( &mvp );
            param.m_subvolume_id = xvl;
            int timeStep = 1;
            servMes.m_flag_send_bins = 0;
            try
            {
                point_generator_lst[fidx].setCoordSynthStr( param.m_x_synthesis, param.m_y_synthesis, param.m_z_synthesis );
                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    tmp_obj = point_generator_lst[fidx].run( param, *servMes.m_camera, timeStep, st, xvl );
                }
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    tmp_obj = point_generator_lst[fidx].run( param, *servMes.m_camera, timeStep, st, xvl );
                }
                else // filetype: kvsml
                {
                    tmp_obj = point_generator_lst[fidx].run( param, *servMes.m_camera, timeStep, st );
                }

                size_t nmemb = tmp_obj->nvertices() * 3;
                vismodule::ValueArray<vismodule::Real32> coords_array ( tmp_obj->coords().pointer(), nmemb );
                vismodule::ValueArray<vismodule::UInt8>  colors_array ( tmp_obj->colors().pointer(), nmemb );
                vismodule::ValueArray<vismodule::Real32> normals_array( tmp_obj->normals().pointer(), nmemb );
                
                originalObject->clear();
                originalObject->setCoords( coords_array );
                originalObject->setColors( colors_array );
                originalObject->setNormals( normals_array );

                // modify by @hira at 2016/12/01  
                int c_count = 0;
                for ( int tf = 0; tf < tf_count; tf++ )
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
                for ( int tf = 0; tf < tf_count; tf++ )
                {
                    int o_nbins = tmp_obj->getNbins();
                    //changed by shimomura 2023/07/24
                    tmp_max[2 * tf] = vismodule::Math::Max( tmp_max[2 * tf], param.m_transfunc_synthesizer->m_o_max[tf] );
                    tmp_min[2 * tf] = vismodule::Math::Min( tmp_min[2 * tf], param.m_transfunc_synthesizer->m_o_min[tf] );
                    for ( int res = 0; res < o_nbins; res++ )
                    {
                        tmp_o_bins[ o_count ] += tmp_obj->getOHistogram()[ o_count ];
                        o_count++;
                    }
                }
            }
            catch ( const std::runtime_error& e )
            {
#ifdef _DEBUG	// debug by @hira
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
        servMes.m_number_particle = object->coords().size() / 3;
        printf(" %zu perticles generated\n", servMes.m_number_particle);

        if ( servMes.m_number_particle > 0 )
        {
            servMes.m_positions = std::make_unique<float[]>(3 * servMes.m_number_particle);
            servMes.m_normals = std::make_unique<float[]>(3 * servMes.m_number_particle);
            servMes.m_colors = std::make_unique<unsigned char[]>(3 * servMes.m_number_particle);
        }
        else
        {
            servMes.m_positions = NULL;
            servMes.m_normals   = NULL;
            servMes.m_colors    = NULL;
        }
        for ( int i = 0; i < servMes.m_number_particle; ++i )
        {
            servMes.m_positions[3 * i + 0] = object->coords()[3 * i + 0];
            servMes.m_positions[3 * i + 1] = object->coords()[3 * i + 1];
            servMes.m_positions[3 * i + 2] = object->coords()[3 * i + 2];
            servMes.m_normals[3 * i + 0] = object->normals()[3 * i + 0];
            servMes.m_normals[3 * i + 1] = object->normals()[3 * i + 1];
            servMes.m_normals[3 * i + 2] = object->normals()[3 * i + 2];
            servMes.m_colors[3 * i + 0] = object->colors()[3 * i + 0];
            servMes.m_colors[3 * i + 1] = object->colors()[3 * i + 1];
            servMes.m_colors[3 * i + 2] = object->colors()[3 * i + 2];
        }

        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_END( 471 );
        }

        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_STA( 472 );
        }
        servMes.m_flag_send_bins = 0;
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
        // delete[] servMes.m_positions;
        // delete[] servMes.m_normals;
        // delete[] servMes.m_colors;
        delete object;
        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_END( 473 );
        }
    } // end of while(DispatchNext)
//CS only end

#ifndef CPU_VER
    if (mpi_size > 1) {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, (tf_count * 2), MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, (tf_count * 2), MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
    }
#endif

    //add by shimomura 2023/06/14
    vr = setVariablerange2( tmp_max, tmp_min, tf_count );
    servMes.m_server_side_variable_range = vr;
    // add by shimomura 2022/12/16
    servMes.setColorHistogramBins(
        param.m_transfunc_array.size(),
        DEFAULT_NBINS,
        tmp_c_bins
    );

    servMes.setOpacityHistogramBins(
        param.m_transfunc_array.size(),
        DEFAULT_NBINS,
        tmp_o_bins
    ); // change by shimomura 2022/12/26

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
    pts.sendMessage( servMes );
    // TEST START 2015.1.14
    servMes.m_server_status = 0;
    // TEST END 2015.1.14

    for ( int tf = 0; tf < tf_count; tf++ )
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

void generate_particle_worker(
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
    int st, vl, wid = 0;
    std::vector<vismodule::CS_PointObjectGenerator> point_generator_lst;
    VariableRange vr;

    point_generator_lst.clear();
    point_generator_lst.resize(mvpl.m_list.size());
                    
    jd.initialize(
        param.m_time_step,
        param.m_time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        param.m_latency_threshold,
        param.m_job_id_pack_size
    );

    param.m_sampling_step = CalculateSamplingStep( mvpl );
    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *param.m_camera );

    int tf_count = transfunc_creator.transfunc().size();

    int c_bins_size = 0;
    int o_bins_size = 0;
    int c_nbins = DEFAULT_NBINS;
    int o_nbins = DEFAULT_NBINS;
    for ( int tf = 0; tf < tf_count; tf++ )
    {
        c_bins_size += c_nbins;
        o_bins_size += o_nbins;
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];

    //add by shimomura 2023/06/14
    float*  tmp_max;
    float*  tmp_min;
    tmp_max = new float[tf_count * 2]; 
    tmp_min = new float[tf_count * 2]; 

    for ( int tf = 0; tf < (tf_count * 2); tf++ )
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
        mvp.setFilePath( param.m_input_data, st, xvl );
        point_generator_lst[fidx].setFilterInfo( &mvp );
        param.m_subvolume_id = xvl;
        int timeStep = 1;

        try
        {
            point_generator_lst[fidx].setCoordSynthStr( param.m_x_synthesis, param.m_y_synthesis, param.m_z_synthesis );
            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, timeStep, st, xvl );

            }
#ifdef EXTEND_FILE_FORMAT
            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, timeStep, st, xvl );
            }                            
#endif
            else // filetype: kvsml
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
        for ( int tf = 0; tf < tf_count; tf++ )
        {
            c_nbins = tmp_obj->getNbins();
            //add by shimomura 2023/06/14
            tmp_max[2*tf+1] = vismodule::Math::Max( tmp_max[2*tf+1] ,param.m_transfunc_synthesizer->m_c_max[tf]);
            tmp_min[2*tf+1] = vismodule::Math::Min( tmp_min[2*tf+1] ,param.m_transfunc_synthesizer->m_c_min[tf]);
            for ( int res = 0; res < c_nbins; res++ )
            {
                tmp_c_bins[c_count] += tmp_obj->getCHistogram()[c_count] ;
                c_count++;
            }
        }

        int o_count = 0;
        for ( int tf = 0; tf < tf_count; tf++ )
        {
            o_nbins = tmp_obj->getNbins();
            //add by shimomura 2023/06/14
            tmp_max[2*tf] = vismodule::Math::Max( tmp_max[2*tf], param.m_transfunc_synthesizer->m_o_max[tf] );
            tmp_min[2*tf] = vismodule::Math::Min( tmp_min[2*tf], param.m_transfunc_synthesizer->m_o_min[tf] );
            for ( int res = 0; res < o_nbins; res++ )
            {
                tmp_o_bins[o_count] += tmp_obj->getOHistogram()[o_count];
                o_count++;
            }
        }
    } // end of while(DispatchNext)

#ifndef CPU_VER
    MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_max, (tf_count * 2), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_min, (tf_count * 2), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
#endif

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    //add by shimomura 20240603
    delete[] tmp_max;
    delete[] tmp_min;
    delete param.m_transfunc_synthesizer;
}

void generate_particle_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, ParticleMonitor& pm, vismodule::Timer& timer,
                         std::string particlePath, std::string tfFilePath, std::string tfFilePath_old,
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
                        servMes.m_number_glyph = 0;
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

                        pts.sendMessage( servMes );

                        timer.start();

                        servMes.m_transfer_function_count = clntMes.m_transfer_function.size();//TF_COUNT
                        servMes.m_color_nbins = new vismodule::UInt64[clntMes.m_transfer_function.size()];
                        servMes.m_opacity_nbins = new vismodule::UInt64[clntMes.m_transfer_function.size()];

                        servMes.m_color_bins.resize( clntMes.m_transfer_function.size() );
                        servMes.m_opacity_bins.resize( clntMes.m_transfer_function.size() );

                        int c_bins_size = 0;
                        int o_bins_size = 0;
                        vismodule::UInt64* tmp_c_bins;
                        vismodule::UInt64* tmp_o_bins;


                        c_bins_size = 0;
                        o_bins_size = 0;

                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            servMes.m_color_nbins[tf] = DEFAULT_NBINS;
                            servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
                            servMes.m_color_bins[tf] =  new vismodule::UInt64[ servMes.m_color_nbins[tf] ];
                            servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ servMes.m_opacity_nbins[tf] ];
                            c_bins_size += servMes.m_color_nbins[tf];
                            o_bins_size += servMes.m_opacity_nbins[tf];
                            for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                            {
                                servMes.m_color_bins[tf][res] = 0;
                            }
                            for ( int res = 0; res < servMes.m_opacity_nbins[tf]; res++ )
                            {
                                servMes.m_opacity_bins[tf][res] = 0;
                            }
                        }

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
                            ppw.inputMessage( clntMes );
                            //ppr.readParameterFile("jupiter_old.tf");
                            ppr.readParameterFile( tfFilePath_old.c_str() );
                            NameListFile nm1 = ppr.getNameListFile();
                            NameListFile nm2 = ppw.getNameListFile();
                            if( nm1 != nm2 )
                            {
                                ppw.writeParameterFile( tfFilePath.c_str() );
                            }
                            // 20181226 end

                            vismodule::PointObject* originalObject = new vismodule::PointObject;
                            TimerStart( 2 );
                            // 20181226 start　環境変数で指定したパスを使用
                            //std::string filename( jupiter_prefix );
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
                            if( pm.setTimeStep_particle( clntMes.m_step ) ) servMes.m_flag_send_bins = 0;
                            else                                 servMes.m_flag_send_bins = 1;
                            if( servMes.m_flag_send_bins == 0)
                            {
                                pm.readParticleHistoryFile();
                                pm.readParticleFile();
                                pm.getParticle( originalObject );
                            }
                            servMes.m_time_step = clntMes.m_step;
                            servMes.m_subpixel_level = pm.getSubpixelLevel();
                            vr = pm.particleHistoryFile().variableRange();
                           
                            //　粒子生成時に変数の数も取得するよう変更, paerticle_limit,particle_densityを取得するよう変更 2025/05/30
                            servMes.m_number_ingredients = pm.particleHistoryFile().nVariables();
                            servMes.m_particle_limit = pm.particleHistoryFile().ParticleLimit();
                            servMes.m_particle_density = pm.particleHistoryFile().ParticleDensity();

                            TimerStart( 3 );
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
                            TimerStop( 3 );
//jupiter end
                            servMes.m_number_particle = originalObject->coords().size() / 3;
                            if ( servMes.m_number_particle > 0 )
                            {
                                servMes.m_positions = std::make_unique<float[]>(3 * servMes.m_number_particle);
                                servMes.m_normals = std::make_unique<float[]>(3 * servMes.m_number_particle);
                                servMes.m_colors = std::make_unique<unsigned char[]>(3 * servMes.m_number_particle);
                            }
                            else
                            {
                                servMes.m_positions = NULL;
                                servMes.m_normals   = NULL;
                                servMes.m_colors    = NULL;
                            }
                            for ( int i = 0; i < servMes.m_number_particle; ++i )
                            {
                                servMes.m_positions[3 * i + 0] = originalObject->coords()[3 * i + 0];
                                servMes.m_positions[3 * i + 1] = originalObject->coords()[3 * i + 1];
                                servMes.m_positions[3 * i + 2] = originalObject->coords()[3 * i + 2];
                                servMes.m_normals[3 * i + 0] = originalObject->normals()[3 * i + 0];
                                servMes.m_normals[3 * i + 1] = originalObject->normals()[3 * i + 1];
                                servMes.m_normals[3 * i + 2] = originalObject->normals()[3 * i + 2];
                                servMes.m_colors[3 * i + 0] = originalObject->colors()[3 * i + 0];
                                servMes.m_colors[3 * i + 1] = originalObject->colors()[3 * i + 1];
                                servMes.m_colors[3 * i + 2] = originalObject->colors()[3 * i + 2];
                            }
                            servMes.m_server_side_variable_range = vr;
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

                            std::cout<<"main.cpp:L779"<<std::endl;

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
//jupiter
                            TimerStop( 1 );
//                          TimerFinish( servMes.m_time_step );
//jupiter
                        } // end of while(DispatchNext)
                        servMes.m_flag_send_bins = 1;
                        servMes.m_message_size = servMes.byteSize();
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0;
                        TimerStart( 11 );
                        pts.sendMessage( servMes );
                        TimerStop( 11 );

                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            delete[] servMes.m_color_bins[tf];
                            delete[] servMes.m_opacity_bins[tf];
                        }
                        delete[] servMes.m_color_nbins;
                        delete[] servMes.m_opacity_nbins;
                        servMes.m_transfer_function_count = 0;
                        servMes.m_flag_send_bins = 1;
                        delete[] tmp_c_bins;
                        delete[] tmp_o_bins;

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

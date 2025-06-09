#ifndef VIS_MODULE_GENERATE_STEP_H_INCLDE
#define VIS_MODULE_GENERATE_STEP_H_INCLDE

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/PointObjectGenerator>

#include <vismodule/Argument>

#include <vismodule/AVSField>
#include <vismodule/Timer>
#include <vismodule/KVSMLObjectPointWriter>
//#include "KVSMLObjectPointMPIWriter.h"
#include <vismodule/JobDispatcher>
#ifndef CPU_VER
#include <vismodule/JobCollector>
#endif
#include <vismodule/MultiVolumeProperty>
#include <vismodule/TransferFunctionProperty>
#ifndef CPU_VER
#include "mpi.h"
#endif

#include <cassert>
#include <signal.h> /* 140319 for client stop by Ctrl+c */
#include <sys/stat.h>
#if (defined(VIS_MODULE_PLATFORM_LINUX) || defined(VIS_MODULE_PLATFORM_MACOSX))
#include <execinfo.h>
#endif
#include <vismodule/File>

#include <vismodule/ExtendedTransferFunction>
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "VariableRange.h"

#include <vismodule/timer_simple>

#include <vismodule/Compiler>
#ifdef VIS_MODULE_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

#include <vismodule/FileChecker>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/CellByCellParticleGenerator>

#include <vismodule/GlyphObjectGenerator>
#include <vismodule/GlyphObjectCreator>

//plot over line
#include <vismodule/POLObjectGenerator>

#include <vismodule/Calculate>
#include <vismodule/PointObjectCreator>
#include <vismodule/SignalHandler>
#include <vismodule/Math>


void generate_particle_master(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
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
    std::vector<GlyphObjectCreator> glyph_creator_lst;

    timer_count++;
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
        servMes.m_number_glyph = 0 ;
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

        if(init_param == jpv::InitializeParameter::generate_particle )
        {
            transfunc_creator.setProtocol( clntMes );
            transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
            param.m_transfunc_synthesizer = transfunc_creator.create();
        }
        else if (init_param == jpv::InitializeParameter::generate_glyph)
        {
            Calculate_minmax_glyph( param, mvpl, clntMes );
        }

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
//        if ( param.m_gt5d == true || param.m_gt5d_full == true )
//        {
//            int timeStep = servMes.m_time_step;
//
//            if ( servMes.m_time_step > 1 )
//            {
//                for ( int nf = 0; nf < point_creator_lst.size(); nf++ )
//                    point_creator_lst[nf].progressValues();
//            }
//        }

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
        if(init_param == jpv::InitializeParameter::generate_particle )
        {
        // 関数の領域確保、初期化を行う : by @hira 2016/12/01
        servMes.initializeTransferFunction(clntMes.m_transfer_function.size(), DEFAULT_NBINS);

        int c_bins_size = 0;
        int o_bins_size = 0;
        vismodule::UInt64* tmp_c_bins;
        vismodule::UInt64* tmp_o_bins;
        c_bins_size = 0;
        o_bins_size = 0;
        
        if(init_param == jpv::InitializeParameter::generate_particle )
        {
            for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
            {
                c_bins_size += servMes.m_color_nbins[tf];
                o_bins_size += servMes.m_opacity_nbins[tf];
            }
        }

        tmp_c_bins = new vismodule::UInt64[c_bins_size];
        tmp_o_bins = new vismodule::UInt64[o_bins_size];

        //add by shimomura 2023/06/14
        int cnt = 2; 
        
        if(init_param == jpv::InitializeParameter::generate_particle )
        {
            cnt = 2* servMes.m_transfer_function_count ;
        }
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

        while ( jd.dispatchNext( wid, &st, &vl ) )
        {
            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_STA( 471 );
            }

            vismodule::PointObject* originalObject = new vismodule::PointObject;

            if (mpi_size == 1) {
                int xvl, fidx;
                fidx = mvpl.getFileIndex( vl, &xvl );
                MultiVolumeProperty& mvp = mvpl.m_list[fidx];

                mvp.setFilePath(param.m_input_data, st, xvl);
                vismodule::PointObject* tmp_obj = NULL;
                point_creator_lst[fidx].setFilterInfo( mvp );
                param.m_subvolume_id = xvl ;
                int timeStep = 1;
                servMes.m_flag_send_bins = 0;
                try
                {
                    point_creator_lst[fidx].setCoordSynthStr( clntMes.m_x_synthesis,
                            clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                    //                                point_creator_lst[fidx].setCoordSynthTkn( clntMes.x_synthesis_token,
                    //                                                                          clntMes.y_synthesis_token, clntMes.z_synthesis_token );
                    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                        tmp_obj = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                    }
                    else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                    {
                        tmp_obj = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                    }
                    else     // filetype: kvsml
                    {
                        tmp_obj = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st );
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
                    for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
                    {
                        int c_nbins = tmp_obj->getNbins();
                        //changed by shimomura 2023/07/24
                        //                                    tmp_max[2*tf+1] = param.m_transfunc_synthesizer-> m_c_max[tf];
                        //                                    tmp_min[2*tf+1] = param.m_transfunc_synthesizer-> m_c_min[tf];
                        tmp_max[2*tf+1] = vismodule::Math::Max( tmp_max[2*tf+1] ,param.m_transfunc_synthesizer-> m_c_max[tf]);
                        tmp_min[2*tf+1] = vismodule::Math::Min( tmp_min[2*tf+1] ,param.m_transfunc_synthesizer-> m_c_min[tf]);
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
                        tmp_max[2*tf] = vismodule::Math::Max( tmp_max[2*tf] ,param.m_transfunc_synthesizer-> m_o_max[tf]);
                        tmp_min[2*tf] = vismodule::Math::Min( tmp_min[2*tf] ,param.m_transfunc_synthesizer-> m_o_min[tf]);
                        for ( int res = 0; res < o_nbins; res++ )
                        {
                            tmp_o_bins[o_count] += tmp_obj->getOHistogram()[ o_count ] ;
                            o_count++;
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
            servMes.m_server_side_variable_range = vr;

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
            std::cout << __FUNCTION__ << __LINE__ << std::endl;                            
            pts.sendMessage( servMes );
            std::cout << __FUNCTION__ << __LINE__ << std::endl;                            
            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_END( 472 );
            }
            if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
            {
                VIS_MODULE_TIMER_STA( 473 );
            }
            //                            delete[] servMes.m_positions;
            //                            delete[] servMes.m_normals;
            //                            delete[] servMes.m_colors;
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
            strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
            servMes.m_server_status = 1;
            servMes.m_number_particle = 0;
            servMes.m_number_glyph = 0 ;
            servMes.m_flag_send_bins = 1;
            std::cout << "!!!!!!!!!!!! Send serverStatus = 1 " << std::endl;
            nan_error = false;
        }
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
        servMes.m_transfer_function_count = 0;
        servMes.m_flag_send_bins = 1;
        delete[] tmp_c_bins;
        delete[] tmp_o_bins;
        //add by shimomura 20240603
        delete[] tmp_max;
        delete[] tmp_min;
        delete param.m_transfunc_synthesizer;

     } // end if init_param

        if(init_param == jpv::InitializeParameter::generate_glyph )
        {

            //add by shimomura 2023/06/14
            int cnt = 2;
            tmp_max = new float[cnt]; 
            tmp_min = new float[cnt];

            for ( int tf = 0; tf < cnt; tf++ )
            {
                tmp_max[tf] = FLT_MIN;
                tmp_min[tf] = FLT_MAX;
            }

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
                        GlyphObjectCreator glyph_creator;
                        glyph_creator.setFilterInfo( mvpl.m_list[idx] );
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
            //delete param.m_transfunc_synthesizer;
        }  // end if generate_glyph

        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
        {
            VIS_MODULE_TIMER_END( 470 );
        }
    } // end of timeParam == 2
    else
    {
//        break;
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

#if 1
void generate_particle_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
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
 
    int bsz = 0;
    int st, vl, wid = 0;
    vismodule::PointObject* object = NULL;
    std::vector<GlyphObjectCreator> glyph_creator_lst;

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
#endif

                    point_creator_lst.clear();
                    for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                    {
                        PointObjectCreator point_creator;
                        if ( param.m_gt5d == true ) point_creator.setGT5D();
                        point_creator.setFilterInfo( mvpl.m_list[idx] );
                        point_creator.setCoordSynthStr( clntMes.m_x_synthesis,
                                                        clntMes.m_y_synthesis, clntMes.m_z_synthesis );
//                        point_creator.setCoordSynthTkn( clntMes.x_synthesis_token,
//                                                        clntMes.y_synthesis_token, clntMes.z_synthesis_token );
                        point_creator_lst.push_back( point_creator );
                    }

                    transfunc_creator.setFilterInfo( mvpl.m_list[0] );
                    transfunc_creator.setProtocol( clntMes );
                    transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                    param.m_transfunc_synthesizer = transfunc_creator.create();

                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
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

                    clntMes.show();
                    int tf_count = clntMes.m_transfer_function.size();
                    int c_nbins = DEFAULT_NBINS;
                    int o_nbins = DEFAULT_NBINS;
                    
                    int c_bins_size = 0;
                    int o_bins_size = 0;
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
                        fidx = mvpl.getFileIndex( vl, &xvl );
                        MultiVolumeProperty& mvp = mvpl.m_list[fidx];

                        size_t found_pfi  = mvp.m_file_path.find(".pfi");
                        size_t found_vtm  = mvp.m_file_path.find(".vtm");
                        size_t found_vtu  = mvp.m_file_path.find(".vtm");
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

                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                object = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl );

                            }
#ifdef EXTEND_FILE_FORMAT
                            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                            {
                                object = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                            }                            
#endif
                            else     // filetype: kvsml
                            {
                                object = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st );
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
                        VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
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
                            tmp_max[2*tf] = vismodule::Math::Max( tmp_max[2*tf] ,param.m_transfunc_synthesizer-> m_o_max[tf]);
                            tmp_min[2*tf] = vismodule::Math::Min( tmp_min[2*tf] ,param.m_transfunc_synthesizer-> m_o_min[tf]);
                            //tmp_max[2*tf] = param.m_transfunc_synthesizer-> m_o_max[tf];
                            //tmp_min[2*tf] = param.m_transfunc_synthesizer-> m_o_min[tf];
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

}
#endif
#endif

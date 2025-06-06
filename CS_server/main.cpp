/*****************************************************************************/
/**
 *  main.cpp
 *  CPU Server Program without CUDA, without VIS_MODULE, without OpenGL.
 */
/*****************************************************************************/

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
#include <vismodule/InitialStep>
#include <vismodule/GenerateParticle>

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;

bool useAllNodes = true;


/**
 * ???C??????:
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char** argv )
{
#ifndef CPU_VER
    MPI_Init( &argc, &argv );
#endif
    VIS_MODULE_TIMER_INIT();
    VIS_MODULE_TIMER_STA( 1 );
    Argument param( argc, argv );
    MultiVolumePropertyList mvpl;
    TransferFunctionSynthesizerCreator transfunc_creator;

//    vismodule::Timer timer( vismodule::Timer::Start );
    vismodule::Camera camera;
    //Timer_CS test;

    //2023/06/01 shimomura 
    
    int retval = 0;
    int mpi_rank = 0;
    std::vector<PointObjectCreator> point_creator_lst;
    vismodule::PointObject* object = NULL;
    std::string output, outdir;
    std::string pout = "PARTICLE_OUTDIR";
    std::string prfx = "PARTICLE_SERVER_PREFIX";

    bool nan_error = false; // Add for NaN 2016.01.14

#ifndef CPU_VER
    int rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
	int mpi_size = 1;
#endif

    if ( param.m_batch == true )
    {
    } // end of batch mode

    else  //=================== client-server mode ===================
    {

        char* buf;
        int bsz = 0;
        JobDispatcher jd;
#ifndef CPU_VER
        JobCollector  jc( &jd );
#endif
        int st, vl, wid = 0;

        int c_bins_size = 0;
        int o_bins_size = 0;
        vismodule::UInt64* tmp_c_bins;
        vismodule::UInt64* tmp_o_bins;
                        
        //add by shimomura 2023/06/14
        float*  tmp_max;
        float*  tmp_min;

        if ( rank > 0 )
        {
            //--------------------- WORKER --------------------
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new vismodule::Camera();

            bool loop = true;

            while ( loop )
            {
                static int timer_count = 0;

                // recv cltMes from process 0 >>
#ifndef CPU_VER
                MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
                if ( bsz < 0 )
                {
                    loop = false;
                    break; // terminate server
                }
                buf = new char[bsz];
#ifndef CPU_VER
                MPI_Bcast( buf, bsz, MPI_BYTE, 0, MPI_COMM_WORLD );
#endif
                clntMes.unpack( buf );
                delete[] buf;
                std::cout << "Rank " << rank << ": Recv Client Message" << std::endl;
                std::cout << "clntMes.m_initialize_parameter = " << (int)clntMes.m_initialize_parameter << std::endl;
                // recv cltMes from process 0 <<
               if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
               {
               }
               else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::end )
               {
               }
               else if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::initial_step )
               {
                   //initial_step_worker(*param, clntMes, *servMes, *mlpv);
                   initial_step_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count );
               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
               {
                    generate_particle_worker(param, clntMes, mvpl, nan_error, point_creator_lst, jc, jd, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
               }
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
               {

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
                    mvpl.searchFile(param);
#endif
                    glyph_creator_lst.clear();
                    for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                    {
                        GlyphObjectCreator glyph_creator;
                        glyph_creator.setFilterInfo( mvpl.m_list[idx] );

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
               
               } // end of generate_glyph
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
               {
#if 1

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
#endif
               } // end of plot_over_line
            } // end of while
        }
        else                    // rank == 0
        {
            //--------------------- MASTER --------------------
            int ptss;

            assert( jpv::ParticleTransferUtils::isLittleEndian() );
//            timer.start();
//            VIS_MODULE_TIMER_STA( 10 );
//            VIS_MODULE_TIMER_END( 10 );
//            timer.stop();
//            std::cout << "first reading time[ms]:" << timer.msec() << std::endl;

            jpv::ParticleTransferServer pts;
            ptss = pts.initializeServer( param.m_port );

            jpv::ParticleTransferServerMessage servMes;
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new vismodule::Camera();
            servMes.m_camera = new vismodule::Camera();

            servMes.m_server_status = 0;
            // クライアント接続待ち
            pts.acceptServer();

            while ( ( ptss != -1 ) && ( pts.good() ) )
            {
                static int timer_count = 0;

                ptss = pts.recvMessage( &clntMes );
                //debug add by shimomura 2023/1/18
                clntMes.show();

#ifdef _WIN32 
                    std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(), '/', '\\');
#elif defined(_WIN64)
                    std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(), '/', '\\');
#endif
#ifdef __APPLE__
//                    std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(),"¥"[0], '/');
//                    std::replace(clntMes.m_input_directory.begin(), clntMes.m_input_directory.end(),'\\', '/');
                    std::string target = "¥";
                    std::string replacement = "/";

                    size_t pos = 0;
                    while ((pos = clntMes.m_input_directory.find(target, pos)) != std::string::npos) {
                        clntMes.m_input_directory.replace(pos, target.length(), replacement);
                        pos += replacement.length();
                    }
#elif defined(__linux__)
                    std::string target = "\\";
                    std::string replacement = "/";

                    size_t pos = 0;
                    while ((pos = clntMes.m_input_directory.find(target, pos)) != std::string::npos) {
                        clntMes.m_input_directory.replace(pos, target.length(), replacement);
                        pos += replacement.length();
                    }

#endif
                std::cout << "input_directory = " << clntMes.m_input_directory << std::endl;
                     
                if ( ptss == -1 ) break;
                /* 140319 for client stop by Ctrl+c */
                signal( SIGABRT, SignalHandler );
                signal( SIGTERM, SignalHandler );
                signal( SIGINT, SignalHandler ); /* SIGINT is invalid here, because mpiexec uses it. */
//              signal( SIGSEGV, SignalHandler );
                if ( clntMes.m_step > mvpl.m_total_last_step )
                {
                    clntMes.m_step = mvpl.m_total_last_step;
                }
                else if ( clntMes.m_step < mvpl.m_total_start_steps )
                {
                    clntMes.m_step = mvpl.m_total_start_steps;
                }

                if ( SigServer )
                {
                    clntMes.m_initialize_parameter = jpv::InitializeParameter::end; 
                    std::cout << "*** SigServer" << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                }
                else
                {
                    /* 140319 for client stop by Ctrl+c */
                    if ( clntMes.m_initialize_parameter != jpv::InitializeParameter::initial_step )
                    {
                        clntMes.m_input_directory = param.m_input_data_base;
                    }
                }

                std::cout << "Recieve message initParam = " << static_cast<int>(clntMes.m_initialize_parameter) << std::endl;
                if ( clntMes.m_initialize_parameter == jpv::InitializeParameter::connection_reset )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;
                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                    pts.disconnect();

                    pts.acceptServer();
                }
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::end )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    // ADD by FEAST 2015.12.24
                    //servMes.m_server_status = 0;
                    // ADD END 2015.12.24
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                    break;
                }
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::initial_step ) // change PFI file.
                {
                   initial_step_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count );
                } // end of change PFI
                //else
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
                {
                    generate_particle_master(param, clntMes, servMes, mvpl, nan_error, point_creator_lst, jc, jd, pts, useAllNodes, transfunc_creator, timer_count, clntMes.m_initialize_parameter );
                } // end of initParam == 1 generate_particle 
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
                {
#if 1
                    timer_count++;
                    std::vector<GlyphObjectCreator> glyph_creator_lst;
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

                        if ( timer_count <= VIS_MODULE_TIMER_COUNT_NUM )
                        {
                            VIS_MODULE_TIMER_END( 470 );
                        }
                    } // end of timeParam == 2
                    else
                    {
                        break;
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
                } // end of initParam = 3 // generateglyph
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
                {
                    std::cout << "jpv::InitializeParameter::plot_over_line" << std::endl;
#if 1
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
                            delete object;
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
                        break;
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
                } // end of initParam = 5 plot_over_line


            } // end of while (pts.good)

            delete clntMes.m_camera;
            delete servMes.m_camera;
            bsz = -1;
#ifndef CPU_VER
            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

            pts.termServer();
        }		// rank == 0
    }		// client-server mode
    if ( param.m_batch == true )
    {
        VIS_MODULE_TIMER_END( 1 );
        VIS_MODULE_TIMER_FIN();
    }
#ifndef CPU_VER
    MPI_Finalize();
#endif
    return retval;
}

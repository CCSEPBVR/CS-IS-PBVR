#include "kvs_wrapper_common.h"
#include "kvs_wrapper.h"

#include <cstdio>
#include <fstream>
#include <vector>

#include <vismodule/ValueArray>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/PointObject>
#include <vismodule/TransferFunction>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>

#include <vismodule/ParameterFileReader>

#include <vismodule/ParticleProperty>

// Generate
#include <vismodule/PointObjectGenerator>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/PlotOverLineGenerator>
#include <vismodule/PlotOverTimeGenerator>

#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/GeneratePOL>

#include <vismodule/EnsembleParticleGenerator> // GenerateEnsembleParticlesStruct, EnsembleParticleArrays, EnsembleStatisticRange

namespace Generator = vismodule::CellByCellParticleGenerator;

static bool is_initial_step = true;
static std::size_t start_time_step = 0;

static void CollectParticleMinMax(
    ParticleProperty& particle_property,
    const domain_parameters_struct& dom,
    Type** values,
    const int nvariables,
    const int tf_number,
    float* tmp_max,
    float* tmp_min
)
{
    vismodule::PointObjectGenerator point_object_generator;
    const char tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method = 'x';

    vismodule::PointObject* point_object = point_object_generator.GenerateParticleStruct(
        particle_property, dom, values, nvariables, ServerMode::IS
    );

    MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );

    delete point_object;
    particle_property.m_sampling_method = tmp_sampling_method;
}

static void CollectParticleHistogram(
    ParticleProperty& particle_property,
    const domain_parameters_struct& dom,
    Type** values,
    const int nvariables,
    const int tf_number,
    vismodule::UInt64* tmp_c_bins,
    vismodule::UInt64* tmp_o_bins
)
{
    vismodule::PointObjectGenerator point_object_generator;
    const char tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method = 'h';

    vismodule::PointObject* point_object = point_object_generator.GenerateParticleStruct(
        particle_property, dom, values, nvariables, ServerMode::IS
    );

    MakeHistgram( point_object, tf_number, tmp_c_bins, tmp_o_bins );

    delete point_object;
    particle_property.m_sampling_method = tmp_sampling_method;
}

static void GenerateParticleObject(
    ParticleProperty& particle_property,
    const domain_parameters_struct& dom,
    Type** values,
    const int nvariables,
    std::vector<float>& particle_coords,
    std::vector<Byte>& particle_colors,
    std::vector<float>& particle_normals
)
{
    vismodule::PointObjectGenerator point_object_generator;
    vismodule::PointObject* point_object = point_object_generator.GenerateParticleStruct(
        particle_property, dom, values, nvariables, ServerMode::IS
    );

    MakeParticle( point_object, particle_coords, particle_colors, particle_normals );

    delete point_object;
}

void OutputCoordMinMaxFile(
    const domain_parameters_struct& dom,
    const std::string& coordMinMaxFilePath
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    static bool minmaxFlag = false;
    if (minmaxFlag == false && mpi_rank == 0) {
        FILE* fp = fopen( coordMinMaxFilePath.c_str(), "w" );
        if( fp )
        {
            fprintf( fp, "%f %f %f %f %f %f\n",
                     dom.x_global_min,
                     dom.y_global_min,
                     dom.z_global_min,
                     dom.x_global_max,
                     dom.y_global_max,
                     dom.z_global_max );
            fclose( fp );
        }
        minmaxFlag = true;
    }
}

void generate_particles(
    int time_step,
    domain_parameters_struct dom,
    Type** values, 
    int nvariables
)
{
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif

    if ( is_initial_step == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    std::string historyFilePath;    
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old;
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        plotOverTimeFilePrefix,
        tfJsonPath,
        tfJsonPath_old,
        tfJsonPath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old,
        plotOverTimeParameterPath,
        plotOverTimeParameterPath_old
    );

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;
    PlotOverTimeProperty pot_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    bool object_generation_enabled = false;
    SetParticleParameter(
        dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
        nvariables, object_generation_enabled
    );
    if ( object_generation_enabled )
    {
        SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, glyph_property );
        SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, pol_property );
        SetPlotOverTimeParameter( plotOverTimeParameterPath, plotOverTimeParameterPath_old, pot_property );
    }

    const int tf_number  = particle_property.m_transfunc_array.size();
    const int resolution = object_generation_enabled ? pol_property.m_sampling_size : 0;

    // particle parameters
    std::vector<float> particle_coords;
    std::vector<Byte>  particle_colors;
    std::vector<float> particle_normals;
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity

    for ( std::size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
    }

    for ( int i = 0; i < (tf_number * 2); i++ )
    {
        tmp_max[i] = FLT_MIN;
        tmp_min[i] = FLT_MAX;
    }

    // glyph parameters
    std::vector<float>         glyph_coords;
    std::vector<float>         glyph_vectors;
    std::vector<float>         glyph_sizes;
    std::vector<unsigned char> glyph_colors;

    // plot over line parameters
    std::vector<float> values_on_line( resolution, 0 );
    std::vector<int>   mask( resolution, 0 );
    std::vector<float> x_axis( resolution, 0 );

    // plot over time paraemters
    std::vector<float> value_on_time( nvariables, 0 );
    bool pot_mask = false;
    
    ServerMode server_mode = ServerMode::IS;
    CollectParticleMinMax(
        particle_property, dom, values, nvariables, tf_number, tmp_max, tmp_min
    );

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    for( std::size_t i = 0; i < tf_number; i++ )
    {
        const float color_variable_min   = tmp_min[2 * i + 1];
        const float color_variable_max   = tmp_max[2 * i + 1];
        const float opacity_variable_min = tmp_min[2 * i    ];
        const float opacity_variable_max = tmp_max[2 * i    ];

        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max );
        }

        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
        }
    }

    CollectParticleHistogram(
        particle_property, dom, values, nvariables, tf_number, tmp_c_bins, tmp_o_bins
    );

    if ( object_generation_enabled )
    {
        GenerateParticleObject(
            particle_property, dom, values, nvariables,
            particle_coords, particle_colors, particle_normals
        );

        if ( glyph_property.m_glyph_flag )
        {
            vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
            vismodule::GlyphSeedGenerator glyph_creator;
            int number_of_divide = mpi_size;
            glyph_creator.GenerateGlyphStruct(
                glyph_property, number_of_divide, dom,
                values, nvariables, server_mode, glyph_object
            );

            MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

            delete glyph_object;
        }

        // if ( pol_property.m_plot_flag )
        if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
        {
            vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
            PlotOverLineGenerator pol_generator;
            pol_generator.GeneratePOLStruct(
                pol_property, dom, values, nvariables, pol_object
            );

            for( std::size_t i = 0; i < resolution; i++ )
            {
                x_axis[i] = pol_object->x_axis()[i];
                if ( pol_object->mask()[i] )
                {
                    mask[i]           = 1;
                    values_on_line[i] = pol_object->values_on_line()[i];
                }
            }
            delete pol_object;
        }

        // if ( pot_property.m_plot_flag )
        if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
        {
            PlotOverTimeGenerator pot_generator;
            pot_mask = pot_generator.GeneratePOTStruct( pot_property, dom, values, nvariables, value_on_time );
        }
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( object_generation_enabled )
    {
        OutputParticleFiles(
            particle_property, mvpl, time_step, particleFilePrefix,
            particle_coords, particle_colors, particle_normals
        );
    }

    OutputParticleHistory(
        particle_property, tf_number, nvariables, historyFilePath,
        tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( object_generation_enabled && glyph_property.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    // if ( pol_property.m_plot_flag )
    if ( object_generation_enabled ) // 常に出力し続ける、将来的に変わる可能性あり
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }

    // if ( pot_property.m_plot_flag )
    if ( object_generation_enabled ) // 常に出力し続ける、将来的に変わる可能性あり
    {
        OutputPOT( time_step, plotOverTimeFilePrefix, pot_mask, value_on_time );
    }

    // OutputParticlesで更新したdefault_old.jsonをtimestep別JSONに保存する
    if ( object_generation_enabled && mpi_rank == 0 )
    {
        std::ifstream src( tfJsonPath_old.c_str(), std::ios::binary );
        std::ofstream dst( tfJsonPath_step.c_str(), std::ios::binary );
        dst << src.rdbuf();
    }
   
    // 粒子ファイル書き込みスレッドが終了するまで待機
    // async_io_enabled, pwtはkvs_wrapper_common.cppに宣言
    if ( async_io_enabled )
    {
        std::cout << "Particle write thread is active."             << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;
    
#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD ); // すべてのプロセスでファイルの書き込む処理が終了するまで待機
#endif
    
    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step       << std::endl;

        ofs.close();
    }
   
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    delete[] tmp_max;
    delete[] tmp_min;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

}

// 構造格子データのアンサンブルPBVR粒子生成ラッパー。
// 通常 generate_particles と同じ準備(パス設定/TF設定)を行い、計算本体を
// vismodule::GenerateEnsembleParticlesStruct に委譲する(非構造版 ensemble_generate_particles と対称)。
bool ensemble_generate_particles(
    int time_step,
    const int num_ensemble,
    domain_parameters_struct dom,
    Type** values,
    int nvariables
)
{
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif

    if ( is_initial_step == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    std::string historyFilePath;
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old;
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;
    SetParameterFilePath(
        time_step,
        historyFilePath, stateFilePath, coordMinMaxFilePath,
        particleFilePrefix, glyphFilePrefix, plotOverLineFilePrefix, plotOverTimeFilePrefix,
        tfJsonPath, tfJsonPath_old, tfJsonPath_step,
        glyphParameterPath, glyphParameterPath_old,
        plotOverLineParameterPath, plotOverLineParameterPath_old,
        plotOverTimeParameterPath, plotOverTimeParameterPath_old
    );

    ParticleProperty particle_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    bool object_generation_enabled = false;
    SetParticleParameter(
        dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
        nvariables, object_generation_enabled
    );

    // アンサンブル統計量粒子(ave/var/cov)の格納先と統計レンジ
    vismodule::EnsembleParticleArrays average;
    vismodule::EnsembleParticleArrays variance;
    vismodule::EnsembleParticleArrays coefficient;
    vismodule::EnsembleStatisticRange average_range;
    vismodule::EnsembleStatisticRange variance_range;
    vismodule::EnsembleStatisticRange co_variation_range;

    // 構造格子版の計算本体(Phase1 は骨組みで false を返す。Phase2 で実装)
    bool ok = vismodule::GenerateEnsembleParticlesStruct(
        num_ensemble,
        particle_property,
        dom,
        values,
        nvariables,
        average, variance, coefficient,
        average_range, variance_range, co_variation_range
#ifdef ENABLE_ENSEMBLE_TIMER
        , NULL   // Phase1: timer 未配線(Phase2 で EnsembleTimerCollector を渡す)
#endif
    );

    (void)mpi_rank;
    (void)mpi_size;

    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

    if ( !ok ) return false;

    // TODO(Phase2): OutputEnsembleStatisticHistory / OutputEnsembleStatisticParticles で
    //               ave/var/cov 粒子と統計履歴を出力する(shared/EnsembleStatisticOutput を配線)。
    return true;
}

bool SetParticleParameter(
    const domain_parameters_struct& dom,
    const std::string& tfJsonPath,
    const std::string& tfJsonPath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables,
    bool& object_generation_enabled
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    ParameterFileReader ppr;
    int size = 0;
    char* buf = nullptr;
    int object_generation_enabled_int = 0;
    static bool hasCachedParticleProperty = false;
    static std::vector<char> cachedParticlePropertyBuffer;

    if ( mpi_rank == 0 )
    {
        std::ifstream tfJson( tfJsonPath );
        std::ifstream tfJsonOld( tfJsonPath_old );

        bool loaded = false;
        if ( tfJson.good() )
        {
            loaded = ppr.readTransferFunctionFromJson( tfJsonPath.c_str(), particle_property );
            if ( loaded )
            {
                std::rename( tfJsonPath.c_str(), tfJsonPath_old.c_str() );
            }
        }
        if ( !loaded && !tfJson.good() && hasCachedParticleProperty )
        {
            size = static_cast<int>( cachedParticlePropertyBuffer.size() );
            if ( size > 0 )
            {
                buf = new char[size];
                for ( int i = 0; i < size; ++i )
                {
                    buf[i] = cachedParticlePropertyBuffer[i];
                }
                particle_property.unpack( buf );
                delete[] buf;
                buf = nullptr;
            }
            loaded = size > 0;
        }
        if ( !loaded && tfJsonOld.good() )
        {
            loaded = ppr.readTransferFunctionFromJson( tfJsonPath_old.c_str(), particle_property );
        }

        if ( loaded )
        {
            object_generation_enabled_int = 1;
        }
        else if ( size == 0 )
        {
            std::cout << "================================================================" << std::endl;
            std::cout << "[WARN] Failed to load transfer function json." << std::endl;
            std::cout << "[WARN] Files: " << FileNameOnly( tfJsonPath ) << " and "
                      << FileNameOnly( tfJsonPath_old ) << std::endl;
            std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnset( "VIS_PARAM_DIR" ) << std::endl;
            std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnset( "PARTICLE_DIR" ) << std::endl;
            std::cout << "[INFO] Set default particle parameters and skip object generation." << std::endl;
            std::cout << "================================================================" << std::endl;

            SetDefaultParticleParameter( particle_property, mvpl, nvariables );
            object_generation_enabled_int = 0;
        }

        size = particle_property.byteSize();

        if ( size > 0 )
        {
            buf = new char[size];
            particle_property.pack( buf );
            if ( object_generation_enabled_int != 0 )
            {
                cachedParticlePropertyBuffer.assign( buf, buf + size );
                hasCachedParticleProperty = true;
            }
        }
    }

#ifndef CPU_VER
    MPI_Bcast( &object_generation_enabled_int, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
    object_generation_enabled = object_generation_enabled_int != 0;

    if ( size > 0 )
    {
        if ( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        if( mpi_rank > 0 ) particle_property.unpack( buf );
        if ( mpi_rank > 0 && object_generation_enabled )
        {
            cachedParticlePropertyBuffer.assign( buf, buf + size );
            hasCachedParticleProperty = true;
        }
        particle_property.UpdateTransferFunctionSynthesizer();
        delete[] buf;
    }
    else
    {
        return false;
    }

    vismodule::Vector3f min_object_coords(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min
    );
    vismodule::Vector3f max_object_coords(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max
    );

    mvpl.m_total_min_object_coord  = min_object_coords;
    mvpl.m_total_max_object_coord  = max_object_coords;

    const float min = vismodule::Math::Min(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min
    );

    const float max = vismodule::Math::Max(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max
    );

    vismodule::StructuredVolumeObject object;
    object.setMinMaxObjectCoords( min_object_coords, max_object_coords );
    object.setMinMaxExternalCoords( min_object_coords, max_object_coords );

    const double total_volume = ( dom.x_global_max - dom.x_global_min )
                              * ( dom.y_global_max - dom.y_global_min )
                              * ( dom.z_global_max - dom.z_global_min );

    const float max_opacity           = 0.98;
    const int particle_limit          = particle_property.m_particle_limit;
    const float extra_opacity_factor  = particle_property.m_extra_opacity_factor;
    particle_property.m_sampling_step = ( max - min ) / 1E1 / extra_opacity_factor;
    const float sampling_step         = particle_property.m_sampling_step;
    const int subpixel_level          = CalculateSubpixelLevel(
        particle_limit,
        *particle_property.m_camera,
        sampling_step,
        total_volume,
        &object
    );

    float sampling_volume_inverse;
    float max_density;

    Generator::CalculateDensityParameters(
        particle_property.m_camera,
        &object,
        (float)subpixel_level,
        sampling_step,
        max_opacity,
        &sampling_volume_inverse,
        &max_density
    );

    particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
    particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
    particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

    if( mpi_rank == 0 )
    {
        fprintf( stdout , "---------initialize Parameters---------------------------------------------\n");
        fprintf( stdout , "particle_limit    = %20d\n"      , particle_limit                             );
        fprintf( stdout , "extra_opacity_factor  = %20f\n"  , extra_opacity_factor                       );
        fprintf( stdout , "resolutin_height  = %20d\n"      , particle_property.m_camera->windowHeight() );
        fprintf( stdout , "resolutin_width   = %20d\n"      , particle_property.m_camera->windowWidth()  );
        fprintf( stdout , "total_volume      = %20.3e\n"    , total_volume                               );
        fprintf( stdout , "  |-X             = %20f\n"      , object.maxObjectCoord().x()                );
        fprintf( stdout , "  |-Y             = %20f\n"      , object.maxObjectCoord().y()                );
        fprintf( stdout , "  |-Z             = %20f\n"      , object.maxObjectCoord().z()                );
        fprintf( stdout , "max_opacity       = %20.3e\n"    , max_opacity                                );
        fprintf( stdout , "max_density       = %20.3e\n"    , max_density                                );
        fprintf( stdout , "sampling_step     = %20.3e\n"    , sampling_step                              );
        fprintf( stdout , "subpixel_level    = %20d\n"      , subpixel_level                             );
        fprintf( stdout , "--------------------------------------------------------------------------\n" );
    }

    return true;
}

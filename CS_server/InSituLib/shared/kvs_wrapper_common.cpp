#include "kvs_wrapper_common.h"
#include <cstdio>
#include <fstream>
#include <vector>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/ParameterFileReader>

#ifndef CPU_VER
#include <mpi.h>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

namespace Generator = vismodule::CellByCellParticleGenerator;

// Asynchronous io, using worker thread pwt.
#include "particle_write_thread.h"
bool async_io_enabled = false;
pbvr::ParticleWriteThread pwt;

/**
 * @brief begin_wrapper_async_io , call to begin async wrapper output
 */
void begin_wrapper_async_io()
{
    async_io_enabled=true;
}
/**
 * @brief end_wrapper_async_io , call to end async wrapper ouput - and wait for last worker thread to finish.
 */
void end_wrapper_async_io()
{
    if(async_io_enabled)
    {
        pwt.join(true);
        async_io_enabled=false;
    }
}

size_t CalculateSubpixelLevel(
    const int particle_limit,
    const vismodule::Camera& camera,
    const float sampling_step,
    const double total_volume,
    const vismodule::ObjectBase* volume
)
{
    double d_nparticles = 0.0;//particle density for subpixel_level=1
    d_nparticles = Generator::CalculateGreatDensity( camera, *volume, 1, sampling_step ) * total_volume;

    //Calculation of optimized subpixel level
    float plimit = static_cast<float>( particle_limit );
    float nparticles = static_cast<float>( d_nparticles );
    float subpixel_level = sqrt( plimit / nparticles );

    if ( subpixel_level < 1 ) subpixel_level = 1;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

bool SetParameterFilePath(
    const int time_step,
    std::string& historyFilePath,
    std::string& stateFilePath,
    std::string& coordMinMaxFilePath,
    std::string& particleFilePrefix,
    std::string& glyphFilePrefix,
    std::string& plotOverLineFilePrefix,
    std::string& tfFilePath,
    std::string& tfFilePath_old,
    std::string& tfFilePath_step,
    std::string& glyphParameterPath,
    std::string& glyphParameterPath_old,
    std::string& plotOverLineParameterPath,
    std::string& plotOverLineParameterPath_old
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    std::string visParamDir;
    std::string tfFilename;
    static bool is_first_setting = true;

    const char *envBuf = NULL;
    envBuf = std::getenv( "VIS_PARAM_DIR" );
    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }
    envBuf = std::getenv( "TF_NAME" );
    if (envBuf == NULL) {
        tfFilename = "default";
    }
    else {
        tfFilename = envBuf;
    }

    std::stringstream step;
    step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;
    historyFilePath = visParamDir + "history" + step.str() + ".txt";
    stateFilePath     = visParamDir + "state.txt";
    
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        coordMinMaxFilePath    = "./t_pfi_coords_minmax.txt";
        particleFilePrefix     = "./t_";
        glyphFilePrefix        = "./g_";
        plotOverLineFilePrefix = "./p_";
    }
    else {
        coordMinMaxFilePath    = envBuf;
        particleFilePrefix     = envBuf;
        glyphFilePrefix        = envBuf;
        plotOverLineFilePrefix = envBuf;
        if (coordMinMaxFilePath[coordMinMaxFilePath.size() - 1] != '/') {
            coordMinMaxFilePath    += "/t_pfi_coords_minmax.txt";
            particleFilePrefix     += "/t_";
            glyphFilePrefix        += "/g_";
            plotOverLineFilePrefix += "/p_";
        }
        else {
            coordMinMaxFilePath    += "t_pfi_coords_minmax.txt";
            particleFilePrefix     += "t_";
            glyphFilePrefix        += "g_";
            plotOverLineFilePrefix += "p_";
        }
    }

    tfFilePath                    = visParamDir + tfFilename + ".tf";
    tfFilePath_old                = visParamDir + tfFilename + "_old.tf";
    tfFilePath_step               = visParamDir + tfFilename + step.str() + ".tf";
    glyphParameterPath            = visParamDir + "parameter.gly";
    glyphParameterPath_old        = visParamDir + "parameter_old.gly";
    plotOverLineParameterPath     = visParamDir + "parameter.pol";
    plotOverLineParameterPath_old = visParamDir + "parameter_old.pol";

    std::ifstream tfFile( tfFilePath );
    std::ifstream glyphParameterFile( glyphParameterPath );
    std::ifstream plotOverLineParameterFile( plotOverLineParameterPath );

    if ( is_first_setting && mpi_rank == 0 )
    {
        if ( !tfFile.good() )
        {
            std::cout << "ERROR:default.tf is not existed." << std::endl;
            return false;
        }

        if ( !glyphParameterFile.good() )
        {
            std::cout << "ERROR:parameter.gly is not existed." << std::endl;
            return false;
        }

        if ( !plotOverLineParameterFile.good() )
        {
            std::cout << "ERROR:parameter.pol is not existed." << std::endl;
            return false;
        }
    }

    is_first_setting = false;
    return true;
}

bool SetGlyphParameter(
    const std::string& glyphParameterPath,
    const std::string& glyphParameterPath_old,
    Argument& param,
    NameListFile& nameListFile
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
    char* buf;

    if ( mpi_rank == 0 )
    {
        bool is_file_exist = false;
        std::ifstream glyphParameterFile( glyphParameterPath );

        if ( glyphParameterFile.good() )
        {
            is_file_exist = true;
            ppr.readGlyphParameterFile( glyphParameterPath.c_str() );
            nameListFile = ppr.getNameListFile();
            std::rename( glyphParameterPath.c_str(), glyphParameterPath_old.c_str() );
        }
        else
        {
            ppr.setNameListFile( nameListFile );
        }

        if ( is_file_exist ) size = nameListFile.byteSize();
        else size = 0;

        if ( size > 0 )
        {
            buf = new char[size];
            nameListFile.pack( buf );
        }
    }

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    if ( size > 0 )
    {
        if ( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        if( mpi_rank > 0 ) nameListFile.unpack( buf );
        delete[] buf;
    }

    if ( mpi_rank > 0 ) ppr.setNameListFile( nameListFile );
    ppr.setGlyphParameter( param );

    return true;
}

bool SetPlotOverLineParameter(
    const std::string& plotOverLineParameterPath,
    const std::string& plotOverLineParameterPath_old,
    Argument& param,
    NameListFile& nameListFile
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
    char* buf;

    if ( mpi_rank == 0 )
    {
        bool is_file_exist = false;
        std::ifstream plotOverLineParameterFile( plotOverLineParameterPath );

        if ( plotOverLineParameterFile.good() )
        {
            is_file_exist = true;
            ppr.readPlotOverLineParameterFile( plotOverLineParameterPath.c_str() );
            nameListFile = ppr.getNameListFile();
            std::rename( plotOverLineParameterPath.c_str(), plotOverLineParameterPath_old.c_str() );
        }
        else
        {
            ppr.setNameListFile( nameListFile );
        }

        if ( is_file_exist ) size = nameListFile.byteSize();
        else size = 0;

        if ( size > 0 )
        {
            buf = new char[size];
            nameListFile.pack( buf );
        }        
    }

#ifndef CPU_VER
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif

    if ( size > 0 )
    {
        if ( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        if( mpi_rank > 0 ) nameListFile.unpack( buf );
        delete[] buf;
    }

    if ( mpi_rank > 0 ) ppr.setNameListFile( nameListFile );
    ppr.setPlotOverLineParameter( param );

    return true;
}

void MakeParticle(
    const vismodule::PointObject* point_object,
    std::vector<float>& coords,
    std::vector<Byte>&  colors,
    std::vector<float>& normals
)
{
    size_t nmemb = point_object->nvertices() * 3;
    vismodule::ValueArray<vismodule::Real32> coords_array ( point_object->coords().pointer() , nmemb );
    vismodule::ValueArray<vismodule::UInt8>  colors_array ( point_object->colors().pointer() , nmemb );
    vismodule::ValueArray<vismodule::Real32> normals_array( point_object->normals().pointer(), nmemb );

    coords.insert( coords.end(), coords_array.begin(), coords_array.end() );
    colors.insert( colors.end(), colors_array.begin(), colors_array.end() );
    normals.insert( normals.end(), normals_array.begin(), normals_array.end() );
}

void MakeGlyph(
    const vismodule::KVSMLObjectGlyph* glyph_object,
    std::vector<float>& coords,
    std::vector<float>& vectors,
    std::vector<float>& sizes,
    std::vector<unsigned char>& colors
)
{
auto safe_append = [](auto& dst, auto const& src, char const* what){
    using size_type = typename std::decay_t<decltype(dst)>::size_type;
    const size_type dsz = dst.size();
    const size_type ssz = src.size();
    const size_type mx  = dst.max_size();

    //　受け渡しの際にセグフォエラーが発生する？これの予防および、調査のためのラムダ関数 
    // オーバーフロー防止（mx - dsz の形で比較）
    if (ssz > mx - dsz) {
        std::ostringstream oss;
        oss << what << ": length_error: size(" << dsz << ") + add(" << ssz
            << ") > max_size(" << mx << ")";
        throw std::length_error(oss.str());
    }
    dst.reserve(dsz + ssz);           // 先に確保しておくと例外の発生箇所がここに来る
    dst.insert(dst.end(), src.begin(), src.end());
};

    auto const& c = glyph_object->coords();
    auto const& v = glyph_object->directions();
    auto const& s = glyph_object->sizes();
    auto const& k = glyph_object->colors();

#if 0 // for debug
     std::cout << "glyph_param.m_glyph_sizes        = " << glyph_param.m_glyph_sizes.size()     << std::endl; 
     std::cout << "glyph_seed.glyph_sizes()         = " << glyph_seed.glyph_sizes().size()      << std::endl; 
     std::cout << "glyph_param.m_glyph_vectors      = " << glyph_param.m_glyph_vectors.size()   << std::endl; 
     std::cout << "glyph_seed.glyph_directions()    = " << glyph_seed.glyph_directions().size() << std::endl; 
     std::cout << "m_glyph_coords.size()            = " << glyph_param.m_glyph_coords.size()    << std::endl; 
     std::cout << "glyph_seed.glyph_coords().size() = " << glyph_seed.glyph_coords().size()     << std::endl; 
     std::cout << "m_glyph_coords.size()            = " << glyph_param.m_glyph_colors.size()    << std::endl; 
     std::cout << "glyph_seed.glyph_colors()        = " << glyph_seed.glyph_colors().size()     << std::endl; 

    std::cout << "vectors:"
              << " cur = " << glyph_param.m_glyph_vectors.size()
              << " add = " << v.size() 
              << " max = " << glyph_param.m_glyph_vectors.max_size() << std::endl;
    std::cout << "coords :"
              << " cur = " << glyph_param.m_glyph_coords.size()
              << " add = " << c.size()
              << " max = " << glyph_param.m_glyph_coords.max_size() << std::endl;
    std::cout << "colors :"
              << " cur = " << glyph_param.m_glyph_colors.size()
              << " add = " << k.size()
              << " max = " << glyph_param.m_glyph_colors.max_size() << std::endl;
    std::cout << "sizes  :"
              << " cur = " << glyph_param.m_glyph_sizes.size()
              << " add = " << s.size()
              << " max = " << glyph_param.m_glyph_sizes.max_size() << std::endl;
#endif // for debug

    // 集約処理
    safe_append(coords , c, "m_glyph_coords");
    safe_append(vectors, v, "m_glyph_vectors");
    safe_append(sizes  , s, "m_glyph_sizes");
    safe_append(colors , k, "m_glyph_colors");
}

void OutputParticles(
    const Argument& param,
    const MultiVolumePropertyList& mvpl,
    const int start_time_step,
    const int time_step,
    const int tf_number,
    const int nvariables,
    const std::string& particleFilePrefix,
    const std::string& stateFilePath,
    const std::string& histryFilePath,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals,
    const vismodule::UInt64* c_bins,
    const vismodule::UInt64* o_bins,
    const float* max_array,
    const float* min_array
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    static bool first_step = true;
    static int count;
    static int num_nodes;

#ifndef CPU_VER
    static MPI_Comm new_comm;
#endif

    /* 各ノード毎に粒子データを出力する。 */
    if( first_step )
    {
#ifndef CPU_VER
        int numprocs, myrank;
        int resultlen;
        char procname[MPI_MAX_PROCESSOR_NAME];
        char* procname_bak;
        char* procname_g;
        char* procname_p;

        MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
        MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

        /* ノード名を取得し、各ランクで共有する. */
        MPI_Get_processor_name( procname, &resultlen );
        procname_g = new char[ MPI_MAX_PROCESSOR_NAME * numprocs ];
        MPI_Allgather( procname,   MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       procname_g, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       MPI_COMM_WORLD );

        int color;
        count = 1;
        for( color = 0; color < numprocs; color++ )
        {
            procname_p = procname_g + MPI_MAX_PROCESSOR_NAME * color;

            /* 要素の隣同士を比較して差異があった場合にカウントし, *
             * ノード毎に連続した番号を割り当てる.                 */
            if( color > 0 )
            {
                procname_bak = procname_p - MPI_MAX_PROCESSOR_NAME;
                if( strcmp( procname_p, procname_bak ) != 0 )
                    count++;
            }

            /* 自分のノード名が一致した要素番号をコミュニケータ分割のcolorとする */
            if( strcmp( procname_p, procname ) == 0 )
                break;
        }

        delete[] procname_g;
        
        MPI_Comm_split( MPI_COMM_WORLD, color, myrank, &new_comm );
        
        int split_numprocs;
        MPI_Comm_size( new_comm, &split_numprocs );
        
        /*
         * 各ノードに均等にランクが割り当てられることを前提とし,
         * 分割前のプロセス数と分割後のプロセス数の非を粒子ファイル数とする.
         */
        num_nodes = numprocs / split_numprocs;
        if( numprocs % split_numprocs > 0 ) num_nodes++;
#else
        count = 1;
        num_nodes = 1;
#endif
        first_step = false;
    }   
    
    /*
     * ファイル名の粒子データのファイル名を入力する.
     * countが各ファイルで連続でない場合,ファイルが不在と見なしてデーモンでスピンロックがかかる.
     */
    // 環境変数で指定したファイルパスを参照する
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
    const std::string particleFilePath = particleFilePrefix + ss.str();

    int particle_size = coords.size();
    int *recvcounts;
    int *displs;

    int new_rank;
    int new_number_of_process;
#ifndef CPU_VER
    MPI_Comm_rank( new_comm, &new_rank );
    MPI_Comm_size( new_comm, &new_number_of_process );
#else
    new_rank = 0;
    new_number_of_process = 1;
#endif

    /*
     *  recvcounts: 各ランク毎の受信バッファサイズ.
     *  displs:     受信先バッファ上の各ランク毎の受信バッファの位置(オフセット)
     */

    displs = new int[ new_number_of_process ];
    recvcounts = new int[ new_number_of_process ];

#ifndef CPU_VER
    MPI_Allgather( &particle_size, 1, MPI_INT,
                   recvcounts,     1, MPI_INT,
                   new_comm );
#else
    recvcounts[0] = particle_size;
#endif

    displs[0] = 0;
    for( int i = 1; i < new_number_of_process; i++ )
        displs[i] = displs[i-1] + recvcounts[i-1];

    vismodule::ValueArray<float> new_coords(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    vismodule::ValueArray<Byte>  new_colors(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    vismodule::ValueArray<float> new_normals( displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );

#ifndef CPU_VER
    vismodule::ValueArray<float> tmp_coords( coords );
    vismodule::ValueArray<Byte>  tmp_colors( colors );
    vismodule::ValueArray<float> tmp_normals( normals );

    MPI_Gatherv( tmp_coords.pointer(),   particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );

    MPI_Gatherv( tmp_colors.pointer(),   particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, new_comm );

    MPI_Gatherv( tmp_normals.pointer(),   particle_size, MPI_FLOAT,
                 new_normals.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );
#else
    for( int i = 0; i < particle_size; i++ )
    {
        new_coords[i]  = coords[i];
        new_colors[i]  = colors[i];
        new_normals[i] = normals[i];
    }
#endif

    /*  分割後コミュニケータのランク0で出力する  */
    if( new_rank == 0 )
    {
        vismodule::PointObject* point_object = new vismodule::PointObject( new_coords, new_colors, new_normals, param.m_subpixel_level );
        point_object->setMinMaxObjectCoords( mvpl.m_total_min_object_coord, mvpl.m_total_max_object_coord );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread = &pwt;
            particle_write_thread->join( true );
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename( particleFilePath.c_str() );
            particle_write_thread->setTimestep( time_step , stateFilePath.c_str() );
            particle_write_thread->setStartTimestep( start_time_step ); //add by shimomura 20240808
            particle_write_thread->work( true );
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            vismodule::KVSMLObjectPoint* kvsml_object = new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( *point_object );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }

    // histgram receiver
    vismodule::UInt64* c_bins_recv;
    vismodule::UInt64* o_bins_recv;
    c_bins_recv = new vismodule::UInt64[tf_number * DEFAULT_NBINS];
    o_bins_recv = new vismodule::UInt64[tf_number * DEFAULT_NBINS];

    for ( size_t i = 0; i < (tf_number * DEFAULT_NBINS); i++ )
    {
        c_bins_recv[i] = 0;
        o_bins_recv[i] = 0;
    }

#ifndef CPU_VER
    //ヒストグラムの集計
    MPI_Reduce( c_bins, c_bins_recv, (tf_number * DEFAULT_NBINS), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    MPI_Reduce( o_bins, o_bins_recv, (tf_number * DEFAULT_NBINS), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
#else
    for( int i = 0; i < (tf_number * DEFAULT_NBINS); i++ )
    {
        c_bins_recv[i] = c_bins[i];
        o_bins_recv[i] = o_bins[i];
    }
#endif

    // min max receiver
    float* max_array_recv;
    float* min_array_recv;
    max_array_recv = new float[tf_number * 2]; // color, opacity
    min_array_recv = new float[tf_number * 2]; // color, opacity

    for ( size_t i = 0; i < (tf_number * 2); i++ )
    {
        max_array_recv[i] = FLT_MIN;
        min_array_recv[i] = FLT_MAX;
    }

#ifndef CPU_VER
    MPI_Reduce( max_array, max_array_recv, (tf_number * 2), MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( min_array, min_array_recv, (tf_number * 2), MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
#else
    for( size_t i = 0; i < (tf_number * 2); i++ )
    {
        max_array_recv[i] = max_array[i];
        min_array_recv[i] = min_array[i];
    }
#endif

    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if ( !async_io_enabled ){
            std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs << "START_STEP  = " << start_time_step << std::endl;
            ofs << "LATEST_STEP = " << time_step       << std::endl;

            ofs.close();
        }

        std::ofstream ofs2( histryFilePath.c_str(), std::ios::out);
        ofs2 << "TF_NUMBER=" << tf_number << std::endl;

        for( size_t i = 0; i < tf_number; i++ )
        {
            ofs2 << "MIN_O" << (i + 1) << "=" << min_array_recv[2 * i    ] << std::endl;
            ofs2 << "MAX_O" << (i + 1) << "=" << max_array_recv[2 * i    ] << std::endl;
            ofs2 << "MIN_C" << (i + 1) << "=" << min_array_recv[2 * i + 1] << std::endl;
            ofs2 << "MAX_C" << (i + 1) << "=" << max_array_recv[2 * i + 1] << std::endl;
            ofs2 << "RESOLUTION_O" << (i + 1) << "=" << DEFAULT_NBINS << std::endl;
            ofs2 << "HISTOGRAM_O"  << (i + 1) << "=";
            for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
            {
                ofs2 << o_bins_recv[j + (i * DEFAULT_NBINS)] << ",";
            }
            ofs2 << std::endl;
            ofs2 << "RESOLUTION_C" << (i + 1) << "=" << DEFAULT_NBINS << std::endl;
            ofs2 << "HISTOGRAM_C"  << (i + 1) << "=";
            for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
            {
                ofs2 << c_bins_recv[j + (i * DEFAULT_NBINS)] << ",";
            }
            ofs2 << std::endl;
        }

        ofs2 << "N_VARIABLES="      << nvariables               << std::endl;
        ofs2 << "PARTICLE_DENSITY=" << param.m_particle_density << std::endl;
        ofs2 << "PARTICLE_LIMIT="   << param.m_particle_limit   << std::endl;
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();
    }
}

void OutputGlyphs(
    const int time_step,
    const std::string& glyphFilePrefix,
    const std::vector<float>& coords,
    const std::vector<float>& vectors,
    const std::vector<float>& sizes,
    const std::vector<unsigned char>& colors
)
{
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif

    vismodule::ValueArray<float> tmp_coords( coords );
    vismodule::ValueArray<float> tmp_vectors( vectors );
    vismodule::ValueArray<Byte>  tmp_colors( colors );
    vismodule::ValueArray<float> tmp_sizes( sizes );

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    std::string glyphFilePath;
    glyphFilePath = glyphFilePrefix + ss.str();
    
    vismodule::KVSMLObjectGlyph glyph_object( tmp_coords, tmp_colors, tmp_vectors, tmp_sizes );
    glyph_object.write( glyphFilePath.c_str() );
}

void OutputLine(
    const int time_step,
    const std::string& plotOverLineFilePrefix,
    const std::vector<float>& values_on_line,
    const std::vector<int>& mask,
    const std::vector<float>& x_axis
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

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".dat";
    std::string plotOverLineFilePath;
    plotOverLineFilePath = plotOverLineFilePrefix + ss.str();

    vismodule::ValueArray<float> tmp_values_on_line( values_on_line );
    vismodule::ValueArray<float> tmp_x_axis( x_axis );

    // convert mask's data from int to bool
    // MPI does not support bool type so use int type
    // vismodule::KVSMLObjectPlotOverLine::m_mask is defined as bool type
    vismodule::ValueArray<bool> tmp_mask( mask.size() );
    for ( size_t i = 0; i < mask.size(); i++ )
    {
        if ( mask[i] != 0 ) tmp_mask[i] = true;
    }

    vismodule::KVSMLObjectPlotOverLine pol_object( tmp_values_on_line, tmp_x_axis, tmp_mask );
    pol_object.write( plotOverLineFilePath.c_str() );
}
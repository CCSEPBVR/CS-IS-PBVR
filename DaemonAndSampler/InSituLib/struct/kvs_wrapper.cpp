#include <fstream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <iomanip>
#include <kvs/Vector3>
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/StructuredVolumeObject>
//#include <kvs/TrilinearInterpolator>
#include <kvs/PointObject>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>
//#include <kvs/CellByCellParticleGenerator>
#include <kvs/TransferFunction>
#include <kvs/RGBColor>
#include <kvs/Timer>
#include <kvs/MersenneTwister>
#include "kvs_wrapper.h"
#include "TFS/CellByCellParticleGenerator.h"
#include "TFS/TransferFunctionSynthesizer.h"
#include "TFS/ParamInfo.h"
#include "TFS/TrilinearInterpolator.h"
//#include "TFS/TransferFunctionSynthesizerCreator.h"
#include "float.h"

#include <mpi.h>

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP



// Asynchronous io, using worker thread pwt.
#include "particle_write_thread.h"
bool async_io_enabled=false;
kvs::ValueArray<float> O_min_recv;
kvs::ValueArray<float> O_max_recv;
kvs::ValueArray<float> C_min_recv;
kvs::ValueArray<float> C_max_recv;
kvs::ValueArray<int> o_histogram_recv;
kvs::ValueArray<int> c_histogram_recv;

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


namespace Generator = pbvr::CellByCellParticleGenerator;


//***** kvs::CellByCellParticleGenerator *****//
float GetRandomNumber()
{

#ifdef KMATH
    double rv;
#pragma omp critical(random)
    {
        km_random.get( rv );
    }
    return static_cast<kvs::Real32>( rv - 1.0 );
#else
    // xorshift RGNs with period at least 2^128 - 1.
//    static float t24 = 1.0/16777216.0; /* 0.5**24 */
    static kvs::UInt32 x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    kvs::UInt32 t;
    t = ( x ^ ( x << 11 ) );
    x = y;
    y = z;
    z = w;
    w = ( w ^ ( w >> 19 ) ) ^ ( t ^ ( t >> 8 ) );

    //return 0;
    return w * ( 1.0f / 4294967296.0f ); // = w * ( 1.0f / kvs::Value<kvs::UInt32>::Max() + 1 )
//    return t24 * static_cast<float>( w >> 8 );
#endif
}

namespace
{

kvs::Vector3f RandomSamplingInCubeKMATH( const kvs::Vector3f vertex )
{
    const float x = GetRandomNumber();
    const float y = GetRandomNumber();
    const float z = GetRandomNumber();
    const kvs::Vector3f d( x, y, z );

    return vertex + d;
}

kvs::Vector3f RandomSamplingInCube( const kvs::Vector3f vertex, kvs::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();
    const kvs::Vector3f d( x, y, z );

    return vertex + d;
}

inline size_t CalculateSubpixelLevel( const int particle_limit,
                                      const kvs::Camera& camera,
                                      const float sampling_step,
                                      const double total_volume,
                                      const kvs::ObjectBase* volume )
{
    namespace Generator = pbvr::CellByCellParticleGenerator;
    double d_nparticles = 0.0;//particle density for subpixel_level=1
    d_nparticles = Generator::CalculateGreatDensity( &camera, volume, 1,
                                                      sampling_step ) * total_volume;

    //Calculation of optimized subpixel level
    float plimit = static_cast<float>( particle_limit );
    float nparticles = static_cast<float>( d_nparticles );
    float subpixel_level = sqrt( plimit / nparticles );

    if ( subpixel_level < 1 ) subpixel_level = 1;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

bool LoadParameterFile( ParamInfo*  param_info,
                        const std::string& filename,
                        const std::string& old_filename )
{
    ParamInfo& param = (*param_info);
    bool opend;
    int size = 0;
    char* buf;

    int mpi_rank;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    if( mpi_rank == 0 )
    {
        bool opened;

        std::ifstream fin( filename.c_str(), std::ios::in );
        opened = fin.is_open();

        if( opened )
        { 
            std::cout << "load parameter file" << std::endl;
            /*ãã¡ã¤ã«ãä¸å®å¨ã®å ´åãå®å¨ã«ãªãã¾ã§ä¸æåæ­¢ããã*/
            param.LoadIN( filename );
            while( param.getString( "END_PARAMETER_FILE" ) != "SUCCESS" )
            {
                param.LoadIN( filename );
            }
            size = param.byteSize();
            if( size > 0 )
            {
                buf = new char [size];
                param.pack( buf );
            }
        }
        else
        {
            size = 0;
        }
    }

    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
    if( size > 0 )
    {
        if( mpi_rank > 0 ) buf = new char [size];
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
        param.unpack( buf );
        delete[] buf;

        if( mpi_rank == 0 )
        {
            std::rename( filename.c_str(), old_filename.c_str() );
        }
        opend = true;
    }
    else
    {
        opend = false;
    }

    return opend;
}

void readTFfromParamInfo( ParamInfo* param,
                          std::vector<pbvr::TransferFunction>& tf,
                          TransferFunctionSynthesizer* tfs )
{
    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var;
    int tf_number;

    i_table = param->getTableInt( "OPA_FUNC_EXP_TOKEN" );
    if (i_table.size() < 128){
        std::cerr<<"Error retrieving TF from ParamInfo"<<std::endl<<
        "If you are trying to overwrite an existing job you may need to execute RESET.sh first."<<std::endl;
        exit(1);
    }
    for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

    i_table = param->getTableInt( "OPA_FUNC_VAR_NAME" );
    for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

    f_table = param->getTableFloat( "OPA_FUNC_VAL_ARRAY" );
    for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

    tfs->setOpacityFunction( eq );

    i_table = param->getTableInt( "COL_FUNC_EXP_TOKEN" );
    for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

    i_table = param->getTableInt( "COL_FUNC_VAR_NAME" );
    for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

    f_table = param->getTableFloat( "COL_FUNC_VAL_ARRAY" );
    for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

    tfs->setColorFunction( eq );

    // get TF_NUMBER
    tf_number = param->getInt( "TF_NUMBER" );

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        i_table = param->getTableInt( tag_base + "O_EXP_TOKEN" );
        for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

        i_table = param->getTableInt( tag_base + "O_VAR_NAME" );
        for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

        f_table = param->getTableFloat( tag_base + "O_VAL_ARRAY" );
        for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

        var.push_back( eq );
    }

    tfs->setOpacityVariable( var );

    var.clear();
    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        i_table = param->getTableInt( tag_base + "C_EXP_TOKEN" );
        for(size_t i=0; i<128; i++) eq.exp_token[i] = i_table[i];

        i_table = param->getTableInt( tag_base + "C_VAR_NAME" );
        for(size_t i=0; i<128; i++) eq.var_name[i] = i_table[i];

        f_table = param->getTableFloat( tag_base + "C_VAL_ARRAY" );
        for(size_t i=0; i<128; i++) eq.val_array[i] = f_table[i];

        var.push_back( eq );
    }

    tfs->setColorVariable( var );

    //Read 1D tf
    int resolution = param->getInt( "TF_RESOLUTION" );
    float min, max;

    tf.clear();

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        min = param->getFloat( tag_base +"MIN_C" );
        max = param->getFloat( tag_base +"MAX_C" );
        i_table = param->getTableInt( tag_base + "TABLE_C" );
        kvs::ValueArray<kvs::UInt8> u_table( i_table.size() );
        for( size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (kvs::UInt8)i_table[j];
        kvs::ColorMap color_map( u_table, min, max );

        min = param->getFloat( tag_base +"MIN_O" );
        max = param->getFloat( tag_base +"MAX_O" );
        f_table = param->getTableFloat( tag_base + "TABLE_O" );
        kvs::ValueArray<float> ff_table( f_table );
        kvs::OpacityMap opacity_map( ff_table, min, max );

        pbvr::TransferFunction tfBuf;
        tfBuf.setColorMap( color_map );
        tfBuf.setOpacityMap( opacity_map );
        tf.push_back(tfBuf);
    }
}

bool initializeParameters(
    TransferFunctionSynthesizer* tfs,
    std::vector<pbvr::TransferFunction>& tf,
    ParamInfo *param_info,
    const kvs::ObjectBase* object,
    float* sampling_volume_inverse,
    float* max_opacity, float* max_density, int* subpixel_level, float* particle_density,
    float* particle_data_size_limit,
    const std::string &visParamDir,
    const std::string &tfFilename )
{
    //std::cout<<"param.LoadIN()\n";
    ParamInfo& param = (*param_info);
    static bool start_flag = true;
    bool opend;

#if 0
    std::string param_filename     = "jupiter.tf";
    std::string old_param_filename = "jupiter_old.tf";
#else 
    // 20181226 start 環境変数で指定したパスを使用
    std::string param_filename     = visParamDir + tfFilename + ".tf";
    std::string old_param_filename = visParamDir + tfFilename + "_old.tf";
    // 20181226 end
#endif

    opend = LoadParameterFile( param_info, param_filename, old_param_filename );

    *particle_density         = param.getFloat( "PARTICLE_DENSITY" );
    *particle_data_size_limit = param.getFloat( "PARTICLE_DATA_SIZE_LIMIT" );


    //2019 kawamura
    readTFfromParamInfo( param_info, tf, tfs );


    //std::cout<<"camera\n";
    kvs::Camera camera;
    int height = param.getInt( "RESOLUTION_HEIGHT" );
    int width  = param.getInt( "RESOLUTION_WIDTH" );
    camera.setWindowSize( height,width );
    float min = 0;
    float max = kvs::Math::Max( object->maxObjectCoord().x(),
                                object->maxObjectCoord().y(),
                                object->maxObjectCoord().z() );
    const float sampling_step = (max - min) / 1E1;
    int particle_limit = param.getInt( "PARTICLE_LIMIT" );
    double total_volume =
          ( object->maxObjectCoord().x() - object->minObjectCoord().x() )
        * ( object->maxObjectCoord().y() - object->minObjectCoord().y() )
        * ( object->maxObjectCoord().z() - object->minObjectCoord().z() );


    *max_opacity = 0.98;
    *subpixel_level = CalculateSubpixelLevel( particle_limit , camera, sampling_step, total_volume, object );

    //std::cout<<"Generator::\n";
    Generator::CalculateDensityParameters(
        &camera,
        object,
        (float)(*subpixel_level),
        sampling_step,
        *max_opacity,
        sampling_volume_inverse,
        max_density );

    int mpi_rank;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    if( mpi_rank == 0 )
    {
        fprintf( stdout , "---------initialize Parameters-------------\n" );
        fprintf( stdout , "particle_limit    = %20d\n", particle_limit );
        fprintf( stdout , "particle_density  = %20f\n", *particle_density );
        fprintf( stdout , "resolutin_height  = %20d\n", height );
        fprintf( stdout , "resolutin_width   = %20d\n", width );
        fprintf( stdout , "total_volume      = %20.3e\n", total_volume );
        fprintf( stdout , "  |-X             = %20f\n", object->maxObjectCoord().x() );
        fprintf( stdout , "  |-Y             = %20f\n", object->maxObjectCoord().x() );
        fprintf( stdout , "  |-Z             = %20f\n", object->maxObjectCoord().x() );
        fprintf( stdout , "max_opacity       = %20.3e\n", *max_opacity );
        fprintf( stdout , "max_density       = %20.3e\n", *max_density );
        fprintf( stdout , "sampling_step     = %20.3e\n", sampling_step );
        fprintf( stdout , "subpixel_level    = %20d\n", *subpixel_level );
        fprintf( stdout , "------------------------------------\n" );
    }

    return opend;
}

void calculate_histogram( kvs::ValueArray<int>&   th_o_histogram,
                          kvs::ValueArray<int>&   th_c_histogram,
                          kvs::ValueArray<float>& th_O_min,
                          kvs::ValueArray<float>& th_O_max,
                          kvs::ValueArray<float>& th_C_min,
                          kvs::ValueArray<float>& th_C_max,
                          // ここまでoutput, 以下input
                          const int nbins, // TFSから読み込む最大最小値
                          const kvs::ValueArray<float>& o_min,
                          const kvs::ValueArray<float>& o_max,
                          const kvs::ValueArray<float>& c_min,
                          const kvs::ValueArray<float>& c_max,
                          const float o_scalars[][SIMDW], // åæå¤
                          const float c_scalars[][SIMDW],
                          const int tf_number  )
{
    //ヒストグラムと最大最小値
    for( int i = 0; i < tf_number; i++ )
    {
        for( int I = 0; I < SIMDW; I++ )
        {
            //不透明度のヒストグラム
            float h = (o_scalars[i][I] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
            int H = (int)h;
            if( 0 <= H && H <= nbins )
            {
                if( H == nbins ) H--;
                th_o_histogram[ H + nbins*i]++;
            }

            //色のヒストグラム
            h = (c_scalars[i][I] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
            H = (int)h;
            if( 0 <= H && H <= nbins )
            {
                if( H == nbins ) H--;
                th_c_histogram[ H + nbins*i]++;
            }

            //不透明度の最大最小値
            th_O_min[i] = th_O_min[i] < o_scalars[i][I] ? th_O_min[i] : o_scalars[i][I];
            th_O_max[i] = th_O_max[i] > o_scalars[i][I] ? th_O_max[i] : o_scalars[i][I];
            //色の最大最小値
            th_C_min[i] = th_C_min[i] < c_scalars[i][I] ? th_C_min[i] : c_scalars[i][I];
            th_C_max[i] = th_C_max[i] > c_scalars[i][I] ? th_C_max[i] : c_scalars[i][I];
        }
    }
}

const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    kvs::MersenneTwister* MT )
{
    const float N = density * volume_of_cell;
    //const float R = GetRandomNumber();//KMATH使用
    const float R = (float)MT->rand();

    size_t n = static_cast<size_t>( N );
    if ( N - n > R )
    {
        ++n;
    }

    return ( n );
}

void show_timer( time_parameters time )
{
    double initialize;
    double sampling;
    double writing;
    double mpi_reduce;
    double write_text;
    int    nparticles;

    double time_total =
        time.initialize + time.sampling + time.writting + time.mpi_reduce + time.write_text;
    double total;

    MPI_Reduce( &(time.initialize), &initialize, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.sampling),   &sampling,   1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.writting),   &writing,   1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.mpi_reduce), &mpi_reduce, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.write_text), &write_text, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time_total),      &total,      1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.nparticles), &nparticles, 1, MPI_INTEGER,MPI_SUM, 0, MPI_COMM_WORLD );


    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    if( mpi_rank == 0 )
    {
        printf("initialize= %8.3e [sec/step]\n", initialize);
        printf("sampling  = %8.3e [sec/step]\n", sampling  );
        printf("writing   = %8.3e [sec/step]\n", writing  );
        printf("mpi_reduce= %8.3e [sec/step]\n", mpi_reduce);
        printf("write_text= %8.3e [sec/step]\n", write_text);
        printf("total     = %8.3e [sec/step]\n", total     );
        printf("nparticles= %d\n", nparticles );
    }
}
}//end of unnamed namespace


void generate_particles( int time_step,
                         domain_parameters dom,
                         Type** volume_data,
                         int num_volume_data )
{

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank;
    int mpi_size;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    //if(mpi->rank==0)std::cout<<"start generate_particles\n";
    static bool start_flag = true;
    static bool parameter_file_opened=false;
    kvs::Timer timer( kvs::Timer::Start );


    const int nvariables = num_volume_data;

    const kvs::Vector3ui resolution( dom.resolution[0], dom.resolution[1], dom.resolution[2] );

    //std::vector< kvs::StructuredVolumeObject* > vol_obj;
    std::vector< std::vector< TFS::TrilinearInterpolator* > >  interp;
/*
    vol_obj.resize( num_volume_data );
    for ( int i = 0; i < num_volume_data; i++ )
    {
         const kvs::ValueArray<Type> v( volume_data[i], resolution.x()*resolution.y()*resolution.z() );
         const kvs::AnyValueArray av( v );
         vol_obj[i] = new kvs::StructuredVolumeObject( resolution, 1, av );
    }
*/
    interp.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        interp[ i ].resize( num_volume_data );
        for ( int j = 0; j < num_volume_data; j++ )
        {
             interp[i][j]  = new TFS::TrilinearInterpolator( volume_data[j], resolution );
        }
    }
    // Set Transfer function synthesizer.
    TransferFunctionSynthesizer* tfs = new TransferFunctionSynthesizer();
    std::vector<pbvr::TransferFunction> tf;
    static ParamInfo param;
    float sampling_volume_inverse;
    float max_opacity;
    float max_density;
    int subpixel_level;
    float particle_density;
    float particle_data_size_limit;

    // 20181226 start  環境変数で指定したファイルパスを参照する
    std::string visParamDir;
    std::string tfFilename;
    std::string stateFilePath;
    std::string minmaxFilePath;
    std::string ptcFilePath;

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
    stateFilePath = visParamDir + "state.txt";
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        minmaxFilePath = "./t_pfi_coords_minmax.txt";
        ptcFilePath = "./t_";
    }
    else {
        minmaxFilePath = envBuf;
        ptcFilePath = envBuf;
        if (minmaxFilePath[minmaxFilePath.size() - 1] != '/') {
            minmaxFilePath += "/t_pfi_coords_minmax.txt";
            ptcFilePath += "/t_";
        }
        else {
            minmaxFilePath += "t_pfi_coords_minmax.txt";
            ptcFilePath += "t_";
        }
    }
    // 20181226 end

    // 20190318 ボリュームサイズのファイル出力
    //全体の最大最小値を示すpfiファイルを生成
    static bool minmaxFlag = false;
    if (minmaxFlag == false && mpi_rank == 0) {
        FILE* fp = fopen( minmaxFilePath.c_str(), "w" );
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
    // 20190318 end

    //粒子サイズを決めるためのGlobal Min Max volume object
    kvs::StructuredVolumeObject* object = new kvs::StructuredVolumeObject();
    kvs::Vector3f min_vec(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min);
    kvs::Vector3f max_vec(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max );

    object->setMinMaxObjectCoords( min_vec, max_vec );
    object->setMinMaxExternalCoords( min_vec, max_vec );

    if(mpi_rank==0)std::cout<<"start initializeTFS()\n";

    bool tmp_parameter_file_opened = 
        initializeParameters( tfs, tf, &param, object,
                              &sampling_volume_inverse, &max_opacity, &max_density,
                              &subpixel_level, &particle_density, &particle_data_size_limit,
                              visParamDir, tfFilename );

    int tf_number = tf.size();

/*
    for ( int j = 0; j < TF_COUNT; j++ )
    {
        std::cout<<"tf:min="<< tf[j].opacityMap().minValue() <<",max="<<tf[j].opacityMap().maxValue()<<std::endl;
        for( int i=0; i<tf[j].opacityMap().table().size();i++){
            std::cout<< tf[j].opacityMap().table()[i]<<",";
        }std::cout<<std::endl;
    }
*/
    if( start_flag ) parameter_file_opened = tmp_parameter_file_opened;

    delete object;
    //if(mpi->rank==0)std::cout<<"end initializeTFS()\n";

    // Calculate maximum number (upper limit) of particles for each cell.
    const int max_nparticles = (int)max_density + 1;
    if(mpi_rank==0) std::cout<<"******* max_nparticles="<<max_nparticles<<std::endl;

    //ヒストグラム
    int nbins = 256;
    kvs::ValueArray<float> o_min( tf_number );//TFSから読み込む最大最小値
    kvs::ValueArray<float> o_max( tf_number );
    kvs::ValueArray<float> c_min( tf_number );
    kvs::ValueArray<float> c_max( tf_number );

    kvs::ValueArray<int> o_histogram( tf_number * nbins );//不透明度ヒストグラムの配列
    kvs::ValueArray<int> c_histogram( tf_number * nbins );//色ヒストグラムの配列

    if( parameter_file_opened )
    {
        O_min_recv.allocate(tf_number);
        O_max_recv.allocate(tf_number);
        C_min_recv.allocate(tf_number);
        C_max_recv.allocate(tf_number);
        o_histogram_recv.allocate(tf_number * nbins);
        c_histogram_recv.allocate(tf_number * nbins);

        o_histogram.fill(0x00);
        c_histogram.fill(0x00);
    }

    for( size_t i = 0; i < tf_number; i++ )
    {
        o_min[i] = tf[i].opacityMap().minValue();
        o_max[i] = tf[i].opacityMap().maxValue();
        c_min[i] = tf[i].colorMap().minValue();
        c_max[i] = tf[i].colorMap().maxValue();
    }

    //最大最小値
    kvs::ValueArray<float> O_min( tf_number );//計算して得る最大最小値
    kvs::ValueArray<float> O_max( tf_number );
    kvs::ValueArray<float> C_min( tf_number );
    kvs::ValueArray<float> C_max( tf_number );

    // 動的な粒子データ配列
    std::vector<float> vertex_coords;
    std::vector<Byte>  vertex_colors;
    std::vector<float> vertex_normals;

    if( parameter_file_opened )
    {
        for ( size_t i = 0; i < tf_number; i++ ) //初期化
        {
            O_min[ i ] =  FLT_MAX;
            O_max[ i ] = -FLT_MAX;
            C_min[ i ] =  FLT_MAX;
            C_max[ i ] = -FLT_MAX;
        }
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
    std::vector< std::vector<pbvr::TransferFunction> > th_tf;
    TFS::TrilinearInterpolator** interp_opacity  = new TFS::TrilinearInterpolator*[max_threads] ;

    for ( int n = 0; n < max_threads; n++ )
    {
        th_tfs[n] = new TransferFunctionSynthesizer( *tfs );
    }

    th_tf.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[ i ].resize( tf_number );
        for ( int j = 0; j < tf_number; j++ )
        {
            th_tf[i][j] = tf[j];
/*
            std::cout<<"tf["<<i<<","<<j<<"]:min="
                     << th_tf[i][j].opacityMap().minValue()
                     <<",max="
                     << th_tf[i][j].opacityMap().maxValue()
                     <<std::endl;
*/
        }
    }
/*
    int total_nparticles = 0;
    int particles_process_limit = static_cast<int> (  ( particle_data_size_limit * 10E6 )
                                                    / ( sizeof( float ) + sizeof( Byte ) + sizeof( float ) ) );
    bool particle_limit_over = false;
*/
    float* opacity_volume = new float[ resolution.x()*resolution.y()*resolution.z() ];
    //kvs::ValueArray<Type> ov( resolution.x()*resolution.y()*resolution.z() );
    //const kvs::AnyValueArray aov( ov );
    //kvs::StructuredVolumeObject* volume_opacity = new kvs::StructuredVolumeObject( resolution, 1, aov );

    time_parameters time;
    timer.stop();
    time.initialize = timer.sec();
    timer.start();

    //頂点解像度と格子解像度
    const int nx = resolution.x();
    const int ny = resolution.y();
    const int nz = resolution.z();
    const int nxy = nx * ny;
    const int nx_1 = nx-1;
    const int ny_1 = ny-1;
    const int nz_1 = nz-1;
    const int nxy_1 = nx_1 * ny_1;

    int total_nparticles = 0;

    static TimedScope td_gatherf("GatherF",1);
    static TimedScope td_gather("gather",1);
    static TimedScope td_kvsml("kvsml",1);
    static TimedScope td_SynthOpacityScalars("SynthesizedOpacityScalars",max_threads);
    static TimedScope td_SynthColorScalars("SynthesizedColorScalars",max_threads);
    static TimedScope td_CalculateHistogram("CalculateHistogram",max_threads);
    static TimedScope td_CalculateOpacity("CalculateOpacity",max_threads);
    static TimedScope td_CalculateDensity("CalculateDensity",max_threads);
    static TimedScope td_CalculateNumPar("Ccalculate_number_of_particles (Random)",max_threads);
    static TimedScope td_CalculateColor("CalculateColor",max_threads);
    static  TimedScope td_VectorPush("Vector Push",max_threads);
    static TimedScope td_VectorIns("Vector Insert",max_threads);
    #pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif



        //th_tfs[thid]->set_debug_thid(thid,max_threads);

        int th_total_nparticles = 0;
        //各スレッド番号をシードにした乱数生成器
        kvs::MersenneTwister MT( thid + mpi_rank * nthreads );

        // 動的な粒子データ配列
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;

        //ヒストグラムの配列
        float o_scalars[tf_number][SIMDW];//頂点の不透明度
        float c_scalars[tf_number][SIMDW];//頂点の色

        kvs::ValueArray<int> th_o_histogram( tf_number * nbins );//不透明度
        kvs::ValueArray<int> th_c_histogram( tf_number * nbins );//色

        if( parameter_file_opened )
        {
            th_o_histogram.fill(0x00);
            th_c_histogram.fill(0x00);
        }

        //最大最小値
        kvs::ValueArray<float> th_O_min( tf_number );//計算して得る最大最小値
        kvs::ValueArray<float> th_O_max( tf_number );
        kvs::ValueArray<float> th_C_min( tf_number );
        kvs::ValueArray<float> th_C_max( tf_number );

        if( parameter_file_opened )
        {
            for ( int i = 0; i < tf_number; i++ ) //初期化
            {
                th_O_min[ i ] =  FLT_MAX;
                th_O_max[ i ] = -FLT_MAX;
                th_C_min[ i ] =  FLT_MAX;
                th_C_max[ i ] = -FLT_MAX;
            }
        }

        //-----------------------------------------//
        //----------------Histogram----------------//
        //-----------------------------------------//
        {
            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ?
                nvertices / SIMDW : nvertices / SIMDW + 1;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    // vertex_id = i + j * nx + k * nx * ny
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * dom.cell_length)+dom.x_min;
                    const float y_g = (y_l * dom.cell_length)+dom.y_min;
                    const float z_g = (z_l * dom.cell_length)+dom.z_min;

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }
                timed_section_start(td_SynthOpacityScalars,thid);
                th_tfs[thid]->SynthesizedOpacityScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, o_scalars );
                timed_section_end(td_SynthOpacityScalars,thid);
                timed_section_start(td_SynthColorScalars,thid);
                th_tfs[thid]->SynthesizedColorScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, c_scalars );
                timed_section_end(td_SynthColorScalars,thid);
                timed_section_start(td_CalculateHistogram,thid);
                calculate_histogram( th_o_histogram, th_c_histogram,
                                     th_O_min, th_O_max, th_C_min, th_C_max,
                                     nbins,
                                     o_min, o_max, c_min, c_max,
                                     o_scalars, c_scalars,
                                     tf_number );
                timed_section_end(td_CalculateHistogram,thid);

            }
        } // end of Histogram

        //-----------------------------------------//
        //--------------粒子生成ループ開始------------//
        //------------------------------------------//
        //不透明度ボリュームの生成
        {
            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ?
                nvertices / SIMDW : nvertices / SIMDW + 1;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];//interp用の相対座標
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];//TFS用の全体座標
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    // vertex_id = i + j * nx + k * nx * ny
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * dom.cell_length)+dom.x_min;
                    const float y_g = (y_l * dom.cell_length)+dom.y_min;
                    const float z_g = (z_l * dom.cell_length)+dom.z_min;

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }

                float vertex_opacity[SIMDW];

                timed_section_start(td_CalculateOpacity,thid);
                th_tfs[thid]->CalculateOpacity( interp[thid], th_tf[thid],
                                                X_l, Y_l, Z_l, X_g, Y_g, Z_g, vertex_opacity );
                timed_section_end(td_CalculateOpacity,thid);

#pragma ivdep
                for( int I = 0; I <SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    if( vertex_id < nvertices )
                    {
                        //ov[ vertex_id ] = vertex_opacity[I];
                        opacity_volume[ vertex_id ] = vertex_opacity[I];
                    }
                }
            }
        }// end of 不透明度ボリュームの生成

#pragma omp critical
        {
            interp_opacity[thid] = new TFS::TrilinearInterpolator( opacity_volume, resolution );
        }

        //粒子生成ループ開始
        {
            // Marge x-y-z loop
            const int ncells = nx_1 * ny_1 * nz_1;
            // "+ 1" means remained loop
            const int outer_loop = (ncells % SIMDW == 0) ?
                ncells / SIMDW : ncells / SIMDW + 1;

            #pragma omp for schedule(dynamic)
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];//interp用の相対座標
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];//TFS用の全体座標
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int cell_id = I + J * SIMDW;
                    const int k =  cell_id / nxy_1;
                    const int j = (cell_id - k * nxy_1) / nx_1;
                    const int i =  cell_id - k * nxy_1 - j * nx_1;

                    const float x_l = (float)i + 0.5;
                    const float y_l = (float)j + 0.5;
                    const float z_l = (float)k + 0.5;
                    const float x_g = (x_l * dom.cell_length)+dom.x_min;
                    const float y_g = (y_l * dom.cell_length)+dom.y_min;
                    const float z_g = (z_l * dom.cell_length)+dom.z_min;

                    X_l[I] =  x_l;
                    Y_l[I] =  y_l;
                    Z_l[I] =  z_l;
                    X_g[I] =  x_g;
                    Y_g[I] =  y_g;
                    Z_g[I] =  z_g;
                }

                float cell_opacity[SIMDW];
                timed_section_start(td_CalculateOpacity,thid);
                th_tfs[thid]->CalculateOpacity( interp[thid], th_tf[thid],
                                                X_l, Y_l, Z_l, X_g, Y_g, Z_g, cell_opacity );
                timed_section_end(td_CalculateOpacity,thid);

                int nparticles[SIMDW];
                #pragma ivdep
                for( int I=0; I<SIMDW; I++)
                {
                     timed_section_start(td_CalculateDensity,thid);
                    const float density = Generator::CalculateDensity( cell_opacity[I],
                                                                       sampling_volume_inverse,
                                                                       max_opacity, max_density );


                    timed_section_end(td_CalculateDensity,thid);
                    timed_section_start(td_CalculateNumPar,thid);

                    const int np = calculate_number_of_particles( density, 1, &MT ) * particle_density;
                    timed_section_end(td_CalculateNumPar,thid);

                    const int cell_id = I + J * SIMDW;

                    nparticles[I] = cell_id < ncells ? np : 0;
                    th_total_nparticles += nparticles[I];
                }

                // 乱数生成はSIMD化できない
                // 粒子位置を逐次計算
                int p_id = 0;
                float p_x_l[SIMDW], p_y_l[SIMDW], p_z_l[SIMDW];
                float p_x_g[SIMDW], p_y_g[SIMDW], p_z_g[SIMDW];
                float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];
                kvs::UInt8 red[SIMDW], green[SIMDW], blue[SIMDW];
                float particle_opacity[SIMDW];
                // the last loop "I==SIMDW" is used for occupy ramained array.
                for(int I=0; I<SIMDW+1; I++)
                {
                    const int cell_id = I + J * SIMDW;
                    const int k =  cell_id / nxy_1;
                    const int j = (cell_id - k * nxy_1) / nx_1;
                    const int i =  cell_id - k * nxy_1 - j * nx_1;

                    const int nparticles_I  = I<SIMDW ? nparticles[I] : SIMDW - p_id;
                    const int zero_id = I<SIMDW ? SIMDW : p_id;
                    for(int p=0; p < nparticles_I; p++)
                    {
                        const kvs::Vector3f vertex( (float)i, (float)j, (float)k );
                        const kvs::Vector3f coord_l( RandomSamplingInCube( vertex, &MT ) );
                        const kvs::Vector3f coord_g(
                            (coord_l.x()*dom.cell_length)+dom.x_min,
                            (coord_l.y()*dom.cell_length)+dom.y_min,
                            (coord_l.z()*dom.cell_length)+dom.z_min );
                        p_x_l[ p_id ] = coord_l.x();
                        p_y_l[ p_id ] = coord_l.y();
                        p_z_l[ p_id ] = coord_l.z();
                        p_x_g[ p_id ] = coord_g.x();
                        p_y_g[ p_id ] = coord_g.y();
                        p_z_g[ p_id ] = coord_g.z();
                        p_id++;

                        if( p_id == SIMDW )
                        {
                            p_id = 0;

                            timed_section_start(td_CalculateOpacity,thid);
                            th_tfs[thid]->CalculateOpacity( interp[thid], th_tf[thid],
                                                            p_x_l, p_y_l, p_z_l,
                                                            p_x_g, p_y_g, p_z_g,
                                                            particle_opacity );
                            timed_section_end(td_CalculateOpacity,thid); 
                            interp_opacity[thid]->attachPoint( p_x_l, p_y_l, p_z_l );
                            interp_opacity[thid]->gradient( grad_x, grad_y, grad_z );
                            timed_section_start(td_CalculateColor,thid);
                            th_tfs[thid]->CalculateColor( interp[thid], th_tf[thid],
                                                          p_x_l, p_y_l, p_z_l,
                                                          p_x_g, p_y_g, p_z_g,
                                                          red, green, blue );
                            timed_section_end(td_CalculateColor,thid); 
                            //SIMDループ
                            for( int pp=0; pp<SIMDW; pp++)
                            {
                                timed_section_start(td_CalculateDensity,thid);
                                const float density =
                                    pp < zero_id ?
                                         Generator::CalculateDensity( particle_opacity[pp],
                                                                      sampling_volume_inverse,
                                                                      max_opacity, max_density ) : 0;

                                timed_section_end(td_CalculateDensity,thid);
                                //棄却法
                                const float random = (float)MT();
                                if( density > max_density * random )
                                {
                                    timed_section_start(td_VectorPush,thid);
                                    th_vertex_coords.push_back( p_x_g[pp] );
                                    th_vertex_coords.push_back( p_y_g[pp] );
                                    th_vertex_coords.push_back( p_z_g[pp] );

                                    th_vertex_colors.push_back( red  [pp] );
                                    th_vertex_colors.push_back( green[pp] );
                                    th_vertex_colors.push_back( blue [pp] );

                                    th_vertex_normals.push_back( grad_x[pp] );
                                    th_vertex_normals.push_back( grad_y[pp] );
                                    th_vertex_normals.push_back( grad_z[pp] );
                                    timed_section_end(td_VectorPush,thid);
                                } // end of if pp
                            } // end of for pp
                        } // end of if p_id
                    } // end of for p
                } // end of for I 粒子位置を逐次計算
            } // end of omp for J outer_loop
        } // end of 粒子生成ループ

        timed_section_start(td_VectorIns,thid);
        #pragma omp critical
        {
            if( parameter_file_opened )
            {
                //最大最小値
                for( int i = 0; i < tf_number; i++ )
                {
                    //不透明度
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    //色
                    C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                    C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];

                }

                for( int n = 0; n < tf_number * nbins; n++ )
                {
                    o_histogram[n] += th_o_histogram[n];
                    c_histogram[n] += th_c_histogram[n];
                }
            }

            total_nparticles += th_total_nparticles;
            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );

            delete interp_opacity[thid];

        } // end of omp critical
        timed_section_end(td_VectorIns,thid);
    } // end of omp parallel


    /*
    total_nparticles = vertex_coords.size()/3;
    if( total_nparticles > particles_process_limit )
        std::cerr  << "[ Warning!! ] Particle Limit is Over. rank = " << mpi->rank
                   << ", Particles = " << total_nparticles << "/"<< particles_process_limit << std::endl;
    */
    //if(mpi->rank == 0) std::cout<<"total_nparticles="<<total_nparticles<<std::endl;
    std::cout<<"rank="<<mpi_rank<<",total_nparticles="<<total_nparticles<<std::endl;

    timer.stop();
    time.sampling = timer.sec();
    time.nparticles = vertex_coords.size()/3;
    timer.start();

    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;

    delete tfs;

    delete[] opacity_volume;
/*
    for ( int i = 0; i < num_volume_data; i++ )
    {
         delete vol_obj[i];
    }
*/
    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < num_volume_data; j++ )
        {
             delete interp[i][j];
        }
    }
    //delete volume_opacity;
    delete[] interp_opacity;

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    kvs::ValueArray<float> coords( vertex_coords );
    kvs::ValueArray<Byte>  colors( vertex_colors );
    kvs::ValueArray<float> normals( vertex_normals );

    timed_section_start(td_gatherf);

    static bool first_step = true;
    static MPI_Comm new_comm;
    static int count;
    static int num_nodes;

    /* 各ノード毎に粒子データを出力する。 */
    if( first_step )
    {
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
        first_step = false;
    }
    timed_section_end(td_gatherf);
    /*
     * ファイル名の粒子データのファイル名を入力する.
     * countが各ファイルで連続でない場合,ファイルが不在と見なしてデーモンでスピンロックがかかる.
     */
#if 0
    char filename[256];
    sprintf(filename, "./jupiter_particle_out/t_%05d_",time_step);
    sprintf(filename,"%s%07d_%07d.kvsml", filename, count, num_nodes );
#else
    // 20181226 start  環境変数で指定したファイルパスを参照する
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
    ptcFilePath += ss.str();
    // 20181226 end
#endif
    timed_section_start(td_gather);
    int particle_size = coords.size();
    int *recvcounts;
    int *displs;
    int  new_number_of_process;
    int new_rank;

    MPI_Comm_rank( new_comm, &new_rank );
    MPI_Comm_size( new_comm, &new_number_of_process );

    /*
     *  recvcounts: 各ランク毎の受信バッファサイズ.
     *  displs:     受信先バッファ上の各ランク毎の受信バッファの位置(オフセット)
     */

    displs = new int[ new_number_of_process ];
    recvcounts = new int[ new_number_of_process ];

    MPI_Allgather( &particle_size, 1, MPI_INT,
                   recvcounts,     1, MPI_INT,
                   new_comm );
    displs[0] = 0;
    for( int i =1; i< new_number_of_process; i++ )
        displs[i] = displs[i-1] + recvcounts[i-1];

    kvs::ValueArray<float> new_coords(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    kvs::ValueArray<Byte>  new_colors(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    kvs::ValueArray<float> new_normals( displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );

    MPI_Gatherv( coords.pointer(),   particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );

    MPI_Gatherv( colors.pointer(),   particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, new_comm );

    MPI_Gatherv( normals.pointer(),   particle_size, MPI_FLOAT,
                 new_normals.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );

    timed_section_end(td_gatherf);
    /*  分割後コミュニケータのランク0で出力する  */
    timed_section_start(td_kvsml);
    if( new_rank == 0 )
    {
        kvs::PointObject* point_object = new kvs::PointObject( new_coords, new_colors, new_normals, subpixel_level );
        point_object->setMinMaxObjectCoords( min_vec, max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
            particle_write_thread->join(true);
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename(ptcFilePath.c_str());
            particle_write_thread->setTimestep(time_step , stateFilePath.c_str());
            particle_write_thread->work(true);
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( point_object );
            kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( ptcFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }
    timed_section_end(td_kvsml);

    timer.stop();
    time.writting = timer.sec();
    timer.start();
    /*
    kvs::ValueArray<float> O_min_recv( tf_number );
    kvs::ValueArray<float> O_max_recv( tf_number );
    kvs::ValueArray<float> C_min_recv( tf_number );
    kvs::ValueArray<float> C_max_recv( tf_number );
    kvs::ValueArray<int> o_histogram_recv( tf_number * nbins );
    kvs::ValueArray<int> c_histogram_recv( tf_number * nbins );
    */
    //最大最小値の集計
    if( parameter_file_opened )
    {
        O_min_recv.fill(0x00);
        O_max_recv.fill(0x00);
        C_min_recv.fill(0x00);
        C_max_recv.fill(0x00);

        //if(mpi->rank==0)std::cout<<"MPI_Reduce"<<std::endl;
        MPI_Reduce( O_min.pointer(), O_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( O_max.pointer(), O_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
        MPI_Reduce( C_min.pointer(), C_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( C_max.pointer(), C_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );

        //if(mpi->rank==0) std::cout<<"end MPI_Reduce"<<std::endl;

        //ヒストグラムの集計
        o_histogram_recv.fill(0x00);
        MPI_Reduce( o_histogram.pointer(), o_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );

        c_histogram_recv.fill(0x00);
        MPI_Reduce( c_histogram.pointer(), c_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    }

    timer.stop();
    time.mpi_reduce = timer.sec();
    timer.start();

    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::ofstream ofs( "state.txt", std::ios::out);
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if (!async_io_enabled){
            std::ofstream ofs( stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< 0 <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
        }


        std::stringstream step;
        step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;

        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::string history_file_name = "history" + step.str() + ".txt";
        std::string history_file_name = visParamDir + "history" + step.str() + ".txt";
        // 20181226 end

        std::ofstream ofs2( history_file_name.c_str(), std::ios::out);
        ofs2<<"TF_NUMBER="<<tf_number<<std::endl;
        for( int i = 0; i < tf_number; i++ )
        {
            ofs2<<"MIN_O"<<i+1<<"="<<O_min_recv[i]<<std::endl;
            ofs2<<"MAX_O"<<i+1<<"="<<O_max_recv[i]<<std::endl;
            ofs2<<"MIN_C"<<i+1<<"="<<C_min_recv[i]<<std::endl;
            ofs2<<"MAX_C"<<i+1<<"="<<C_max_recv[i]<<std::endl;
            ofs2<<"RESOLUTION_O"<<i+1<<"="<<nbins<<std::endl;
            ofs2<<"HISTOGRAM_O"<<i+1<<"=";
            for(int j=0; j<nbins; j++)
            {
                ofs2<<o_histogram_recv[j + i*nbins]<<",";
            }
            ofs2<<std::endl;
            ofs2<<"RESOLUTION_C"<<i+1<<"="<<nbins<<std::endl;
            ofs2<<"HISTOGRAM_C"<<i+1<<"=";
            for(int j=0; j<nbins; j++)
            {
                ofs2<<c_histogram_recv[j + i*nbins]<<",";
            }
            ofs2<<std::endl;
        }
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();

        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::string jupiter_file_name = "jupiter" + step.str() + ".tf";
        std::string jupiter_file_name = visParamDir + tfFilename + step.str() + ".tf";
        // 20181226 end
        param.write( jupiter_file_name );
    }

    timer.stop();
    time.write_text = timer.sec();

    show_timer( time );
    if(mpi_rank == 0){
        TimedScope::summary(mpi_size);
    }
}

void state_txt_writer( void )
{
    std::ofstream ofs( "state.txt", std::ios::out);
        if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

        ofs<<"LATEST_STEP=NO_STEP"<<std::endl;

        ofs.close();
}





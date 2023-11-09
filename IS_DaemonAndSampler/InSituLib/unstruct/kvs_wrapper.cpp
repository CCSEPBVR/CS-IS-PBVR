#include <fstream>
#include <cstdio>
#include <vector>
#include <kvs/Vector3>
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/StructuredVolumeObject>
#include <kvs/TrilinearInterpolator>
#include <kvs/PointObject>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>
#include <kvs/MersenneTwister>
#include <kvs/TransferFunction>
#include <kvs/RGBColor>
#include <kvs/Timer>

#include <sstream>
#include <iomanip>

#include "kvs_wrapper.h"
#include "TFS/CellByCellParticleGenerator.h"
#include "TFS/TransferFunctionSynthesizer.h"
#include "TFS/ParamInfo.h"
#include "float.h"
#include "TFS/UnstructuredVolumeObject.h"
#include <mpi.h>
#include "TFS/CellBase.h"
#include "TFS/CellBase_hex.h"
#include "TFS/TetrahedralCell.h"
#include "TFS/QuadraticTetrahedralCell.h"
#include "TFS/HexahedralCell.h"
#include "TFS/QuadraticHexahedralCell.h"
#include "TFS/PrismaticCell.h"
#include "TFS/PyramidalCell.h"



// add FJ start
#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 128
#endif
// add FJ  end


#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#define RANK 1

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
namespace
{

kvs::Vector3f RandomSamplingInCube( kvs::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();

    const kvs::Vector3f vertex( x, y, z );

    return vertex;
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
            /*ファイルが不完全(書き込み途中)の場合、完全になるまで一時停止する。*/
            //ファイルが完全ならLoadINは1回で終わる
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

    // get TF_NUMBER
    tf_number = param->getInt( "TF_NUMBER" );

    tfs->setColorFunction( eq );

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
    float min = kvs::Math::Min( object->minObjectCoord().x(),
                                object->minObjectCoord().y(),
                                object->minObjectCoord().z() );
    float max = kvs::Math::Max( object->maxObjectCoord().x(),
                                object->maxObjectCoord().y(),
                                object->maxObjectCoord().z() );
    const float sampling_step = (max - min) / 1E1;
    int particle_limit = param.getInt( "PARTICLE_LIMIT" );
#if 0
    double total_volume = static_cast<double>( cdo->gnx )
                        * static_cast<double>( cdo->gny )
                        * static_cast<double>( cdo->gnz );
#else
    double total_volume = ( object->maxObjectCoord().x() - object->minObjectCoord().x() )
                        * ( object->maxObjectCoord().y() - object->minObjectCoord().y() )
                        * ( object->maxObjectCoord().z() - object->minObjectCoord().z() );
#endif
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

    //if( mpi_rank == RANK )
    if( mpi_rank == 0 )
    {
        fprintf( stdout , "---------initialize Parameters-------------\n" );
        fprintf( stdout , "particle_limit    = %20d\n", particle_limit );
        fprintf( stdout , "particle_density  = %20f\n", *particle_density );
        fprintf( stdout , "resolutin_height  = %20d\n", height );
        fprintf( stdout , "resolutin_width   = %20d\n", width );
        fprintf( stdout , "total_volume      = %20.3e\n", total_volume );
        fprintf( stdout , "  |-X             = %20f\n", object->maxObjectCoord().x() );
        fprintf( stdout , "  |-Y             = %20f\n", object->maxObjectCoord().y() );
        fprintf( stdout , "  |-Z             = %20f\n", object->maxObjectCoord().z() );
        fprintf( stdout , "max_opacity       = %20.3e\n", *max_opacity );
        fprintf( stdout , "max_density       = %20.3e\n", *max_density );
        fprintf( stdout , "sampling_step     = %20.3e\n", sampling_step );
        fprintf( stdout , "subpixel_level    = %20d\n", *subpixel_level );
        fprintf( stdout , "------------------------------------\n" );
    }
    
    return opend;
}

bool NullSpace( const kvs::UInt32* indeces,
                Type* fs0, Type* fs1, 
                Type* fl0, Type* fl1 )
{
    bool null = true;

    for( int i=0; i<8; i++ )
    {

        if( fs0[indeces[i]] > 0.4 ) null = false;
        if( fs1[indeces[i]] > 0.4 ) null = false;
        if( fl0[indeces[i]] > 0.4 ) null = false;
        if( fl1[indeces[i]] > 0.4 ) null = false;
    }

    return null;
}

}//end of unnamed namespace

const size_t calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    kvs::MersenneTwister* MT )
{
    const float N = density * volume_of_cell;
    const float R = MT->rand();

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
    double writting;
    double mpi_reduce;
    double write_text;
    int    nparticles;

    double time_total =
        time.initialize + time.sampling + time.writting + time.mpi_reduce + time.write_text;
    double total;

    MPI_Reduce( &(time.initialize), &initialize, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.sampling),   &sampling,   1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
    MPI_Reduce( &(time.writting),   &writting,   1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
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
        printf("witting   = %8.3e [sec/step]\n", writting  );
        printf("mpi_reduce= %8.3e [sec/step]\n", mpi_reduce);
        printf("write_text= %8.3e [sec/step]\n", write_text);
        printf("total     = %8.3e [sec/step]\n", total     );
        printf("nparticles= %d\n", nparticles );
    }
}


#if 0
void generate_particles( const int time_step,
                         Type** values, int nvariables,
                         float* coords, int ncoords,
                         unsigned int* connections, int ncells,
                         domain_parameters* cdo,
                         mpi_parameters* mpi,
                         time_parameters* time )
#else
void generate_particles( int time_step,
                         domain_parameters dom,
                         Type** values, int nvariables,
                         float* coordinates, int ncoords,
                         unsigned int* connections, int ncells, const pbvr::VolumeObjectBase::CellType& celltype) //celltype  enum 型に変更
#endif
{

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    const int num_volume_data = nvariables;

    //if(mpi->rank==0)std::cout<<"start generate_particles\n";
    static bool start_flag = true;
    static bool parameter_file_opened=false;
    kvs::Timer timer( kvs::Timer::Start );

    std::vector< std::vector< pbvr::CellBase<Type>* > >  interp;
    interp.resize( max_threads );

    switch ( celltype )
    {
        case pbvr::VolumeObjectBase::Tetrahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::TetrahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::QuadraticTetrahedra:
            {
                std::cout << "Cell type : Quadratic tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::QuadraticTetrahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Hexahedra:
            {
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables  );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::QuadraticHexahedra:
            {
                std::cout << "Cell type : Quadratic hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::QuadraticHexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Prism:
            {
                if (mpi_rank == 0) std::cout << "celltype: Prism " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::PrismaticCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Pyramid:
            {
                if (mpi_rank == 0) std::cout << "celltype: Pyramid" << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::PyramidalCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
//        case pbvr::VolumeObjectBase::Triangle:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::TriangleCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case pbvr::VolumeObjectBase::QuadraticTriangle:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::QuadraticTriangleCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case pbvr::VolumeObjectBase::Square:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::SquareCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case pbvr::VolumeObjectBase::QuadraticSquare:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::QuadraticSquareCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
        default:
            {
                //BaseClass::m_is_success = false;
                //kvsMessageError( "Unsupported cell type." );
                std::cout << "Unsupported cell type." << std::endl; 
                return;
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

    //if(mpi->rank==0)std::cout<<"start initializeTFS()\n";
    kvs::StructuredVolumeObject* object = new kvs::StructuredVolumeObject();//Global Min Max volume object
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
    if(mpi_rank==RANK) std::cout<<"max_vec:"<<max_vec<<std::endl;
    bool tmp_parameter_file_opened = 
        initializeParameters( tfs, tf, &param, object, &sampling_volume_inverse, &max_opacity, &max_density, &subpixel_level, &particle_density, &particle_data_size_limit, visParamDir, tfFilename );

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

    if(mpi_rank == 0) std::cout<<"******* max_nparticles="<<max_nparticles<<std::endl;

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
        }
    }

    int particles_process_limit = static_cast<int> (  ( particle_data_size_limit * 10E6 )
                                                    / ( sizeof( float ) + sizeof( Byte ) + sizeof( float ) ) );
    bool particle_limit_over = false;

    time_parameters time;

    timer.stop();
    time.initialize = timer.sec();
    timer.start();

    #pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

        timer.start();

        kvs::MersenneTwister MT( thid + mpi_rank * nthreads );

        // 動的な粒子データ配列
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;

        //ヒストグラムの配列
        std::vector<float> o_scalars( tf_number );//頂点の不透明度
        std::vector<float> c_scalars( tf_number );//頂点の色
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

        // -----------------------------------
        //配列の追加
        kvs::Vector3f local_center_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_center_array[ SIMD_BLK_SIZE ];
        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];

        float cell_opacity_array[ SIMD_BLK_SIZE ];
        std::vector<float> o_scalars_array[ SIMD_BLK_SIZE ];
        std::vector<float> c_scalars_array[ SIMD_BLK_SIZE ];

        for (int i = 0; i < SIMD_BLK_SIZE; i++ )
        {
            o_scalars_array[i].resize( tf_number );
            c_scalars_array[i].resize( tf_number );
        }

        int nparticles_array[ SIMD_BLK_SIZE ];

        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
        float density_array[ SIMD_BLK_SIZE ];

        
        kvs::Vector3f l_plus_coord[ SIMD_BLK_SIZE ];
        kvs::Vector3f l_minus_coord[ SIMD_BLK_SIZE ];
        kvs::Vector3f g_plus_coord[ SIMD_BLK_SIZE ];
        kvs::Vector3f g_minus_coord[ SIMD_BLK_SIZE ];
        float S_plus_opacity[ SIMD_BLK_SIZE ];
        float S_minus_opacity[ SIMD_BLK_SIZE ];
        float dsdx_array[ SIMD_BLK_SIZE ];
        float dsdy_array[ SIMD_BLK_SIZE ];
        float dsdz_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f grad_array[ SIMD_BLK_SIZE ];
        kvs::RGBColor color_array[ SIMD_BLK_SIZE ];
        // -----------------------------------

        //粒子生成ループ開始
#pragma omp for schedule( dynamic ) nowait
//#pragma omp for schedule( static ) nowait
//#pragma omp for schedule( static, 1 ) nowait
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMD_BLK_SIZE )
        {
           //ブロック内でのループ回数を取得
            int remain = ( ncells - cell_base > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: ncells - cell_base;

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = kvs::Vector3f ( 0.5, 0.5, 0.5 );
            }

            //補間器にセルを一括でバインド
            for(int i = 0; i < nvariables; i++)
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }

            interp[thid][0]->setLocalPointArray( remain, local_center_array );
            interp[thid][0]->transformLocalToGlobalArray( remain,
                                                          local_center_array,
                                                          global_center_array );

            if( parameter_file_opened )
            {

               th_tfs[thid]->SynthesizedOpacityScalarsArray( interp[thid],
                                                              remain,
                                                              local_center_array,
                                                              global_center_array,
                                                              o_scalars_array );

               th_tfs[thid]->SynthesizedColorScalarsArray( interp[thid],
                                                           remain,
                                                           local_center_array,
                                                           global_center_array,
                                                           c_scalars_array );

               for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
               {
                   for( int i = 0; i < tf_number; i++ )
                   {
                        float h = (o_scalars_array[cell_BLK][i] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
                        int H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_o_histogram[ H + nbins*i]++;
                        }

                        h = (c_scalars_array[cell_BLK][i] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
                        H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_c_histogram[ H + nbins*i]++;
                        }
/*
                        if( cell_BLK==0 )
                        {
                            th_O_min[i] = o_scalars_array[cell_BLK][i];
                            th_O_max[i] = o_scalars_array[cell_BLK][i];
                            th_C_min[i] = c_scalars_array[cell_BLK][i];
                            th_C_max[i] = c_scalars_array[cell_BLK][i];
                        }
                        else
                        {
                            th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                            th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                            th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                            th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];
                        }
*/
                        // 20190128 修正
                        th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                        th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                        th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                        th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];


                    }
                }
            }


            th_tfs[thid]->CalculateOpacityArrayAverage( interp[thid],
                                                 remain,
                                                 local_center_array,
                                                 global_center_array,
                                                 th_tf[thid],
                                                 cell_opacity_array );
             //生成粒子数を計算
            int nparticles_num = 0;
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    const float density = Generator::CalculateDensity( cell_opacity_array[cell_BLK],
                                                                       sampling_volume_inverse,
                                                                       max_opacity, max_density );
//                    const float density = cell_opacity_array[cell_BLK] < 0.00001 ? 0.0 : max_density;
                    interp[thid][0]->bindCell( cell_index[cell_BLK] );
                    nparticles_array[cell_BLK] 
                        = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT );
                nparticles_array[cell_BLK] *= particle_density;
                nparticles_num += nparticles_array[cell_BLK];

            }

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
        /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                // ------------------------------------------------

                for( int i = 0; i < nparticles_array[cell_BLK]; i+=SIMD_BLK_SIZE )
                {
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( nparticles_array[cell_BLK] - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_array[cell_BLK] - i;
                    //一括でセルをバインドするための配列と、座標の取得
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
                        kvs::UInt32 cell_id = cell_base + cell_BLK;
                    while(1)
                    {
                        local_coord_array[j] = interp[thid][0] -> randomSampling_MT( &MT );

                        //補間器にセルを一括でバインド
                        for( int k = 0; k < nvariables; k++ )
                        {
                            interp[thid][k]->bindCell( cell_index[j] );
                        }
                    
                        interp[thid][0]->setLocalPoint( local_coord_array[j] );
                        global_coord_array[j] = interp[thid][0]->transformLocalToGlobal( local_coord_array[j] );
                        cell_opacity_array[j] = th_tfs[thid]->CalculateOpacity( interp[thid],
                                local_coord_array[j],
                                global_coord_array[j],
                                th_tf[thid]);
                        
                        density_array[j] = Generator::CalculateDensity( cell_opacity_array[j],
                                 sampling_volume_inverse,
                                 max_opacity, max_density );
                        if(density_array[j] > 0)
                        {
//                            std::cout << "break!!!!!" <<std::endl; 
                            break;
                        }

                    }
                    }
                         
//                    //densityの条件に適合するnparticlesの個数の取得
//                    //そのときのcell_index, local_coordを再配置
                    int nparticles_count = 0;
                    for( int j = 0; j < remain_BLK; j++ )
                    {
                        while( 1 )
                        {
                            //if( density > max_density * (float)MT.rand() )
                            if( density_array[j] > max_density * (float)MT.rand() )
                            {
                                cell_index[ nparticles_count ] = cell_index[j];
                                local_coord_array[ nparticles_count ] = local_coord_array[j];
                                global_coord_array[ nparticles_count ] = global_coord_array[j];
                                nparticles_count +=1;
                                break;
                            }
                        } //while loop
                    }

// ------------------------------------------------

                    for( int j = 0; j < nvariables; j++ )
                    {
                        interp[thid][j]->bindCellArray( nparticles_count, cell_index );
                    }

                    // dsdx ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0.1,0,0);
                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(-0.1,0,0);

                    }

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );


                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf[thid],
                                                         S_plus_opacity );
                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf[thid],
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        dsdx_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------

                    // dsdy ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0.1,0);
                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,-0.1,0);
                    }

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );

                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf[thid],
                                                         S_plus_opacity );
                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf[thid],
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        dsdy_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------
                    // dsdz ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0,0.1);
                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0,-0.1);
                    }

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );

                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf[thid],
                                                         S_plus_opacity );
                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf[thid],
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        dsdz_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------
                    //grad_arrayの算出
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        //JacobiMatrixでメンバ変数を使用しているので再度バインド
                        interp[thid][0]->bindCell( cell_index[j] );

                        const kvs::Vector3f g( -dsdx_array[j], -dsdy_array[j], -dsdz_array[j] );
                        const kvs::Matrix33f J = interp[thid][0]->JacobiMatrix();
                        float determinant = 0.0f;
                        const kvs::Vector3f G = J.inverse( &determinant ) * g;
                        grad_array[j] = kvs::Math::IsZero( determinant ) ? kvs::Vector3f( 0.0f, 0.0f, 0.0f ) : G;
                    }

                    //色の計算
                    th_tfs[thid]->CalculateColorArray( interp[thid],
                                                       nparticles_count,
                                                       local_coord_array,
                                                       global_coord_array,
                                                       th_tf[thid],
                                                       color_array );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        th_vertex_coords.push_back( global_coord_array[j].x() );
                        th_vertex_coords.push_back( global_coord_array[j].y() );
                        th_vertex_coords.push_back( global_coord_array[j].z() );

                        th_vertex_colors.push_back( color_array[j].r() );
                        th_vertex_colors.push_back( color_array[j].g() );
                        th_vertex_colors.push_back( color_array[j].b() );

                        th_vertex_normals.push_back( grad_array[j].x() );
                        th_vertex_normals.push_back( grad_array[j].y() );
                        th_vertex_normals.push_back( grad_array[j].z() );
                    }
                // ------------------------------------------------
                
                }//end of for i
            }
        /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
        }// end of for cell
//        #pragma omp barrier
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

            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
        }

                timer.stop();
    } //#pragma omp parallel

    //std::cout << "end sampling l: " << __LINE__<<  std::endl; 

    timer.stop();
    time.sampling = timer.sec();
    time.nparticles = vertex_coords.size()/3;
    timer.start();

    delete tfs;

    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;

    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < nvariables; j++ )
        {
             if (interp[i][j] != NULL)delete interp[i][j];
        }
    }
    
    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    kvs::ValueArray<float> coords( vertex_coords );
    kvs::ValueArray<Byte>  colors( vertex_colors );
    kvs::ValueArray<float> normals( vertex_normals );

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

    /*  分割後コミュニケータのランク0で出力する  */
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
            particle_write_thread->setTimestep(time_step ,stateFilePath.c_str());
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
    //if(mpi->rank==0)std::cout<<"end generate_particles\n";

}

void state_txt_writer( void )
{
    std::ofstream ofs( "state.txt", std::ios::out);
        if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

        ofs<<"LATEST_STEP=NO_STEP"<<std::endl;

        ofs.close();
}



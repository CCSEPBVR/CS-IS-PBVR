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
#include <memory>

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

#include <kvs/HexahedralCell>
#include <kvs/TetrahedralCell>
#include <kvs/CellBase>

#ifdef VTK
//VTK
#include <vtkSmartPointerBase.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include "FileFormat/VtkUnstructuredFileFormat.h"
#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include <vtkUnstructuredGrid.h>

//kvsmlImporter
#include "CvtTypeTraits.h"
#include "Importer/VtkImporter.h"
#include "TFS/TetrahedralCell.h"
#endif

//Glyph
#include "TFS/GlyphGenerator.h"
#include "TFS/GlyphProperty.h"

//PlotOverLine
#include "TFS/PlotOverLine.h"

// add FJ start
#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 128
#endif
// add FJ  end

#include <unistd.h>
#include <random>

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
TransferFunctionSynthesizer* m_tfs;
static bool generate_flag  = false;
size_t  st_time_step = 0;

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

void SetDefalutParameter( TransferFunctionSynthesizer* tfs,
                          pbvr_parameters* particleBase,
//                          std::vector<pbvr::TransferFunction>* tf,
                          const int nvariables, Type** values, const int ncoords)
{
    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var1;
    int tf_number;
    // get TF_NUMBER
    tf_number = nvariables;
    //std::vector<pbvr::TransferFunction> tf = particleBase ->m_tf;
    std::vector<pbvr::TransferFunction> tf;

    //Read 1D tf
    int resolution = 256;
    float min, max;

    tf.clear();

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        // 各変量のminmax
        // calc_minmax
        min = values[i][0];
        max = values[i][0];
        
        for(int j = 1; j < ncoords; j++)
        {
            min = min < values[i][j] ? min : values[i][j]; 
            max = max > values[i][j] ? max : values[i][j]; 
        }

        kvs::ColorMap color_map( 256, min, max );
        kvs::OpacityMap opacity_map( 256, min, max );

        pbvr::TransferFunction tfBuf;
        tfBuf.setColorMap( color_map );
        tfBuf.setOpacityMap( opacity_map );
        tf.push_back(tfBuf);
    }
    particleBase ->m_tf = tf;

    // add by shimomura 2024/03/25
    std::string  equation;

    equation = "a1";
    //std::replace(equation.begin(), equation.end(), 'O', 'a');
    eq = tfs->convert_token(equation);
    tfs->setOpacityFunction( eq );

    equation = "c1" ;
    //std::replace(equation.begin(), equation.end(), 'C', 'c');
    eq = tfs->convert_token(equation);
    tfs->setColorFunction( eq );

    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "q" << i + 1 ;
        const std::string tag_base = tss.str();

        equation = tag_base;
        eq = tfs->convert_token(equation);
        var1.push_back( eq );
    }

    tfs->setOpacityVariable( var1 );
    tfs->setColorVariable( var1 );

    var1.clear();
    int nbin = 256;
    particleBase->m_tf_number = nvariables;
    particleBase->m_O_max.allocate(particleBase->m_tf_number);
    particleBase->m_O_min.allocate(particleBase->m_tf_number);
    particleBase->m_C_max.allocate(particleBase->m_tf_number);
    particleBase->m_C_min.allocate(particleBase->m_tf_number);
    particleBase->m_O_max.fill(0x00);
    particleBase->m_O_min.fill(0x00);
    particleBase->m_C_max.fill(0x00);
    particleBase->m_C_min.fill(0x00);
    particleBase->m_o_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_c_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_o_histogram.fill(0x00);
    particleBase->m_c_histogram.fill(0x00);
}


void readTFfromParamInfo( ParamInfo* param,
                          std::vector<pbvr::TransferFunction>& tf,
                          TransferFunctionSynthesizer* tfs )
{

    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    //Read TFS
    std::vector<int> i_table;
    std::vector<float> f_table;
    EquationToken eq;
    std::vector<EquationToken> var;
    int tf_number;
    // get TF_NUMBER
    tf_number = param->getInt( "TF_NUMBER" );

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
    

#if 1
    // add by shimomura 2024/03/25
    std::string  equation;

    equation = param->getString( "OPACITY_SYNTH" );
    std::replace(equation.begin(), equation.end(), 'O', 'a');
    eq = tfs->convert_token(equation);
    tfs->setOpacityFunction( eq );

    equation = param->getString( "COLOR_SYNTH" );
    std::replace(equation.begin(), equation.end(), 'C', 'c');
    eq = tfs->convert_token(equation);
    tfs->setColorFunction( eq );


    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = param->getString( tag_base +"VAR_C" );
        eq = tfs->convert_token(equation);


        var.push_back( eq );

    }

    tfs->setOpacityVariable( var );

    var.clear();
    for ( size_t i = 0; i < tf_number; i++ )
    {
        std::stringstream tss;
        tss << "TF_NAME" << i + 1 << "_";
        const std::string tag_base = tss.str();

        equation = param->getString( tag_base +"VAR_O" );
        eq = tfs->convert_token(equation);

        var.push_back( eq );
    }

    tfs->setColorVariable( var );
    var.clear();
#endif 

}

bool initializeParameters(
    TransferFunctionSynthesizer* tfs,
    std::vector<pbvr::TransferFunction>& tf,
    ParamInfo *param_info,
    const kvs::ObjectBase* object,
    float* sampling_volume_inverse,
    float* max_opacity, float* max_density, int* subpixel_level, float* particle_density, int* particle_limit,
    float* particle_data_size_limit,
    const std::string &visParamDir,
    const std::string &tfFilename, 
    const int time_step )
//    const std::string &tfFilename )
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
    // add by shimomura 0808
    if (generate_flag ==false && opend == false)
    {
        //  TFファイル未読み取り判定ならば、return
        return opend; 
    }
    else if (generate_flag ==false && opend == true )
    {
        generate_flag =true;
        st_time_step = time_step;
    }

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
    *particle_limit = param.getInt( "PARTICLE_LIMIT" );
#if 0
    double total_volume = static_cast<double>( cdo->gnx )
                        * static_cast<double>( cdo->gny )
                        * static_cast<double>( cdo->gnz );
#else
    double total_volume = ( object->maxObjectCoord().x() - object->minObjectCoord().x() )
                        * ( object->maxObjectCoord().y() - object->minObjectCoord().y() )
                        * ( object->maxObjectCoord().z() - object->minObjectCoord().z() );
#endif
    //*max_opacity = 0.98;
    *max_opacity = 0.9;
//    *subpixel_level = CalculateSubpixelLevel( *particle_limit , camera, sampling_step, total_volume, object );
    *subpixel_level = 1.f;
//    *subpixel_level = 2.f;
//    *subpixel_level = 3.f; // 4 scalar

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
        fprintf( stdout , "particle_limit    = %20d\n", *particle_limit );
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

inline const size_t calculate_number_of_particles(
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

void generate_particles( int time_step, domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype )
{
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    kvs::Timer timer( kvs::Timer::Start );

    static ParamInfo param;
    pbvr_parameters particleBase;
    bool skip_flag;
    skip_flag = SetParameter(dom, &particleBase, &param, time_step);

    particleBase.m_nvariables = nvariables; 
    PlotOverLine plot_over_line;
    plot_over_line.SetPOLParameter(time_step);
    timer.stop();
    if(mpi_rank == 0 ) std::cout << "initialize_time = " << timer.sec() << std::endl;
    if (skip_flag == false)
    {
        // デフォルト設定を伝達関数に設定
        SetDefalutParameter(m_tfs, &particleBase, nvariables, values, ncoords);
        GenerateHistogram(time_step, dom, values,
                nvariables, coordinates, ncoords,
                connections, ncells, celltype, particleBase);
    }    
    else
    {
    timer.start();
        GenerateParticles(time_step, dom, values,
            nvariables, coordinates, ncoords,
            connections, ncells, celltype, particleBase);
    timer.stop();
    std::cout << "generate_time = " << timer.sec() <<  std::endl;

//        GenerateGlyphs(time_step, dom, values,
//            nvariables, coordinates, ncoords,
//            connections, ncells, celltype);
//         callPlotOverLine(time_step, dom, values,
//             nvariables, coordinates, ncoords,
//             connections, ncells, celltype, &plot_over_line);                                        
//         plot_over_line.OutputLine(time_step);
 


    }

    OutputParticles(time_step, nvariables, particleBase, &param, skip_flag);
    delete m_tfs;
}

void SetVariables(kvs::UnstructuredVolumeObject* object, Type** values, pbvr::VolumeObjectBase::CellType* celltype )
{

        switch ( object -> cellType() )
        {
            case 4: // pbvr::VolumeObjectBase::Tetrahedra:
                {
                    *celltype = pbvr::VolumeObjectBase::Tetrahedra;
                    break;
                }
            case 10: //pbvr::VolumeObjectBase::QuadraticTetrahedra:
                {
                    *celltype = pbvr::VolumeObjectBase::QuadraticTetrahedra;
                }
            case 8: //  pbvr::VolumeObjectBase::Hexahedra:
                {
                    *celltype = pbvr::VolumeObjectBase::Hexahedra;
                    break;
                }
            case 20: //pbvr::VolumeObjectBase::QuadraticHexahedra:
                {
                    *celltype = pbvr::VolumeObjectBase::QuadraticHexahedra;
                    break;
                }
            case 6: // pbvr::VolumeObjectBase::Prism:
                {
                    *celltype = pbvr::VolumeObjectBase::Prism;
                    break;
                }
            case 5: //pbvr::VolumeObjectBase::Pyramid:
                {
                    *celltype = pbvr::VolumeObjectBase::Pyramid;
                    break;
                }
            default:
                {
                    std::cout << "Unsupported cell type." << std::endl; 
                    return;
                }
        }

        int nvariables = object -> veclen();    
        int ncoords = object -> nnodes(); 
        for ( int j = 0; j < nvariables; j++ )
        {
            for ( int i = 0; i < ncoords; i++ )
            {
                int  it = j * ncoords  + i;
                values[j][i] = object ->values().at<Type>(it);
            }
        }
}
#ifdef VTK
void SetDomain( vtkUnstructuredGrid* ucd, domain_parameters* dom)
{
    double bounds[6];
    ucd -> GetPoints() -> GetBounds(bounds); 
    float recv_Xmin, recv_Xmax;
    float recv_Ymin, recv_Ymax;
    float recv_Zmin, recv_Zmax;

    float Xmin = bounds[0];
    float Xmax = bounds[1];
    float Ymin = bounds[2];
    float Ymax = bounds[3];
    float Zmin = bounds[4];
    float Zmax = bounds[5];
    MPI_Allreduce(&Xmin, &recv_Xmin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&Ymin, &recv_Ymin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&Zmin, &recv_Zmin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&Xmax, &recv_Xmax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&Ymax, &recv_Ymax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&Zmax, &recv_Zmax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

//    std:: cout << "maxX = "<< recv_Xmax << std::endl;
//    std:: cout << "minX = "<< recv_Xmin << std::endl;
//    std:: cout << "maxY = "<< recv_Ymax << std::endl;
//    std:: cout << "minY = "<< recv_Ymin << std::endl;
//    std:: cout << "maxZ = "<< recv_Zmax << std::endl;
//    std:: cout << "minZ = "<< recv_Zmin << std::endl;

    dom->x_global_min = recv_Xmin;
    dom->y_global_min = recv_Ymin;
    dom->z_global_min = recv_Zmin;
    dom->x_global_max = recv_Xmax;
    dom->y_global_max = recv_Ymax;
    dom->z_global_max = recv_Zmax;

}


//#ifdef VTK
void generate_particles_vtk(  int time_step, vtkUnstructuredGrid* ucd ) 
{
    kvs::Timer timer( kvs::Timer::Start );
 
    domain_parameters dom; 
    SetDomain(ucd, &dom); 

    static ParamInfo param;
    pbvr_parameters particleBase;
    
    
    bool skip_flag;
    skip_flag = SetParameter(dom, &particleBase, &param, time_step);

    int mpi_rank = 0;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    
    // plot over line
    PlotOverLine plot_over_line;
    plot_over_line.SetPOLParameter(time_step);

    timer.stop();
    std::cout << mpi_rank << ", set_parameter = " << timer.sec() <<std::endl;
    timer.start();

    int  nvariables;
    float t_extract = 0;
    float t_generate_particles = 0;

    for (int i =0; i<8 ;i++)
    {
        particleBase.con_log[i] =0; 
    }        
    
    
    cvt::VtkXmlUnstructuredGrid input_vtu( ucd );
    for ( auto vtu : input_vtu.eachCellType() )
    {
        vtkSmartPointer<vtkUnstructuredGrid> volume = vtu.get();
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer( &vtu );
        kvs::UnstructuredVolumeObject* object = &importer;

        int ncoords = object -> nnodes();
        nvariables  = object -> veclen();
        particleBase.m_nvariables = nvariables; 

        pbvr::VolumeObjectBase::CellType celltype;
        Type** values;
        values = new Type * [nvariables];
        for ( int j = 0; j < nvariables; j++ )
        {
            values[j] = new float[ncoords];
        }

        SetVariables(object, values, &celltype); 


        timer.stop();
        t_extract += timer.sec();
        timer.start();
        if (skip_flag == false)
        {
            // デフォルト設定を伝達関数に設定
            SetDefalutParameter(m_tfs, &particleBase, nvariables, values, ncoords);
            GenerateHistogram(time_step, dom, values,
                    nvariables, (float*)object->coords().pointer(), ncoords,
                    (unsigned int*)object->connections().pointer(), object -> ncells(), celltype, particleBase);
        }    
        else
        {
            GenerateParticles(time_step, dom, values,
                    nvariables, (float*)object->coords().pointer(), ncoords,
                    (unsigned int*)object->connections().pointer() , object -> ncells(), celltype, particleBase);

            GenerateGlyphs(time_step, dom, values,
                    nvariables, (float*)object->coords().pointer(), ncoords,
                    (unsigned int*)object->connections().pointer() , object -> ncells(), celltype);

            GeneratePlotOverLine(time_step, object, &plot_over_line);
        }
        timer.stop();
        t_generate_particles += timer.sec();
        timer.start();

        for (int i =0; i< nvariables ; i++)
        {
            delete  values[i];
        }
        delete[] values;
    }

    // plot over line
    plot_over_line.OutputLine(time_step);


    timer.stop();
    std::cout << mpi_rank << ", extract() = " << t_extract <<std::endl;
    std::cout << mpi_rank << ", generate_particles() = " << t_generate_particles <<std::endl;
    timer.start();
    std::cout << "all nparitcles = " << particleBase.m_sample_coords.size()/3 <<std::endl;  
    OutputParticles(time_step, nvariables, particleBase, &param, skip_flag);
    delete m_tfs;
    
    timer.stop();
    std::cout << mpi_rank << ", output_particles =  " << timer.sec() <<std::endl;
}
#endif

bool SetParameter(const domain_parameters dom, pbvr_parameters* particleBase, ParamInfo *m_param ,const int time_step)
{
    int mpi_rank = 0;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    // Set Transfer function synthesizer.
    particleBase->m_tf.resize(0);
    m_tfs = new TransferFunctionSynthesizer();
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
    particleBase->m_visParamDir = visParamDir;
    particleBase->m_ptcFilePath = ptcFilePath;
    particleBase->m_stateFilePath = stateFilePath;
    particleBase->m_tfFilename = tfFilename;

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
    particleBase->m_min_vec = min_vec;
    particleBase->m_max_vec = max_vec;
    if(mpi_rank==RANK) std::cout<<"max_vec:"<<max_vec<<std::endl;
    bool tmp_parameter_file_opened =
        initializeParameters( m_tfs, particleBase->m_tf, m_param, object, &particleBase->m_sampling_volume_inverse, &particleBase->m_max_opacity, &particleBase->m_max_density,
                             &particleBase->m_subpixel_level, &particleBase->m_particle_density, &particleBase->m_particle_limit, &particleBase->m_particle_data_size_limit, visParamDir, tfFilename, time_step );

    int tf_number = particleBase->m_tf.size();

    
    particleBase->m_tf_number = tf_number;
    particleBase->m_parameter_file_opened = true;

    delete object;
    //if(mpi->rank==0)std::cout<<"end initializeTFS()\n";

    //add by shimomura 20240722
    int nbin =256;
    particleBase->m_O_max.allocate(particleBase->m_tf_number);
    particleBase->m_O_min.allocate(particleBase->m_tf_number);
    particleBase->m_C_max.allocate(particleBase->m_tf_number);
    particleBase->m_C_min.allocate(particleBase->m_tf_number);
    particleBase->m_O_max.fill(0x00);
    particleBase->m_O_min.fill(0x00);
    particleBase->m_C_max.fill(0x00);
    particleBase->m_C_min.fill(0x00);
    particleBase->m_o_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_c_histogram.allocate(particleBase->m_tf_number*nbin);
    particleBase->m_o_histogram.fill(0x00);
    particleBase->m_c_histogram.fill(0x00);

    // TFファイルがないならば、retrun
    if ( generate_flag == false )
    {
        std::cout << "find no .tf!! skipping generate_particle !!!" << std::endl;
        return false;
    }
    // moved by shimomura 20240807

    return true;
}

void GenerateHistogram( int time_step,
                         domain_parameters dom,
                         Type** values, int nvariables,
                         float* coordinates, int ncoords,
                         unsigned int* connections, int ncells, const pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase) //celltype  enum 型に変更
{

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );


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

    int   tf_number                = particleBase.m_tf_number;
    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;
    int   subpixel_level           = particleBase.m_subpixel_level          ;
    float particle_density         = particleBase.m_particle_density        ;
    float particle_data_size_limit = particleBase.m_particle_data_size_limit;
    parameter_file_opened = particleBase.m_parameter_file_opened;
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
        o_min[i] = particleBase.m_tf[i].opacityMap().minValue();
        o_max[i] = particleBase.m_tf[i].opacityMap().maxValue();
        c_min[i] = particleBase.m_tf[i].colorMap().minValue();
        c_max[i] = particleBase.m_tf[i].colorMap().maxValue();
    }

    //最大最小値
    kvs::ValueArray<float> O_min( tf_number );//計算して得る最大最小値
    kvs::ValueArray<float> O_max( tf_number );
    kvs::ValueArray<float> C_min( tf_number );
    kvs::ValueArray<float> C_max( tf_number );

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
        th_tfs[n] = new TransferFunctionSynthesizer( *m_tfs );
    }

    th_tf.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[ i ].resize( tf_number );
        for ( int j = 0; j < tf_number; j++ )
        {
            th_tf[i][j] = particleBase.m_tf[j];
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

        //粒子生成ループ開始
#pragma omp for schedule( dynamic ) nowait
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMD_BLK_SIZE )
        {
           //ブロック内でのループ回数を取得
            int remain = ( ncells - cell_base > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: ncells - cell_base;

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                //local_center_array[cell_BLK] = kvs::Vector3f ( 0.5, 0.5, 0.5 );
                local_center_array[cell_BLK] = interp[thid][0]->localGravityPoint();
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

                        // 20190128 修正
                        th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                        th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                        th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                        th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];
                    }
                }
            }
        }

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
        }

    } // end loop omp parallel

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
    
    for( int n = 0; n < tf_number * nbins; n++ )
    {
        particleBase.m_o_histogram[n] += o_histogram[n];
        particleBase.m_c_histogram[n] += c_histogram[n];
    }

    for( int i = 0; i < tf_number; i++ )
    {
        //不透明度
        particleBase.m_O_min[i] = particleBase.m_O_min[i] < O_min[i] ? particleBase.m_O_min[i] : O_min[i];
        particleBase.m_O_max[i] = particleBase.m_O_max[i] > O_max[i] ? particleBase.m_O_max[i] : O_max[i];
        //色
        particleBase.m_C_min[i] = particleBase.m_C_min[i] < C_min[i] ? particleBase.m_C_min[i] : C_min[i];
        particleBase.m_C_max[i] = particleBase.m_C_max[i] > C_max[i] ? particleBase.m_C_max[i] : C_max[i];
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
void GenerateParticles( int time_step,
                         domain_parameters dom,
                         Type** values, int nvariables,
                         float* coordinates, int ncoords,
                         unsigned int* connections, int ncells, const pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase) //celltype  enum 型に変更
#endif
{

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    if(mpi_rank==0)std::cout<<"start generate_particles\n";
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
   
    int   tf_number                = particleBase.m_tf_number;
    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;
    int   subpixel_level           = particleBase.m_subpixel_level          ;
    float particle_density         = particleBase.m_particle_density        ;
    float particle_data_size_limit = particleBase.m_particle_data_size_limit;
    parameter_file_opened = particleBase.m_parameter_file_opened;
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
        o_min[i] = particleBase.m_tf[i].opacityMap().minValue();
        o_max[i] = particleBase.m_tf[i].opacityMap().maxValue();
        c_min[i] = particleBase.m_tf[i].colorMap().minValue();
        c_max[i] = particleBase.m_tf[i].colorMap().maxValue();
        
        o_min[i] = o_min[i] < FLT_MIN ? FLT_MIN : o_min[i];  
        o_max[i] = o_max[i] > FLT_MAX ? FLT_MAX : o_max[i]; 
        c_min[i] = c_min[i] < FLT_MIN ? FLT_MIN : c_min[i]; 
        c_max[i] = c_max[i] > FLT_MAX ? FLT_MAX : c_max[i]; 
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
        th_tfs[n] = new TransferFunctionSynthesizer( *m_tfs );
    }

    th_tf.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[ i ].resize( tf_number );
        for ( int j = 0; j < tf_number; j++ )
        {
            th_tf[i][j] = particleBase.m_tf[j];
        }
    }

    int particles_process_limit = static_cast<int> (  ( particle_data_size_limit * 10E6 )
                                                    / ( sizeof( float ) + sizeof( Byte ) + sizeof( float ) ) );
    bool particle_limit_over = false;

    time_parameters time;

    timer.stop();
    time.initialize = timer.sec();
    timer.start();

  
            std::cout <<  mpi_rank << ": "  <<  __FUNCTION__  << ": " << __LINE__ << std::endl;
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
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMD_BLK_SIZE )
        {
           //ブロック内でのループ回数を取得
            int remain = ( ncells - cell_base > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: ncells - cell_base;

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = interp[thid][0]->localGravityPoint();
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
                std::vector<bool> o_zero_flag(tf_number); 
                std::vector<bool> c_zero_flag(tf_number); 
                for( int i = 0; i < tf_number; i++ )
                {
                    o_zero_flag[i] = false;
                    c_zero_flag[i] = false;
                    if ( kvs::Math::Equal<float>(o_max[i], o_min[i] ))   //0　判定ならば、一様分布にする
                    {
                        o_zero_flag[i] = true;
                        for (int k =0 ; k< nbins; k++)
                        {
                            th_o_histogram[ k+nbins*i] ++;
                        }
                    }

                    if ( kvs::Math::Equal<float>(c_max[i], c_min[i] ))   //0　判定ならば、一様分布にする
                    {
                        c_zero_flag[i] = true;

                        for (int k =0 ; k< nbins; k++)
                        {
                            th_c_histogram[ k+nbins*i] ++;
                        }

                    }

                }

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
                       if (!o_zero_flag[i]) 
                       {
                           float h = (o_scalars_array[cell_BLK][i] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
                           int H = (int)h;
                           if( 0 <= H && H <= nbins )
                           {
                               if( H == nbins ) H--;
                               th_o_histogram[ H + nbins*i]++;
                           }


                       }

                       if (!c_zero_flag[i]) 
                       {
                           float h = (c_scalars_array[cell_BLK][i] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
                           int H = (int)h;
                           if( 0 <= H && H <= nbins )
                           {
                               if( H == nbins ) H--;
                               th_c_histogram[ H + nbins*i]++;
                           }
                        }

//                        if(thid == 0 && cell_BLK == 0 && o_scalars_array[cell_BLK][0] > 0 )std::cout << "o_scalars_array[cell_BLK][i] = " << o_scalars_array[cell_BLK][0] << std::endl; 
//                        if(thid == 0 &&  o_scalars_array[cell_BLK][0] > -8 )std::cout << "o_scalars_array[cell_BLK][i] = " << o_scalars_array[cell_BLK][0] << std::endl; 
                        // 20190128 修正
                        th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                        th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                        th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                        th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];

                   }

                }
            }

            //th_tfs[thid]->CalculateOpacityArrayAverage( interp[thid],
            th_tfs[thid]->CalculateOpacityArray( interp[thid],
                                                 remain,
                                                 local_center_array,
                                                 global_center_array,
                                                 th_tf[thid],
                                                 cell_opacity_array);
             //生成粒子数を計算
            int nparticles_num = 0;
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    float density = Generator::CalculateDensity( cell_opacity_array[cell_BLK],
                                                                       sampling_volume_inverse,
                                                                       max_opacity, max_density );
//                  if(cell_opacity_array[cell_BLK] > 0)  std::cout << "density = " << density <<  ", cell_opacity_array[cell_BLK] =  " << cell_opacity_array[cell_BLK] << ", sampling_volume_inverse = " << sampling_volume_inverse  << std::endl;
#ifdef REJECTION
                    density             = cell_opacity_array[cell_BLK] < 0.0039 ? 0.0 : density; //  less than 1/256
#endif
                    interp[thid][0]->bindCell( cell_index[cell_BLK] );
                    nparticles_array[cell_BLK] 
                        = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT ) ;
                nparticles_array[cell_BLK] *= particle_density;
                nparticles_num += nparticles_array[cell_BLK];
                if(nparticles_array[cell_BLK] > 0 ) std::cout << "nparticles_array[cell_BLK] = " << nparticles_array[cell_BLK] << "nparticles_num = " << nparticles_num << std::endl;
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
#ifdef REJECTION
                    int nparticles_count = 0;
#endif
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
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

#ifdef REJECTION
                          if( density_array[j] > max_density * (float)MT.rand() )
                          {
                              cell_index[ nparticles_count ] = cell_index[j];
                              local_coord_array[ nparticles_count ] = local_coord_array[j];
                              global_coord_array[ nparticles_count ] = global_coord_array[j];
                              nparticles_count +=1;
                              break;
                          }
#else
                          if(density_array[j] > 0)
                          {
//                          std::cout << "break!!!!!" <<std::endl; 
                              break;
                          }
#endif                          
                        }  //while loop 
                    }
                    //densityの条件に適合するnparticlesの個数の取得
                    //そのときのcell_index, local_coordを再配置
#ifdef REJECTION 
#else
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
#endif

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
        #pragma omp barrier
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

//                timer.stop();
    } //#pragma omp parallel

    timer.stop();
    time.sampling = timer.sec();
    time.nparticles = vertex_coords.size()/3;
    timer.start();

//    delete tfs;

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
    
    for( int n = 0; n < tf_number * nbins; n++ )
    {
        particleBase.m_o_histogram[n] += o_histogram[n];
        particleBase.m_c_histogram[n] += c_histogram[n];
    }

    for( int i = 0; i < tf_number; i++ )
    {
        //不透明度
        particleBase.m_O_min[i] = particleBase.m_O_min[i] < O_min[i] ? particleBase.m_O_min[i] : O_min[i];
        particleBase.m_O_max[i] = particleBase.m_O_max[i] > O_max[i] ? particleBase.m_O_max[i] : O_max[i];
        //色
        particleBase.m_C_min[i] = particleBase.m_C_min[i] < C_min[i] ? particleBase.m_C_min[i] : C_min[i];
        particleBase.m_C_max[i] = particleBase.m_C_max[i] > C_max[i] ? particleBase.m_C_max[i] : C_max[i];
        std::cout << mpi_rank <<" : particleBase.m_C_min["<< i << "] = " << particleBase.m_C_min[i] << std::endl;
        std::cout << mpi_rank <<" : particleBase.m_C_max["<< i << "] = " << particleBase.m_C_max[i] << std::endl;
    }

    std::cout << "nparticles = " <<  vertex_coords.size()/3   << std::endl;
    particleBase.m_sample_coords.insert(particleBase.m_sample_coords.end(), vertex_coords.begin(), vertex_coords.end());
    particleBase.m_sample_colors.insert(particleBase.m_sample_colors.end(), vertex_colors.begin(), vertex_colors.end());
    particleBase.m_sample_normals.insert(particleBase.m_sample_normals.end(), vertex_normals.begin(), vertex_normals.end());

    timer.stop();
    time.writting = timer.sec();
//    show_timer( time );
}

void GenerateGlyphs( int time_step,
                         domain_parameters dom,
                         Type** values, int nvariables,
                         float* coordinates, int ncoords,
                         unsigned int* connections, int ncells, const pbvr::VolumeObjectBase::CellType& celltype) //celltype  enum 型に変更
{
        GlyphGenerator glyph_generator( values, nvariables,
                coordinates, ncoords, connections, ncells, celltype); 
     
        glyph_generator.OutputGlyph( time_step);
}

void GeneratePlotOverLine( const int time_step,
                           const kvs::UnstructuredVolumeObject* volume,
                           PlotOverLine* plot_over_line  ) 
{
       if(plot_over_line->plot_flag())
       {
           plot_over_line->extractPlotLine( volume );
           plot_over_line->CellTypeReduceing();
       } 

}

void callPlotOverLine( int time_step,
                             domain_parameters dom, 
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells,
                             const  pbvr::VolumeObjectBase::CellType& celltype , PlotOverLine* plot_over_line )  
{

        kvs::UnstructuredVolumeObject* object = new kvs::UnstructuredVolumeObject;
        object -> setNNodes(ncoords);
        object -> setNCells(ncells);
        object -> setVeclen(nvariables);
        std::vector<float> Values;
        Values.resize(ncoords * nvariables);
        for (int i =0; i<nvariables ; i++) 
        {    
            for (int k=0; k< ncoords; k++) 
            {    
                Values[k+i*ncoords] = values[i][k];
            }    
        }    

        kvs::AnyValueArray Var(Values);
        object -> setValues(Var);
        kvs::ValueArray<float> Coords(coordinates, ncoords*3);
        object -> setCoords(Coords);

        kvs::ValueArray<kvs::UInt32> Connections;
        switch ( celltype )  // 2次要素は一旦除外
        {
            case 4: // pbvr::VolumeObjectBase::Tetrahedra:
                {
                    int nconnection = ncells * 4;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(kvs::VolumeObjectBase::Tetrahedra);
                    break;
                }
            case 8: //  pbvr::VolumeObjectBase::Hexahedra:
                {
                    int nconnection = ncells * 8;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(kvs::VolumeObjectBase::Hexahedra);
                    break;
                }
            case 6: // pbvr::VolumeObjectBase::Prism:
                {
                    int nconnection = ncells * 6;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(kvs::VolumeObjectBase::Prism);
                    break;
                }
            case 5: //pbvr::VolumeObjectBase::Pyramid:
                {
                    int nconnection = ncells * 5;
                    Connections.allocate(nconnection); // hexのみ
                    for (int i =0; i< nconnection ;i ++)
                    {
                        Connections[i] = connections[i];
                    }
                    object -> setCellType(kvs::VolumeObjectBase::Pyramid);
                    break;
                }
            default:
                {
                    std::cout << "Unsupported cell type." << std::endl;
                    return;
                }
        }

        object -> setConnections(Connections);

        GeneratePlotOverLine(time_step, object, plot_over_line);
}




void OutputParticles(int time_step, int nvariables, pbvr_parameters& particleBase, ParamInfo *param, bool skip_flag)
{
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    int tf_number = particleBase.m_tf_number;
    int nbins = 256;

    if (skip_flag)
    {
    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    kvs::ValueArray<float> coords( particleBase.m_sample_coords );
    kvs::ValueArray<Byte>  colors( particleBase.m_sample_colors );
    kvs::ValueArray<float> normals(particleBase.m_sample_normals );

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
    //add by shimomura 20240614
//    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
//    ss << "_";
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
    particleBase.m_ptcFilePath += ss.str();
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
        kvs::PointObject* point_object = new kvs::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
        point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
            particle_write_thread->join(true);
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename(particleBase.m_ptcFilePath.c_str());
            particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
            particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
            particle_write_thread->work(true);
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( point_object );
            kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleBase.m_ptcFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }
    
    }// end if skip_flag
    else 
    {
            std::ofstream ofs( particleBase.m_stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< 0 <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
    }

//    timer.stop();
//    time.writting = timer.sec();
//    timer.start();

    //static bool parameter_file_opened= particleBase.m_parameter_file_opened;
    static bool parameter_file_opened= true;
    //最大最小値の集計
    if( parameter_file_opened )
    {
        O_min_recv.fill(0x00);
        O_max_recv.fill(0x00);
        C_min_recv.fill(0x00);
        C_max_recv.fill(0x00);

//        if(mpi_rank==0)std::cout<<"MPI_Reduce"<<std::endl;
        MPI_Reduce( particleBase.m_O_min.pointer(), O_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_O_max.pointer(), O_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_min.pointer(), C_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_max.pointer(), C_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );

//        if(mpi_rank==0) std::cout<<"end MPI_Reduce"<<std::endl;

        //ヒストグラムの集計
        o_histogram_recv.fill(0x00);
        MPI_Reduce( particleBase.m_o_histogram.pointer(), o_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );

        c_histogram_recv.fill(0x00);
        MPI_Reduce( particleBase.m_c_histogram.pointer(), c_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    }

//    timer.stop();
//    time.mpi_reduce = timer.sec();
//    timer.start();

    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::ofstream ofs( "state.txt", std::ios::out);
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if (!async_io_enabled){
            std::ofstream ofs( particleBase.m_stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< st_time_step <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
        }
        std::stringstream step;
        step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;

        // 20181226 start 環境変数で指定したファイルパスを使用
        std::string history_file_name = particleBase.m_visParamDir + "history" + step.str() + ".txt";
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
        ofs2<<"N_VARIABLES="<<particleBase.m_nvariables<<std::endl;
        ofs2<<"PARTICLE_DENSITY="<<particleBase.m_particle_density<<std::endl;
        ofs2<<"PARTICLE_LIMIT="<<particleBase.m_particle_limit<<std::endl;
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();

        if (skip_flag)
        {
            // 20181226 start 環境変数で指定したファイルパスを使用
            std::string jupiter_file_name = particleBase.m_visParamDir + particleBase.m_tfFilename + step.str() + ".tf";
            // 20181226 end
            param->write( jupiter_file_name );
        }
    }
//    timer.stop();
//    time.write_text = timer.sec();
//
//    show_timer( time );
    //if(mpi->rank==0)std::cout<<"end generate_particles\n";

}

void state_txt_writer( void )
{
    std::ofstream ofs( "state.txt", std::ios::out);
        if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

        ofs<<"LATEST_STEP=NO_STEP"<<std::endl;

        ofs.close();
}

void ens_OutputParticles(int time_step, int nvariables, pbvr_parameters& particleBase, ParamInfo *param, bool skip_flag)
{
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    int tf_number = particleBase.m_tf_number;
    int nbins = 256;

    if (skip_flag)
    {
    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    kvs::ValueArray<float> coords( particleBase.m_sample_coords );
    kvs::ValueArray<Byte>  colors( particleBase.m_sample_colors );
    kvs::ValueArray<float> normals(particleBase.m_sample_normals );

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
        std::cout << "num_nodes = " << num_nodes  << "numprocs = " << numprocs << ", split_numprocs = " << split_numprocs << std::endl;
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
    //add by shimomura 20240614
//    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
//    ss << "_";
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
    particleBase.m_ptcFilePath += ss.str();
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
        kvs::PointObject* point_object = new kvs::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
        point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
            particle_write_thread->join(true);
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename(particleBase.m_ptcFilePath.c_str());
            particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
            particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
            particle_write_thread->work(true);
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( point_object );
            kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleBase.m_ptcFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }
    delete[] displs;
    delete[] recvcounts;
    }// end if skip_flag

//    timer.stop();
//    time.writting = timer.sec();
//    timer.start();

    //static bool parameter_file_opened= particleBase.m_parameter_file_opened;
    static bool parameter_file_opened= true;
    //最大最小値の集計
    if( parameter_file_opened )
    {
        O_min_recv.fill(0x00);
        O_max_recv.fill(0x00);
        C_min_recv.fill(0x00);
        C_max_recv.fill(0x00);

//        if(mpi_rank==0)std::cout<<"MPI_Reduce"<<std::endl;
        MPI_Reduce( particleBase.m_O_min.pointer(), O_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_O_max.pointer(), O_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_min.pointer(), C_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_max.pointer(), C_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );

//        if(mpi_rank==0) std::cout<<"end MPI_Reduce"<<std::endl;

        //ヒストグラムの集計
        o_histogram_recv.fill(0x00);
        MPI_Reduce( particleBase.m_o_histogram.pointer(), o_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );

        c_histogram_recv.fill(0x00);
        MPI_Reduce( particleBase.m_c_histogram.pointer(), c_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    }

//    timer.stop();
//    time.mpi_reduce = timer.sec();
//    timer.start();
#if 1
    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::ofstream ofs( "state.txt", std::ios::out);
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if (!async_io_enabled){
            std::ofstream ofs( particleBase.m_stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< st_time_step <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
        }
        std::stringstream step;
        step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;

        // 20181226 start 環境変数で指定したファイルパスを使用
        std::string history_file_name = particleBase.m_visParamDir + "history" + step.str() + ".txt";
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
        ofs2<<"N_VARIABLES="<<particleBase.m_nvariables<<std::endl;
        ofs2<<"PARTICLE_DENSITY="<<particleBase.m_particle_density<<std::endl;
        ofs2<<"PARTICLE_LIMIT="<<particleBase.m_particle_limit<<std::endl;
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();

        if (skip_flag)
        {
            // 20181226 start 環境変数で指定したファイルパスを使用
            std::string jupiter_file_name = particleBase.m_visParamDir + particleBase.m_tfFilename + step.str() + ".tf";
            // 20181226 end
            param->write( jupiter_file_name );
        }
    }
#endif
//    timer.stop();
//    time.write_text = timer.sec();
//
//    show_timer( time );
    //if(mpi->rank==0)std::cout<<"end generate_particles\n";

#if 1
    if (skip_flag)
    {
        //　分散のファイル出力
        ///-------------------------------------//
        ///--------分散配列をファイル出力----------//
        //--------------------------------------//
        kvs::ValueArray<float> coords( particleBase.m_varience_coords );
        kvs::ValueArray<Byte>  colors( particleBase.m_varience_colors );
        kvs::ValueArray<float> normals(particleBase.m_varience_normals );

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
            std::cout << "num_nodes = " << num_nodes  << "numprocs = " << numprocs << ", split_numprocs = " << split_numprocs << std::endl;
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
        //add by shimomura 20240614
        //    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
        //    ss << "_";
        ss << std::setfill('0') << std::setw(5) << time_step;
        ss << "_";
        ss << std::setfill('0') << std::setw(7) << count;
        ss << "_";
        ss << std::setfill('0') << std::setw(7) << num_nodes;
        ss << ".kvsml";
        particleBase.m_varFilePath = particleBase.m_visParamDir + "particle_out/var_";
        particleBase.m_varFilePath += ss.str();
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
            kvs::PointObject* point_object = new kvs::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
            point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
            // If async_io is enabled, use worker thread to write kvsml data and state.txt
            if (async_io_enabled){
                pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
                particle_write_thread->join(true);
                particle_write_thread->setPointObject( point_object );
                //particle_write_thread->setFilename(particleBase.m_ptcFilePath.c_str());
                particle_write_thread->setFilename(particleBase.m_varFilePath.c_str());
                particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
                particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
                particle_write_thread->work(true);
            }// If async_io is disabled, use kvs::PointExporter here in main thread.
            else{
                kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( point_object );
                kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
                kvsml_object->write( particleBase.m_varFilePath.c_str() );
                delete kvsml_object;
            }
            delete point_object;
        }
    }
#endif
#if  0
    if (skip_flag)
    {
    //　歪度のファイル出力
    ///-------------------------------------//
    ///--------歪度配列をファイル出力----------//
    //--------------------------------------//
    kvs::ValueArray<float> coords( particleBase.m_skewness_coords );
    kvs::ValueArray<Byte>  colors( particleBase.m_skewness_colors );
    kvs::ValueArray<float> normals(particleBase.m_skewness_normals );

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
        std::cout << "num_nodes = " << num_nodes  << "numprocs = " << numprocs << ", split_numprocs = " << split_numprocs << std::endl;
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
    //add by shimomura 20240614
//    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
//    ss << "_";
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << num_nodes;
    ss << ".kvsml";
//    particleBase.m_ptcFilePath += ss.str();
    particleBase.m_skeFilePath = particleBase.m_visParamDir + "particle_out/ske_";
    particleBase.m_skeFilePath += ss.str();
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
        kvs::PointObject* point_object = new kvs::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
        point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
        if (async_io_enabled){
            pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
            particle_write_thread->join(true);
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename(particleBase.m_skeFilePath.c_str());
            particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
            particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
            particle_write_thread->work(true);
        }// If async_io is disabled, use kvs::PointExporter here in main thread.
        else{
            kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( point_object );
            kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleBase.m_skeFilePath.c_str() );
            delete kvsml_object;
        }
        delete point_object;
    }
    }
#endif
}

void ensemble_generate_particles( int time_step, domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype )
{
    static ParamInfo param;
    pbvr_parameters particleBase;
    bool skip_flag;
    skip_flag = SetParameter(dom, &particleBase, &param, time_step);
    skip_flag = true;
    particleBase.m_nvariables = nvariables; 
        EnsembleGenerateParticles(time_step, dom, values,
            nvariables, coordinates, ncoords,
            connections, ncells, celltype, particleBase);

    ens_OutputParticles(time_step, nvariables, particleBase, &param, skip_flag);
}


void ensemble_test( int time_step, domain_parameters dom,
                             Type** values, int nvariables,
                             float* coordinates, int ncoords,
                             unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype )
{
    static ParamInfo param;
    pbvr_parameters particleBase;
    bool skip_flag;
    skip_flag = SetParameter(dom, &particleBase, &param, time_step);
    skip_flag = true;
    particleBase.m_nvariables = nvariables; 
        EnsembleTest(time_step, dom, values,
            nvariables, coordinates, ncoords,
            connections, ncells, celltype, particleBase);

//    ens_OutputParticles(time_step, nvariables, particleBase, &param, skip_flag);
}

kvs::Vector3f RandomSamplingInCube( const kvs::Vector3f vertex, kvs::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();
    const kvs::Vector3f d( x, y, z );

    return vertex + d;
}

void uniform_sampling(
    std::vector< pbvr::CellBase<Type>* > cell,
    Ensembleparameters& ens_param,
    pbvr_parameters& particleBase)
{
    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;
    int   subpixel_level           = particleBase.m_subpixel_level          ;
    float particle_density         = particleBase.m_particle_density        ;

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    int ncells = ens_param.ncells;
    int MPIprocess_per_ensemble = ens_param.MPIprocess_per_ensemble;
    std::vector<kvs::Real32>& vertex_coords  = ens_param.vertex_coords ;
    std::vector<kvs::Real32>& vertex_scalars = ens_param.vertex_scalars;
    std::vector<kvs::Real32>& vertex_normals = ens_param.vertex_normals;
    std::vector<int>& vertex_cellids         = ens_param.vertex_cellids;

    const int local_rank = mpi_rank % MPIprocess_per_ensemble;

    std::vector<kvs::Real32> per_thread_coords [max_threads];
    std::vector<kvs::Real32> per_thread_scalars[max_threads];
    std::vector<kvs::Real32> per_thread_normals[max_threads];
    std::vector<int>         per_thread_cellids[max_threads];

    int nparticles[ ncells ];
    kvs::Timer timer( kvs::Timer::Start );
#pragma omp parallel
{    
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif
        //配列の追加
        kvs::Vector3f local_center_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_center_array[ SIMD_BLK_SIZE ];
        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];

        float cell_opacity_array[ SIMD_BLK_SIZE ];
        int nparticles_array[ SIMD_BLK_SIZE ];
        int th_total_particles =0;

        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
        float density_array[ SIMD_BLK_SIZE ];
        float volume_array[ SIMD_BLK_SIZE ];


    kvs::MersenneTwister MT( local_rank * nthreads );
//    std::vector<kvs::Real32> th_vertex_coords;
//    std::vector<kvs::Real32> th_vertex_scalars;
//    std::vector<kvs::Real32> th_vertex_normals;
//    std::vector<int>         th_vertex_cellids;

    float time1=0, time2 =0, time3 =0, time4 = 0, time5 = 0;
    float timeN[20]= {0};
    kvs::Timer th_timer( kvs::Timer::Start );
#if 1
//#pragma omp for schedule( dynamic ) nowait
#pragma omp for schedule(static) 
    for ( size_t index = 0; index < ncells; index += SIMD_BLK_SIZE )
    {
           th_timer.start();
           //ブロック内でのループ回数を取得
            int remain = ( ncells - index > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: ncells - index;

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
//            #pragma omp simd
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(index + cell_BLK);
            }

            th_timer.stop();
            timeN[0] += th_timer.sec();
            th_timer.start();

            cell[thid]->bindCellArray( remain, cell_index );

            th_timer.stop();
            timeN[1] += th_timer.sec();
            th_timer.start();
            
            cell[thid]->volumeArray( remain, volume_array);
            th_timer.stop();
            timeN[2] += th_timer.sec();
            th_timer.start();

            //生成粒子数を計算
            #pragma simd
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    nparticles_array[cell_BLK] 
                        = calculate_number_of_particles( max_density, volume_array[cell_BLK], &MT ) *  particle_density * ens_param.repitetion;
                        //=  size_t (max_density * volume_array[cell_BLK]) *  particle_density ;
//                        = 10;
                        //if (thid == 0)std::cout << "mpi_rank = " << mpi_rank << ", nparticles_array[cell_BLK] = " << nparticles_array[cell_BLK] << std::endl; 
            }
            th_timer.stop();
            timeN[3] += th_timer.sec();
            // アンサンブル粒子データを作成
            int p_id = 0;
            float p_x_l[remain], p_y_l[remain], p_z_l[remain];
            float p_x_g[remain], p_y_g[remain], p_z_g[remain];
            for(int cell_BLK = 0; cell_BLK < remain+1; cell_BLK++ )
            {
                const int nparticles_I  = cell_BLK<remain ? nparticles_array[cell_BLK] : 1;
                // ------------------------------------------------
                
                for( int i = 0; i < nparticles_I; i+=SIMD_BLK_SIZE )
                {
                    th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( nparticles_I - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_I - i;
//                    const int zero_id = cell_BLK<SIMDW ? SIMDW : p_id;

                    th_timer.stop();
                    timeN[4] += th_timer.sec();
                    if( cell_BLK < remain )
                    {
#pragma simd
                   for( int j = 0; j < remain_BLK; j++ ) 
                   {
                       th_timer.start();
                       cell_index[p_id] = index + cell_BLK;
                       th_timer.stop();
                       timeN[4] += th_timer.sec();
                       th_timer.start();

                       local_coord_array[p_id] = cell[thid] -> randomSampling_MT( &MT );
                       // デバッグ用の座標固定
//                        local_coord_array[p_id] = kvs::Vector3f (0,0,0);
                       th_timer.stop();
                       timeN[5] += th_timer.sec();
                       p_id ++;
                       if(p_id == SIMD_BLK_SIZE )
                       {
                           th_timer.start();

                           //補間器にセルを一括でバインド
                           cell[thid]->bindCellArray( p_id, cell_index );
                           th_timer.stop();
                           timeN[6] += th_timer.sec();
                           th_timer.start();
                           cell[thid]->setLocalPointArray(  p_id, local_coord_array );

                           th_timer.stop();
                           timeN[7] += th_timer.sec();
                           th_timer.start();
                           float scalar_array[p_id];
                           float grad_array_x[p_id];
                           float grad_array_y[p_id];
                           float grad_array_z[p_id];

                           //                    cell[thid]->CalcScalarGrad( p_id,
                           //                            scalar_array,
                           //                            grad_array_x,
                           //                            grad_array_y,
                           //                            grad_array_z );

                           cell[thid] -> scalar_ary( scalar_array, p_id);
                           th_timer.stop();
                           timeN[8] += th_timer.sec();
                           th_timer.start();
                           cell[thid] -> grad_ary( grad_array_x, grad_array_y, grad_array_z, p_id);

                           th_timer.stop();
                           timeN[9] += th_timer.sec();
                           th_timer.start();
                           // resize による最適化テスト
                           size_t base_c = per_thread_coords [thid].size();
                           size_t base_n = per_thread_normals[thid].size();
                           size_t base_s = per_thread_scalars[thid].size();
                           size_t base_id= per_thread_cellids[thid].size();

                           per_thread_coords [thid].resize(base_c + p_id * 3);
                           per_thread_normals[thid].resize(base_n + p_id * 3);
                           per_thread_scalars[thid].resize(base_s + p_id);
                           per_thread_cellids[thid].resize(base_id + p_id);

#pragma simd
                           for( int j = 0; j < p_id; j++ ) 
                           {
                               // resize による最適化テスト
                               size_t c = base_c + 3*j;
                               per_thread_coords[thid][c+0] = local_coord_array[j].x();
                               per_thread_coords[thid][c+1] = local_coord_array[j].y();
                               per_thread_coords[thid][c+2] = local_coord_array[j].z();
                               
                               per_thread_scalars[thid][base_s + j] = scalar_array[j];

                               size_t n = base_n + 3*j;
                               per_thread_normals[thid][n+0] = grad_array_x[j];
                               per_thread_normals[thid][n+1] = grad_array_y[j];
                               per_thread_normals[thid][n+2] = grad_array_z[j];

                               per_thread_cellids[thid][base_id + j] = cell_index[j];

                           }
                           th_timer.stop();
                           timeN[10] += th_timer.sec();
                           p_id = 0;
                       } 
                   }
                    }// end if cell_BLK < remain 
                    if(cell_BLK == remain)  // p_id ==SIMD_BLK と一緒にしたかったが、無理っぽい
                    {
                        th_timer.start();

                    //補間器にセルを一括でバインド
                    cell[thid]->bindCellArray( p_id, cell_index );
                    th_timer.stop();
                    timeN[6] += th_timer.sec();
                    th_timer.start();
                    cell[thid]->setLocalPointArray(  p_id, local_coord_array );
                    
                    th_timer.stop();
                    timeN[7] += th_timer.sec();
                    th_timer.start();
                    float scalar_array[p_id];
                    float grad_array_x[p_id];
                    float grad_array_y[p_id];
                    float grad_array_z[p_id];

//                    cell[thid]->CalcScalarGrad( p_id,
//                            scalar_array,
//                            grad_array_x,
//                            grad_array_y,
//                            grad_array_z );
                        
                    cell[thid] -> scalar_ary( scalar_array, p_id);
                    th_timer.stop();
                    timeN[8] += th_timer.sec();
                    th_timer.start();
                    cell[thid] -> grad_ary( grad_array_x, grad_array_y, grad_array_z, p_id);

                    th_timer.stop();
                    timeN[9] += th_timer.sec();
                    th_timer.start();
// resize による最適化テスト
                    size_t base_c = per_thread_coords [thid].size();
                    size_t base_n = per_thread_normals[thid].size();
                    size_t base_s = per_thread_scalars[thid].size();
                    size_t base_id= per_thread_cellids[thid].size();

                    per_thread_coords [thid].resize(base_c + p_id * 3);
                    per_thread_normals[thid].resize(base_n + p_id * 3);
                    per_thread_scalars[thid].resize(base_s + p_id);
                    per_thread_cellids[thid].resize(base_id + p_id);

#pragma simd
                    for( int j = 0; j < p_id; j++ ) 
                    {
// resize による最適化テスト
                        size_t c = base_c + 3*j;
                        per_thread_coords[thid][c+0] = local_coord_array[j].x();
                        per_thread_coords[thid][c+1] = local_coord_array[j].y();
                        per_thread_coords[thid][c+2] = local_coord_array[j].z();

                        per_thread_scalars[thid][base_s + j] = scalar_array[j];

                        size_t n = base_n + 3*j;
                        per_thread_normals[thid][n+0] = grad_array_x[j];
                        per_thread_normals[thid][n+1] = grad_array_y[j];
                        per_thread_normals[thid][n+2] = grad_array_z[j];

                        per_thread_cellids[thid][base_id + j] = cell_index[j];

                    }
                        th_timer.stop();
                        timeN[10] += th_timer.sec();
                    p_id = 0;
                    }


                } // end for Nparticle_I 
            }
    }
#endif
//#pragma omp barrier
//#pragma omp critical
//            {
//                for(int i =0; i <  7; i++ )
//                    timeN[i] += th_timeN[i]/nthreads;    
//            }


#pragma omp barrier
#pragma omp critical
        for (int i =0;i < 11; i++)
        {
            std::cout << mpi_rank <<  ": ave_sampling_time["<< i <<"] =" << timeN[i] << std::endl;
        }
}  //end omp loop
timer.stop();
    std::cout << mpi_rank <<  ": uniform_sampling_time =" << timer.sec() << std::endl;
timer.start();

const size_t base_coords   = vertex_coords.size();
const size_t base_scalars  = vertex_scalars.size();
const size_t base_normals  = vertex_normals.size();
const size_t base_cellids  = vertex_cellids.size();

size_t add_coords  = 0, add_scalars = 0, add_normals = 0, add_cellids = 0;

for (int t = 0; t < max_threads; ++t) 
{
    add_coords  += per_thread_coords[t].size();
    add_scalars += per_thread_scalars[t].size();
    add_normals += per_thread_normals[t].size();
    add_cellids += per_thread_cellids[t].size();
}

vertex_coords.resize(base_coords + add_coords);
vertex_scalars.resize(base_scalars + add_scalars);
vertex_normals.resize(base_normals + add_normals);
vertex_cellids.resize(base_cellids + add_cellids);

//方法1
size_t off_c = base_coords;
size_t off_s = base_scalars;
size_t off_n = base_normals;
size_t off_cl = base_cellids;
for (int t = 0; t < max_threads; ++t) 
{
    const auto& src_c = per_thread_coords[t];
    const auto& src_s = per_thread_scalars[t];
    const auto& src_n = per_thread_normals[t];
    const auto& src_cl= per_thread_cellids[t];
    // 型が同じなら std::copy が最速寄り
    std::copy(src_c.begin(), src_c.end(), vertex_coords.begin() + off_c);
    off_c += src_c.size();
    std::copy(src_s.begin(), src_s.end(), vertex_scalars.begin() + off_s);
    off_s += src_s.size();
    std::copy(src_n.begin(), src_n.end(), vertex_normals.begin() + off_n);
    off_n += src_n.size();
    std::copy(src_cl.begin(), src_cl.end(), vertex_cellids.begin() + off_cl);
    off_cl += src_cl.size();
}

//方法2
//vertex_coords.reserve (base_coords  + add_coords);
//vertex_scalars.reserve(base_scalars + add_scalars);
//vertex_normals.reserve(base_normals + add_normals);
//vertex_cellids.reserve(base_cellids + add_cellids);
//
//for (int t = 0; t < max_threads; ++t) {
//    vertex_coords .insert(vertex_coords.end(),
//                          per_thread_coords[t].begin(),  per_thread_coords[t].end());
//    vertex_scalars.insert(vertex_scalars.end(),
//                          per_thread_scalars[t].begin(), per_thread_scalars[t].end());
//    vertex_normals.insert(vertex_normals.end(),
//                          per_thread_normals[t].begin(), per_thread_normals[t].end());
//    vertex_cellids.insert(vertex_cellids.end(),
//                          per_thread_cellids[t].begin(), per_thread_cellids[t].end());
//}

timer.stop();

    std::cout << mpi_rank <<  ": ave_sampling_time[11] =" << timer.sec() << std::endl;
    std::cout << mpi_rank <<  ": uniform_nparticles : " <<  vertex_scalars.size()  << " particle_density = " << particle_density << std::endl;
}

void ensemble_reduce_scatter(
    std::vector< pbvr::CellBase<Type>* > cell,
    Ensembleparameters& ens_param, 
    pbvr_parameters& particleBase
)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    float timeN[20]= {0};
    kvs::Timer timer( kvs::Timer::Start );
    int ens_number = ens_param.ensemble_number;
    // 送信データの個数 
    const int local_size = ens_param.vertex_scalars.size(); 
//    int recv_size = 0;
    int recv_size = local_size;

    //プロセス間で粒子数は同じという前提のもと、
    //　不安制定を犠牲にallreduce をやめる
    // reduce演算のためにデータサイズを最小粒子数に揃える
//    MPI_Allreduce(&local_size, &recv_size, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD); 

    std::vector<float>& vertex_scalars = ens_param.vertex_scalars;
    std::vector<float>& vertex_normals = ens_param.vertex_normals;
    std::vector<float>& vertex_coords  = ens_param.vertex_coords;
    std::vector<int>& vertex_cellids   = ens_param.vertex_cellids;

    timer.stop();
    timeN[0] += timer.sec();
    timer.start();
    // debug
//    for (int i = 0 ; i< vertex_scalars.size() ; i+= 1000000)
//    {
//        std::cout << mpi_rank << ", vertex_scalars["<<i<<"] = " << vertex_scalars [i] << "cell_id["<<i<<"] = " << ens_param.vertex_cellids[i] << ", coords = " << ens_param.vertex_coords[3*i] << ", " << ens_param.vertex_coords[3*i+1] << ", " << ens_param.vertex_coords[3*i+2] << std::endl;
//    }

    // vertex_scalars , normal, cellid, coordをアンサンブル数で等分する 
    std::vector<float> div_vertex_scalars;
    std::vector<float> div_vertex_normals;
    std::vector<float> div_vertex_coords;
    std::vector<int> div_vertex_cellids;
    int scalars_total = recv_size;
    int normals_total = 3*recv_size;
    int scalars_baseSize = scalars_total / ens_number;
    //int normals_baseSize = normals_total / ens_number;
    int normals_baseSize = 3*scalars_baseSize;
    int scalars_remainder = scalars_total % ens_number;
    //int normals_remainder = normals_total % ens_number;
//    int normals_remainder = 3*scalars_remainder;

    int scalars_start = 0;
    int normals_start = 0;
#pragma omp for
    for (int i = 0; i < ens_number; i++) {
        int scalars_chunkSize = scalars_baseSize + (i < scalars_remainder ? 1 : 0);
        //int normals_chunkSize = normals_baseSize + (i < normals_remainder ? 3 : 0);
        int normals_chunkSize = normals_baseSize + (i < scalars_remainder ? 3 : 0);

//        result.push_back(
        if(i == mpi_rank)
        {
            div_vertex_cellids.insert( div_vertex_cellids.end() , vertex_cellids.begin()  + scalars_start, vertex_cellids.begin() + scalars_start + scalars_chunkSize);
            div_vertex_coords .insert( div_vertex_coords .end() , vertex_coords.begin() + normals_start, vertex_coords.begin()  + normals_start + normals_chunkSize);
        }

        scalars_start += scalars_chunkSize;
        normals_start += normals_chunkSize;
    }
    
    vertex_cellids = div_vertex_cellids;
    vertex_coords  = div_vertex_coords;

    // debug
//    for (int i = 0 ; i< div_vertex_cellids.size() ; i+= 1000000)
//    {
//        std::cout << mpi_rank << ", vertex_scalars["<<i<<"] = " << vertex_scalars [i] << "cell_id["<<i<<"] = " << div_vertex_cellids[i] << ", coords = " << ens_param.vertex_coords[3*i] << ", " << ens_param.vertex_coords[3*i+1] << ", " << ens_param.vertex_coords[3*i+2] << std::endl;
//    }

    std::vector<int> scalars_counts(ens_number, scalars_total / ens_number);
#pragma omp for
    for (int r = 0; r < scalars_total % ens_number; r++) scalars_counts[r]++;  // 余りを前から配る

    std::vector<int> normals_counts(ens_number, normals_total/ ens_number);
#pragma omp for
    for (int r = 0; r < scalars_total % ens_number; r++) normals_counts[r]+=3;  // 余りを前から配る

    timer.stop();
    timeN[1] += timer.sec();
    timer.start();
    // 受信バッファ
    // 変数値
//    std::vector<float> recv_scalars(recv_size); 
    std::vector<float> recv_scalars(scalars_counts[mpi_rank]); 
    // 法線
//    std::vector<float> recv_normals(3*recv_size);
    std::vector<float> recv_normals(normals_counts[mpi_rank]);

    // 分散用配列
    std::vector<float> varience(scalars_counts[mpi_rank]);
    std::vector<float> tmp_term(3*recv_size);
    std::vector<float> varience_normals(normals_counts[mpi_rank]);
        // 二乗
    std::vector<float> sq_scalars(recv_size);
    std::vector<float> recv_sq_scalars(scalars_counts[mpi_rank]);
    std::vector<float> recv_tmp_term(normals_counts[mpi_rank]);

    // 二乗の計算
#pragma simd
    for (int i =0 ; i< recv_size ; i++ )
    {
        sq_scalars[i]   = vertex_scalars[i] * vertex_scalars[i];
        tmp_term[3*i+0] = vertex_scalars[i] * vertex_normals[3*i+0]; 
        tmp_term[3*i+1] = vertex_scalars[i] * vertex_normals[3*i+1];  
        tmp_term[3*i+2] = vertex_scalars[i] * vertex_normals[3*i+2];
    }

    timer.stop();
    timeN[2] += timer.sec();
    timer.start();
     MPI_Barrier(MPI_COMM_WORLD);
//     std::cout << "vertex_scalars.size() = " << vertex_scalars.size() << ", recv_scalars = " << recv_scalars.size() << ", scalars_counts[0] = " << scalars_counts[0] << ", scalars_counts[1] = " << scalars_counts[1] <<std::endl;
    // MPI_reduce による変数、法線の合算
    MPI_Reduce_scatter(vertex_scalars.data(), recv_scalars.data(), scalars_counts.data(), MPI_FLOAT,
            MPI_SUM, MPI_COMM_WORLD);

    MPI_Reduce_scatter(sq_scalars.data(), recv_sq_scalars.data(), scalars_counts.data(), MPI_FLOAT,
            MPI_SUM, MPI_COMM_WORLD);

    MPI_Reduce_scatter(vertex_normals.data(), recv_normals.data(), normals_counts.data(), MPI_FLOAT,
            MPI_SUM, MPI_COMM_WORLD);
    
    MPI_Reduce_scatter(tmp_term.data(), recv_tmp_term.data(), normals_counts.data(), MPI_FLOAT,
            MPI_SUM, MPI_COMM_WORLD);
 
    timer.stop();
    timeN[3] += timer.sec();
    timer.start();
    // mpi_rank = 0 にて 平均値、分散計算処理
    // 平均値計算
    const float mpi_size_inv = 1 /float(mpi_size);  // 1アンサンブル 1MPIプロセスと仮定
    //        const float mpi_size_inv = ens_param.MPIprocess_per_ensemble /mpi_size;
#pragma simd
    for (int i =0 ; i< scalars_counts[mpi_rank] ; i++ )
    {
        recv_scalars[i]    *= mpi_size_inv;
        recv_sq_scalars[i] *= mpi_size_inv;
    }

#pragma simd
    for (int i =0 ; i< normals_counts[mpi_rank] ; i++ )
    {
        recv_normals[i] *= -mpi_size_inv;
        recv_tmp_term[i] *= mpi_size_inv; 
    }

    timer.stop();
    timeN[4] += timer.sec();
    timer.start();
    // 分散
#pragma simd
    for (int i =0 ; i< scalars_counts[mpi_rank] ; i++ )
    {
        varience[i] = recv_sq_scalars[i] - recv_scalars[i]* recv_scalars[i] ;     
    }        

#pragma simd
    for (int i =0 ; i< normals_counts[mpi_rank] ; i++ )
    {
        varience_normals[i] = - 2 * recv_tmp_term[i] + 2*recv_scalars[i] *recv_normals[i] ;     
    }        

    timer.stop();
    timeN[5] += timer.sec();
    timer.start();
    ens_param.average_scalars = recv_scalars;     
    ens_param.average_normals = recv_normals;     
    ens_param.varience_scalars= varience    ;
    ens_param.varience_normals= varience_normals;

    timer.stop();
    timeN[6] += timer.sec();
    timer.start();

    //for (int i = 0 ; i< vertex_scalars.size() ; i+= 1000000)
//    for (int i = 0 ; i< vertex_scalars.size() ; i+= 1)
//    {
//        std::cout << mpi_rank << ", varience["<<i<<"] = " << varience [i] << ", average_scalars["<<i<<"] = " << recv_scalars[i] << ", vertex_scalars["<<i<<"] = " << vertex_scalars[i] <<  ", coords = " << ens_param.vertex_coords[3*i] << ", " << ens_param.vertex_coords[3*i+1] << ", " << ens_param.vertex_coords[3*i+2] << std::endl;
//    }
    for (int i =0;i < 7; i++)
    {
        std::cout << mpi_rank <<  ": reduce_scatter_time["<< i <<"] =" << timeN[i] << std::endl;
    }


}

void ensemble_reduce(
    std::vector< pbvr::CellBase<Type>* > cell,
    Ensembleparameters& ens_param, 
    pbvr_parameters& particleBase
)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    float timeN[20]= {0};
    kvs::Timer timer( kvs::Timer::Start );

    // 送信データの個数 
    const int local_size = ens_param.vertex_scalars.size(); 
    int recv_size = 0;
    // reduce演算のためにデータサイズを最小粒子数に揃える
    MPI_Allreduce(&local_size, &recv_size, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD); 

    std::vector<float>& vertex_scalars = ens_param.vertex_scalars;
    std::vector<float>& vertex_normals = ens_param.vertex_normals;

    timer.stop();
    timeN[0] += timer.sec();
    timer.start();
    // debug
//    for (int i = vertex_scalars.size() -10; i< vertex_scalars.size() ; i++)
//    for (int i = 0 ; i< vertex_scalars.size() ; i+= 1000000)
//    {
//        std::cout << mpi_rank << ", vertex_scalars["<<i<<"] = " << vertex_scalars [i] << "cell_id["<<i<<"] = " << ens_param.vertex_cellids[i] << ", coords = " << ens_param.vertex_coords[3*i] << ", " << ens_param.vertex_coords[3*i+1] << ", " << ens_param.vertex_coords[3*i+2] << std::endl;
//    }

    // 受信バッファ
    // 変数値
    std::vector<float> recv_scalars(recv_size); 
    // 法線
    std::vector<float> recv_normals(3*recv_size);
//    std::vector<float> sq_normals;

    // 分散用配列
    std::vector<float> varience(recv_size);
    std::vector<float> tmp_term(3*recv_size);
    std::vector<float> varience_normals(3*recv_size);
        // 二乗
    std::vector<float> sq_scalars(recv_size);
    std::vector<float> recv_sq_scalars(recv_size);
    std::vector<float> recv_tmp_term(3*recv_size);

    // 二乗の計算
#pragma simd 
    for (int i =0 ; i< recv_size ; i++ )
    {
        sq_scalars[i]   = vertex_scalars[i] * vertex_scalars[i];
        tmp_term[3*i+0] = vertex_scalars[i] * vertex_normals[3*i+0]; 
        tmp_term[3*i+1] = vertex_scalars[i] * vertex_normals[3*i+1];  
        tmp_term[3*i+2] = vertex_scalars[i] * vertex_normals[3*i+2];
    }


    timer.stop();
    timeN[1] += timer.sec();
    timer.start();
     MPI_Barrier(MPI_COMM_WORLD);
     std::cout << "vertex_scalars.size() = " << vertex_scalars.size() << ", recv_scalars = " << recv_scalars.size() << ", local_size = " << local_size <<std::endl;
    // MPI_reduce による変数、法線の合算
    MPI_Reduce(vertex_scalars.data(), recv_scalars.data(), recv_size, MPI_FLOAT,
            MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Reduce(sq_scalars.data(), recv_sq_scalars.data(), recv_size, MPI_FLOAT,
            MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Reduce(vertex_normals.data(), recv_normals.data(), 3*recv_size, MPI_FLOAT,
            MPI_SUM, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(tmp_term.data(), recv_tmp_term.data(), 3*recv_size, MPI_FLOAT,
            MPI_SUM, 0, MPI_COMM_WORLD);
 
    timer.stop();
    timeN[2] += timer.sec();
    timer.start();
    // mpi_rank = 0 にて 平均値、分散計算処理
    if(mpi_rank == 0)
    {
        // 平均値計算
        const float mpi_size_inv = 1 /float(mpi_size);  // 1アンサンブル 1MPIプロセスと仮定
//        const float mpi_size_inv = ens_param.MPIprocess_per_ensemble /mpi_size;
#pragma simd 
        for (int i =0 ; i< recv_size ; i++ )
        {
            recv_scalars[i]    *= mpi_size_inv;
            recv_sq_scalars[i] *= mpi_size_inv;
        }

#pragma simd 
        for (int i =0 ; i< 3*recv_size ; i++ )
        {
            recv_normals[i] *= -mpi_size_inv;
            recv_tmp_term[i] *= mpi_size_inv; 
        }

    timer.stop();
    timeN[3] += timer.sec();
    timer.start();
        // 分散
#pragma simd 
        for (int i =0 ; i< recv_size ; i++ )
        {
            varience[i] = recv_sq_scalars[i] - recv_scalars[i]* recv_scalars[i] ;     
        }        

#pragma simd 
        for (int i =0 ; i< 3*recv_size ; i++ )
        {
            varience_normals[i] = - 2 * recv_tmp_term[i] +  2*recv_scalars[i] *recv_normals[i] ;     
        }        

    timer.stop();
    timeN[4] += timer.sec();
    timer.start();
         ens_param.average_scalars = recv_scalars;     
         ens_param.average_normals = recv_normals;     
         ens_param.varience_scalars= varience    ;
         ens_param.varience_normals= varience_normals;

    timer.stop();
    timeN[5] += timer.sec();
//         for (int i = 0 ; i< vertex_scalars.size() ; i+= 1000000)
//         {
//             std::cout << mpi_rank << ", vertex_scalars["<<i<<"] = " << vertex_scalars [i] << "cell_id["<<i<<"] = " << ens_param.vertex_cellids[i] << ", coords = " << ens_param.vertex_coords[3*i] << ", " << ens_param.vertex_coords[3*i+1] << ", " << ens_param.vertex_coords[3*i+2] << std::endl;
//         }
//    for (int i = 0 ; i< vertex_scalars.size() ; i+= 1)
//    {
//        std::cout << mpi_rank << ", varience["<<i<<"] = " << varience [i] << ", average_scalars["<<i<<"] = " << recv_scalars[i] << ", vertex_scalars["<<i<<"] = " << vertex_scalars[i] <<  ", coords = " << ens_param.vertex_coords[3*i] << ", " << ens_param.vertex_coords[3*i+1] << ", " << ens_param.vertex_coords[3*i+2] << std::endl;
//    }

    } 
    for (int i =0;i < 6; i++)
    {
        std::cout << mpi_rank <<  ": reduce_time["<< i <<"] =" << timeN[i] << std::endl;
    }


}

void rejection_process(
    std::vector< pbvr::CellBase<Type>* > cell,
    Ensembleparameters& ens_param, 
    pbvr_parameters& particleBase
)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    float timeN[20]= {0};
    kvs::Timer timer( kvs::Timer::Start );
    auto tf = particleBase.m_tf[0];

    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;

    //棄却後用配列
    std::vector<kvs::Real32> result_average_coords;
    std::vector<kvs::UInt8>  result_average_colors;
    std::vector<kvs::Real32> result_average_normals; 

    std::vector<kvs::Real32> result_varience_coords;
    std::vector<kvs::UInt8>  result_varience_colors;
    std::vector<kvs::Real32> result_varience_normals; 

    const int local_size = ens_param.average_scalars.size(); 
    std::vector<float>& vertex_scalars = ens_param.vertex_scalars;
    std::vector<float>& vertex_normals = ens_param.vertex_normals;
    std::vector<float>& vertex_coords  = ens_param.vertex_coords;
    std::vector<int>&   vertex_cellids = ens_param.vertex_cellids;

    std::vector<float>& average_scalars = ens_param.average_scalars;
    std::vector<float>& average_normals = ens_param.average_normals;

    std::vector<float>& varience_scalars = ens_param.varience_scalars;
    std::vector<float>& varience_normals = ens_param.varience_normals;
     std::cout << __LINE__ <<std::endl;
    //棄却法を適応する
#if 1

//    for (int i = 0; i < vertex_scalars.size(); i++ )
//    {
//        std::cout << mpi_rank << ", vertex_cellids["<<i<<"] = " << vertex_cellids[i] << std::endl;
//    }

////// 常用対数を取るための計算
// std::vector<float> log_vertex_scalars(vertex_scalars.size());
       float delta = 1e-30;
//       int N = 0; // OutputFuncで正規化したオーダー
//#pragma omp simd
//       for (int i = 0; i < vertex_scalars.size(); i++ )
//       {
////           log_vertex_scalars[ i ]     = std::log10(vertex_scalars[ i ]); 
////           log_vertex_scalars[ i ]     =  vertex_scalars[ i ] > delta ? std::log10(vertex_scalars[ i ]) : -30; 
//           log_vertex_scalars[ i ]     =  vertex_scalars[ i ] > delta ? std::log10(vertex_scalars[ i ]) -N : -30; 
//       }

    timer.stop();
    timeN[0] += timer.sec();
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
//
    kvs::MersenneTwister MT( 10 + thid );
//    kvs::MersenneTwister MT( vertex_scalars.size() +  thid + mpi_rank * nthreads );
    std::vector<kvs::Real32> th_vertex_coords;
    std::vector<kvs::Real32> th_vertex_normals;
    std::vector<kvs::UInt8>  th_vertex_colors;
    std::vector<kvs::Real32>  th_vertex_scalars;
    std::vector<kvs::Real32> th_vertex_varience_coords;
    std::vector<kvs::Real32> th_vertex_varience_normals;
    std::vector<kvs::UInt8>  th_vertex_varience_colors;
    std::vector<int>  th_vertex_cellids;

    kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
    kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
    kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
    float opacity_array_average[ SIMD_BLK_SIZE ];
    float opacity_array_varience[ SIMD_BLK_SIZE ];
    float density_array_average[ SIMD_BLK_SIZE ];
    float density_array_varience[ SIMD_BLK_SIZE ];
    float th_timeN[20]= {0};
    kvs::Timer th_timer( kvs::Timer::Start );

#pragma omp for  
    for(int i =0; i< local_size ;i+= SIMD_BLK_SIZE)
    {
        th_timer.start();
        //ブロック内でのループ回数を取得
        int remain_BLK = ( local_size - i > SIMD_BLK_SIZE )
            ? SIMD_BLK_SIZE: local_size - i;
        // セル登録
#pragma omp simd
        for( int j = 0; j < remain_BLK; j++ ) 
        {
            cell_index[j] = vertex_cellids[i + j];
        }
        cell[thid] -> bindCellArray(remain_BLK, cell_index);

        th_timer.stop();
        th_timeN[0] += th_timer.sec();
        th_timer.start();

        //局所座標の詰め替え
        //            #pragma omp simd
        for( int j = 0; j < remain_BLK; j++ ) 
        {
            local_coord_array[j].x() = vertex_coords[ 3*(i+j)+ 0];
            local_coord_array[j].y() = vertex_coords[ 3*(i+j)+ 1];
            local_coord_array[j].z() = vertex_coords[ 3*(i+j)+ 2];
        }

        th_timer.stop();
        th_timeN[1] += th_timer.sec();
        th_timer.start();
        //座標の登録
        cell[thid] -> setLocalPointArray(remain_BLK,local_coord_array);

        th_timer.stop();
        th_timeN[2] += th_timer.sec();
        th_timer.start();
        //全体座標への変換
        cell[thid] -> transformLocalToGlobalArray(remain_BLK,local_coord_array,global_coord_array);

        th_timer.stop();
        th_timeN[3] += th_timer.sec();
        th_timer.start();

        //opacity 計算 
        for( int j = 0; j < remain_BLK; j++ ) 
        {
            opacity_array_average [j] = tf.opacityMap().at(average_scalars[ i+j ]);
            //                    opacity_array_average [j] = tf.opacityMap().at(vertex_scalars[ i+j ]);
            opacity_array_varience[j] = tf.opacityMap().at(varience_scalars[ i+j ]);
            //                    対数値を参照
            //                    opacity_array[j] = tf.opacityMap().at(log_vertex_scalars[ i+j ]);
        }
        th_timer.stop();
        th_timeN[4] += th_timer.sec();
        th_timer.start();

#pragma omp simd
        for( int j = 0; j < remain_BLK; j++ ) 
        {
            density_array_average [j] = opacity_array_average [j] < max_opacity ? -std::log( 1.0f - opacity_array_average [j] ) * sampling_volume_inverse: max_density;  
            density_array_varience[j] = opacity_array_varience[j] < max_opacity ? -std::log( 1.0f - opacity_array_varience[j] ) * sampling_volume_inverse: max_density;  
        }
        th_timer.stop();
        th_timeN[5] += th_timer.sec();

        for( int j = 0; j < remain_BLK; j++ ) 
        {
            th_timer.start();
            const float R = MT.rand();
            th_timer.stop();
            th_timeN[6] += th_timer.sec();
            if ( density_array_average[j] > max_density * R )
            {
                th_timer.start();
                // Calculate a color.
                const kvs::RGBColor color( tf.colorMap().at( average_scalars[ i+j ] ) );
//                   const kvs::RGBColor color( tf.colorMap().at( vertex_scalars[ i+j ] ) );
//                    対数値を参照
//                   const kvs::RGBColor color( tf.colorMap().at( log_vertex_scalars[ i+j ] ) );
                th_timer.stop();
                th_timeN[7] += th_timer.sec();
                th_timer.start();

                // Calculate a normal.
                th_timer.stop();
                th_timeN[8] += th_timer.sec();
                th_timer.start();

                // set coord, color, and normal to point object( this ).
                th_vertex_coords.push_back( global_coord_array[j].x() );
                th_vertex_coords.push_back( global_coord_array[j].y() );
                th_vertex_coords.push_back( global_coord_array[j].z() );

                th_vertex_colors.push_back( color.r() );
                th_vertex_colors.push_back( color.g() );
                th_vertex_colors.push_back( color.b() );

                th_vertex_normals.push_back( average_normals[3*(i+j)+0] );
                th_vertex_normals.push_back( average_normals[3*(i+j)+1] );
                th_vertex_normals.push_back( average_normals[3*(i+j)+2] );

                th_timer.stop();
                th_timeN[9] += th_timer.sec();
                //                   if( average_scalars[ i+j ] > 100 ) std::cout << "average_scalars[ i+j ] = "  << average_scalars[ i+j ] << ", coords = " << global_coord_array[j] << std::endl;
            }

            if ( density_array_varience[j] > max_density * R )
            {
                th_timer.start();
                // Calculate a color.
                const kvs::RGBColor color( tf.colorMap().at( varience_scalars[ i+j ] ) );
                //                    対数値を参照
                //                   const kvs::RGBColor color( tf.colorMap().at( log_vertex_scalars[ i+j ] ) );
                th_timer.stop();
                th_timeN[10] += th_timer.sec();
                th_timer.start();

                // Calculate a normal.
                th_timer.stop();
                th_timeN[11] += th_timer.sec();
                th_timer.start();

                // set coord, color, and normal to point object( this ).
                th_vertex_varience_coords.push_back( global_coord_array[j].x() );
                th_vertex_varience_coords.push_back( global_coord_array[j].y() );
                th_vertex_varience_coords.push_back( global_coord_array[j].z() );

                th_vertex_varience_colors.push_back( color.r() );
                th_vertex_varience_colors.push_back( color.g() );
                th_vertex_varience_colors.push_back( color.b() );

                th_vertex_varience_normals.push_back( varience_normals[3*(i+j)+0] );
                th_vertex_varience_normals.push_back( varience_normals[3*(i+j)+1] );
                th_vertex_varience_normals.push_back( varience_normals[3*(i+j)+2] );

                th_timer.stop();
                th_timeN[12] += th_timer.sec();

            }
        }
    }
                th_timer.start();
#pragma omp critical
       {
           result_average_coords.insert  (result_average_coords.end() , th_vertex_coords.begin() , th_vertex_coords.end());
           result_average_colors.insert  (result_average_colors.end(), th_vertex_colors.begin() , th_vertex_colors.end());
           result_average_normals.insert (result_average_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end());
           result_varience_coords.insert (result_varience_coords.end() , th_vertex_varience_coords.begin() , th_vertex_varience_coords.end());
           result_varience_colors.insert (result_varience_colors.end() , th_vertex_varience_colors.begin() , th_vertex_varience_colors.end());
           result_varience_normals.insert(result_varience_normals.end(), th_vertex_varience_normals.begin(), th_vertex_varience_normals.end());
       }
                th_timer.stop();
                th_timeN[13] += th_timer.sec();
#pragma omp critical
                {    
                    for (int i =0;i < 14; i++)
                    {
                        std::cout << mpi_rank <<  ": ave_rejection_time["<< i <<"] =" << th_timeN[i] << std::endl;
                    }
                }

}
       timer.stop();
       std::cout << mpi_rank << ": rejection_exe_time =" << timer.sec() << std::endl;
       int size = result_average_coords.size();
#endif

    std::cout << mpi_rank <<  ": nparticles : " <<  size/3   << std::endl;

//    // 平均値データを集約する
    particleBase.m_sample_coords.insert(particleBase.m_sample_coords.end()  , result_average_coords.begin() , result_average_coords.end());
    particleBase.m_sample_colors.insert(particleBase.m_sample_colors.end()  , result_average_colors.begin() , result_average_colors.end());
    particleBase.m_sample_normals.insert(particleBase.m_sample_normals.end(), result_average_normals.begin(), result_average_normals.end());
    particleBase.m_varience_coords.insert( particleBase.m_varience_coords.end()  , result_varience_coords.begin() , result_varience_coords.end());
    particleBase.m_varience_colors.insert( particleBase.m_varience_colors.end()  , result_varience_colors.begin() , result_varience_colors.end());
    particleBase.m_varience_normals.insert(particleBase.m_varience_normals.end() , result_varience_normals.begin(), result_varience_normals.end());

}

void reduce_scatter_ensemble(pbvr::TransferFunction& tf,  std::vector< pbvr::CellBase<Type>* > cell, pbvr_parameters& particleBase ,const  int ncells )
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;
    int   subpixel_level           = particleBase.m_subpixel_level          ;
    float particle_density         = particleBase.m_particle_density        ;
    float particle_data_size_limit = particleBase.m_particle_data_size_limit;
    // repeat level を手動設定
//    int   repetitions             = 24 ; // 4 scalar
//    int   repetitions             = 18 ;
    int   repetitions             = 8 ;
//    parameter_file_opened = particleBase.m_parameter_file_opened;
    // アンサンブル数でrepetitionを徐算
    const int MPIprocess_per_ensemble = 1;  
    int ens_number = mpi_size/MPIprocess_per_ensemble;
//    repetitions /= ens_number;
//    if (repetitions < ens_number ) repetitions = 1;


    //　各粒子の変数データ
    std::vector<kvs::Real32> vertex_coords;
    std::vector<kvs::Real32> vertex_scalars;
    std::vector<kvs::Real32> vertex_normals;
    std::vector<int> vertex_cellids;

    //平均値データ
    std::vector<kvs::Real32> average_coords;
    std::vector<kvs::UInt8>  average_colors;
    std::vector<kvs::Real32> average_scalars;
    std::vector<kvs::Real32> average_normals; 
    std::vector<int>         average_cellids;

    // 分散値データ
    std::vector<kvs::Real32> varience_scalars;
    std::vector<kvs::Real32> varience_normals; 

    // 引数ようにstructにまとめる
	Ensembleparameters ens_param{
        vertex_coords,
            vertex_scalars,
            vertex_normals,
            vertex_cellids,
            average_scalars,
            average_normals,
            varience_scalars,
            varience_normals,
            ncells,
            MPIprocess_per_ensemble,
            ens_number,
            repetitions
    };

    uniform_sampling(cell, 
                     ens_param,
                     particleBase); 

    ensemble_reduce_scatter(cell, 
            ens_param,
            particleBase);

    rejection_process(cell, ens_param, particleBase);
}

void all_reduce_ensemble(pbvr::TransferFunction& tf,  std::vector< pbvr::CellBase<Type>* > cell, pbvr_parameters& particleBase ,const  int ncells )
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;
    int   subpixel_level           = particleBase.m_subpixel_level          ;
    float particle_density         = particleBase.m_particle_density        ;
    float particle_data_size_limit = particleBase.m_particle_data_size_limit;
    // repeat level を手動設定
//    int   repetitions             = 24 ; // 4 scalar
//    int   repetitions             = 18 ;
    int   repetitions             = 8 ;
//    parameter_file_opened = particleBase.m_parameter_file_opened;
    // アンサンブル数でrepetitionを徐算
    const int MPIprocess_per_ensemble = 1;  
    int ens_number = mpi_size/MPIprocess_per_ensemble;
//    repetitions /= ens_number;
//    if (repetitions < ens_number ) repetitions = 1;


    //　各粒子の変数データ
    std::vector<kvs::Real32> vertex_coords;
    std::vector<kvs::Real32> vertex_scalars;
    std::vector<kvs::Real32> vertex_normals;
    std::vector<int> vertex_cellids;

    //平均値データ
    std::vector<kvs::Real32> average_coords;
    std::vector<kvs::UInt8>  average_colors;
    std::vector<kvs::Real32> average_scalars;
    std::vector<kvs::Real32> average_normals; 
    std::vector<int>         average_cellids;

    // 分散値データ
    std::vector<kvs::Real32> varience_scalars;
    std::vector<kvs::Real32> varience_normals; 

    // 引数ようにstructにまとめる
	Ensembleparameters ens_param{
        vertex_coords,
            vertex_scalars,
            vertex_normals,
            vertex_cellids,
            average_scalars,
            average_normals,
            varience_scalars,
            varience_normals,
            ncells,
            MPIprocess_per_ensemble,
            ens_number,
            repetitions
    };

    uniform_sampling(cell, 
                     ens_param,
                     particleBase); 

    ensemble_reduce(cell, 
            ens_param,
            particleBase);

    if(mpi_rank == 0) rejection_process(cell, ens_param, particleBase);
}


void EnsembleGenerateParticles( int time_step,
                         domain_parameters dom,
                         Type** values, int nvariables,
                         float* coordinates, int ncoords,
                         unsigned int* connections, int ncells, const pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase) //celltype  enum 型に変更
{
    
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );

    if(mpi_rank==0)std::cout<<"start generate_particles\n";
    static bool start_flag = true;
    static bool parameter_file_opened=false;

    // 初回サンプリング処理
    // 動的な粒子データ配列
    std::vector<kvs::Real32> vertex_coords;
    std::vector<kvs::Real32> vertex_scalars;
    std::vector<kvs::Real32> vertex_normals;
    std::vector<int> vertex_cellids;

    //平均値データ
    std::vector<kvs::Real32> average_coords;
    std::vector<kvs::UInt8>  average_colors;
    std::vector<kvs::Real32> average_scalars;
    std::vector<kvs::Real32> average_normals; 
    std::vector<int>         average_cellids;

//  pbvr::cellbaseの宣言 
    std::vector< pbvr::CellBase<Type>* > cell;
    cell.resize( max_threads );

    switch ( celltype )
    {
        case pbvr::VolumeObjectBase::Tetrahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    cell[i]  = new pbvr::TetrahedralCell<Type>( values[0], coordinates, ncoords, connections, ncells );
                }
                break;
            }
        case pbvr::VolumeObjectBase::Hexahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: Hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                     cell[i]  = new pbvr::HexahedralCell<Type>( values[0], coordinates, ncoords, connections, ncells );
                }
                break;
            }

        default:
            {
                //BaseClass::m_is_success = false;
                //kvsMessageError( "Unsupported cell type." );
                std::cout << "Unsupported cell type." << std::endl; 
                return;
            }
       }

//    int   tf_number                = particleBase.m_tf_number;
    float sampling_volume_inverse  = particleBase.m_sampling_volume_inverse ;
    float max_opacity              = particleBase.m_max_opacity             ;
    float max_density              = particleBase.m_max_density             ;
    int   subpixel_level           = particleBase.m_subpixel_level          ;
    float particle_density         = particleBase.m_particle_density        ;
    float particle_data_size_limit = particleBase.m_particle_data_size_limit;
    // repeat level を手動設定
//    int   repetitions             = 24 ; // 4 scalar
//    int   repetitions             = 18 ;
    int   repetitions             = 8 ;
    parameter_file_opened = particleBase.m_parameter_file_opened;
//    const int max_nparticles = (int)max_density + 1;

    // アンサンブル数でrepetitionを徐算
    const int MPIprocess_per_ensemble = 1;  
    //const int MPIprocess_per_ensemble = 4;  
    int ens_number = mpi_size/MPIprocess_per_ensemble;
    repetitions /= ens_number;
    if (repetitions < ens_number ) repetitions = 1;

//    float max_opacity              = 0.98;
    // Hydrogen
    const float min_value = 0;
    const float max_value = 255;
    const float var_min_value = -8;
    const float var_max_value =-8;
////    // spx 
//    const float min_value = 0.2;
//    const float max_value = 1;
    
    
    const int tf_resolution = 256;

    // tfクラスの作成（デフォルトカラー）
    // stab data
    
    std::vector<kvs::UInt8> c_table ={5,48,97,6,50,100,7,52,102,8,54,105,9,56,108,10,58,111,11,60,114,12,62,116,14,64,119,15,66,122,16,68,125,17,70,128,18,72,131,19,74,134,20,76,136,21,78,139,22,80,142,23,83,145,24,85,148,25,87,151,27,89,154,28,91,157,29,93,160,30,95,163,31,98,166,32,100,169,33,102,172,35,104,173,37,105,174,38,107,175,40,109,176,41,111,177,43,113,178,45,114,178,46,116,179,47,118,180,49,120,181,50,121,182,51,123,183,53,125,184,54,127,185,55,129,186,57,130,187,58,132,188,59,134,189,60,136,189,61,138,190,63,140,191,64,141,192,65,143,193,66,145,194,67,147,195,71,149,196,74,151,197,78,153,198,81,154,199,85,156,200,88,158,201,91,160,202,95,162,203,98,164,204,101,166,205,104,168,206,107,170,207,110,172,209,113,174,210,116,175,211,118,177,212,121,179,213,124,181,214,127,183,215,130,185,216,132,187,217,135,189,218,138,191,219,141,193,220,143,195,221,146,197,222,149,198,223,151,200,223,154,201,224,157,202,225,159,203,226,162,205,226,164,206,227,167,207,228,169,208,228,172,210,229,174,211,230,177,212,231,179,214,231,182,215,232,184,216,233,187,217,234,189,219,234,192,220,235,194,221,236,197,223,236,199,224,237,202,225,238,204,226,239,207,228,239,209,229,240,210,230,240,212,230,241,213,231,241,215,232,241,216,232,241,218,233,242,219,234,242,221,235,242,222,235,242,224,236,243,225,237,243,227,237,243,228,238,244,230,239,244,231,239,244,233,240,244,234,241,245,235,241,245,237,242,245,238,243,245,240,244,246,241,244,246,243,245,246,244,246,246,246,246,247,247,247,247,247,246,245,248,245,243,248,244,241,248,243,240,249,242,238,249,241,236,249,239,234,250,238,232,250,237,230,250,236,228,250,235,227,251,234,225,251,233,223,251,232,221,251,231,219,251,230,217,252,229,215,252,228,214,252,227,212,252,225,210,252,224,208,252,223,206,253,222,204,253,221,203,253,220,201,253,219,199,253,217,196,253,215,193,252,212,191,252,210,188,252,208,185,252,206,182,252,204,179,251,202,177,251,200,174,251,197,171,250,195,168,250,193,165,250,191,163,249,189,160,249,187,157,248,184,154,248,182,152,248,180,149,247,178,146,247,176,143,246,174,141,246,171,138,245,169,135,245,167,133,244,165,130,243,162,128,242,160,126,241,157,124,240,155,122,239,152,119,238,149,117,237,147,115,235,144,113,234,142,111,233,139,109,232,136,107,231,134,105,230,131,103,229,128,101,227,126,99,226,123,97,225,120,95,224,118,93,223,115,91,221,112,89,220,110,87,219,107,85,218,104,83,217,102,81,215,99,79,214,96,77,213,94,76,211,91,74,210,89,73,208,86,71,207,84,70,206,82,68,204,79,67,203,77,66,201,74,64,200,72,63,198,69,62,197,66,60,196,64,59,194,61,57,193,58,56,191,55,55,190,53,53,188,50,52,187,46,51,185,43,49,184,40,48,182,36,47,181,33,46,179,29,44,178,24,43,175,23,43,172,22,42,169,21,42,166,20,41,162,19,41,159,18,40,156,17,40,153,15,39,150,14,39,147,13,38,144,12,38,141,11,37,138,10,37,135,9,36,132,8,36,129,7,35,126,6,35,123,5,34,120,4,34,117,3,33,115,2,33,112,2,33,109,1,32,106,1,32,103,0,31};

    kvs::ValueArray<kvs::UInt8> cc_table(c_table);
        
    kvs::ColorMap color_map( cc_table, min_value, max_value  );
    kvs::OpacityMap opacity_map( tf_resolution, min_value, max_value );
    //auto tf = kvs::TransferFunction( color_map );
    auto tf = pbvr::TransferFunction( color_map );
 
//  tf のハードコーディング 
//	kvs::ColorMap color_map;
//    auto tf = pbvr::TransferFunction( tf_resolution );
//    tf.setColorMap(color_map) ;
//    tf.setRange(min_value, max_value);

//  .tfファイルを参照
//    auto tf = particleBase.m_tf[0];
//    const float max_value_tf = particleBase.m_tf[0].opacityMap().maxValue();
//    const float min_value_tf = particleBase.m_tf[0].opacityMap().minValue();

//    all_reduce_ensemble(tf, cell, particleBase, ncells);
   
    reduce_scatter_ensemble(tf, cell, particleBase, ncells);

//#define OLD_ENSEMBLE

#ifdef OLD_ENSEMBLE
        int nparticles[ ncells ];
    kvs::Timer timer( kvs::Timer::Start );
#pragma omp parallel
{    
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif
        //配列の追加
        kvs::Vector3f local_center_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_center_array[ SIMD_BLK_SIZE ];
        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];

        float cell_opacity_array[ SIMD_BLK_SIZE ];
        int nparticles_array[ SIMD_BLK_SIZE ];
        int th_total_particles =0;

        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
        float density_array[ SIMD_BLK_SIZE ];
        float volume_array[ SIMD_BLK_SIZE ];


    kvs::MersenneTwister MT( thid + mpi_rank * nthreads );
    std::vector<kvs::Real32> th_vertex_coords;
    std::vector<kvs::Real32> th_vertex_scalars;
    std::vector<kvs::Real32> th_vertex_normals;
    std::vector<int>         th_vertex_cellids;
    std::vector<kvs::Real32> th_vertex_normals_x;
    std::vector<kvs::Real32> th_vertex_normals_y;
    std::vector<kvs::Real32> th_vertex_normals_z;

    float time1=0, time2 =0, time3 =0, time4 = 0, time5 = 0;
    float timeN[20]= {0};
//    kvs::Timer th_timer( kvs::Timer::Start );
#if 1
#pragma omp for schedule( dynamic ) nowait
    for ( size_t index = 0; index < ncells; index += SIMD_BLK_SIZE )
    {
//           th_timer.start();
           //ブロック内でのループ回数を取得
            int remain = ( ncells - index > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: ncells - index;

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
//            #pragma omp simd
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(index + cell_BLK);
            }

//            th_timer.stop();
//            timeN[0] += th_timer.sec();
//            th_timer.start();

            cell[thid]->bindCellArray( remain, cell_index );

//            th_timer.stop();
//            timeN[1] += th_timer.sec();
//            th_timer.start();
            
            cell[thid]->volumeArray( remain, volume_array);
//            th_timer.stop();
//            timeN[2] += th_timer.sec();
//            th_timer.start();

            //生成粒子数を計算
            #pragma simd
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    nparticles_array[cell_BLK] 
                        = calculate_number_of_particles( max_density, volume_array[cell_BLK], &MT ) * repetitions * particle_density ;
            }
//            th_timer.stop();
//            timeN[3] += th_timer.sec();
            // アンサンブル粒子データを作成
            int p_id = 0;
            float p_x_l[remain], p_y_l[remain], p_z_l[remain];
            float p_x_g[remain], p_y_g[remain], p_z_g[remain];
            for(int cell_BLK = 0; cell_BLK < remain+1; cell_BLK++ )
            {
                const int nparticles_I  = cell_BLK<remain ? nparticles_array[cell_BLK] : 1;
                // ------------------------------------------------
                
                for( int i = 0; i < nparticles_I; i+=SIMD_BLK_SIZE )
                {
//                    th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( nparticles_I - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_I - i;
//                    const int zero_id = cell_BLK<SIMDW ? SIMDW : p_id;

//                    th_timer.stop();
//                    timeN[4] += th_timer.sec();
                    if( cell_BLK < remain )
                    {
//                   if(thid == 0)std::cout << mpi_rank << ", "  << __LINE__ << ", remain = " << remain  << ", remain_BLK ="  << remain_BLK << ", p_id = " << p_id  <<std::endl;
#pragma simd
                   for( int j = 0; j < remain_BLK; j++ ) 
                   {
//                       th_timer.start();
                       cell_index[p_id] = index + cell_BLK;
//                       th_timer.stop();
//                       timeN[4] += th_timer.sec();
//                       th_timer.start();

                       local_coord_array[p_id] = cell[thid] -> randomSampling_MT( &MT );
                       // デバッグ用の座標固定
//                        local_coord_array[p_id] = kvs::Vector3f (0,0,0);
//                       th_timer.stop();
//                       timeN[5] += th_timer.sec();
                       p_id ++;
                       if(p_id == SIMD_BLK_SIZE )
                       {
//                           th_timer.start();

                           //補間器にセルを一括でバインド
                           cell[thid]->bindCellArray( p_id, cell_index );
//                           th_timer.stop();
//                           timeN[6] += th_timer.sec();
//                           th_timer.start();
                           cell[thid]->setLocalPointArray(  p_id, local_coord_array );

//                           th_timer.stop();
//                           timeN[7] += th_timer.sec();
//                           th_timer.start();
                           float scalar_array[p_id];
                           float grad_array_x[p_id];
                           float grad_array_y[p_id];
                           float grad_array_z[p_id];

                           //                    cell[thid]->CalcScalarGrad( p_id,
                           //                            scalar_array,
                           //                            grad_array_x,
                           //                            grad_array_y,
                           //                            grad_array_z );

                           cell[thid] -> scalar_ary( scalar_array, p_id);
//                           th_timer.stop();
//                           timeN[8] += th_timer.sec();
//                           th_timer.start();
                           cell[thid] -> grad_ary( grad_array_x, grad_array_y, grad_array_z, p_id);

//                           th_timer.stop();
//                           timeN[9] += th_timer.sec();
//                           th_timer.start();
                           // resize による最適化テスト
                           size_t base_c = th_vertex_coords.size();
                           size_t base_n = th_vertex_normals.size();
                           size_t base_s = th_vertex_scalars.size();
                           size_t base_id = th_vertex_cellids.size();

                           th_vertex_coords.resize(base_c + p_id * 3);
                           th_vertex_normals.resize(base_n + p_id * 3);
                           th_vertex_scalars.resize(base_s + p_id);
                           th_vertex_cellids.resize(base_id + p_id);

#pragma simd
                           for( int j = 0; j < p_id; j++ ) 
                           {
                               // resize による最適化テスト
                               size_t c = base_c + 3*j;
                               th_vertex_coords[c+0] = local_coord_array[j].x();
                               th_vertex_coords[c+1] = local_coord_array[j].y();
                               th_vertex_coords[c+2] = local_coord_array[j].z();

                               th_vertex_scalars[base_s + j] = scalar_array[j];

                               size_t n = base_n + 3*j;
                               th_vertex_normals[n+0] = grad_array_x[j];
                               th_vertex_normals[n+1] = grad_array_y[j];
                               th_vertex_normals[n+2] = grad_array_z[j];

                               th_vertex_cellids[base_id + j] = cell_index[j];

                               //                        th_vertex_coords.push_back( local_coord_array[j].x() );
                               //                        th_vertex_coords.push_back( local_coord_array[j].y() );
                               //                        th_vertex_coords.push_back( local_coord_array[j].z() );
                               //
                               //                        th_vertex_scalars.push_back( scalar_array[j] );
                               //
                               //                        th_vertex_normals.push_back( grad_array_x[j] );
                               //                        th_vertex_normals.push_back( grad_array_y[j] );
                               //                        th_vertex_normals.push_back( grad_array_z[j] );
                               ////                        th_vertex_normals_x.push_back( grad_array_x[j] );
                               ////                        th_vertex_normals_y.push_back( grad_array_y[j] );
                               ////                        th_vertex_normals_z.push_back( grad_array_z[j] );
                               //
                               //                        th_vertex_cellids.push_back( cell_index[j] );
                           }
//                           th_timer.stop();
//                           timeN[10] += th_timer.sec();
                           p_id = 0;
                       } 
                   }
                    }// end if cell_BLK < remain 
                    if(cell_BLK == remain)  // p_id ==SIMD_BLK と一緒にしたかったが、無理っぽい
                    {
//                        th_timer.start();

                    //補間器にセルを一括でバインド
                    cell[thid]->bindCellArray( p_id, cell_index );
//                    th_timer.stop();
//                    timeN[6] += th_timer.sec();
//                    th_timer.start();
                    cell[thid]->setLocalPointArray(  p_id, local_coord_array );
                    
//                    th_timer.stop();
//                    timeN[7] += th_timer.sec();
//                    th_timer.start();
                    float scalar_array[p_id];
                    float grad_array_x[p_id];
                    float grad_array_y[p_id];
                    float grad_array_z[p_id];

//                    cell[thid]->CalcScalarGrad( p_id,
//                            scalar_array,
//                            grad_array_x,
//                            grad_array_y,
//                            grad_array_z );
                        
                    cell[thid] -> scalar_ary( scalar_array, p_id);
//                    th_timer.stop();
//                    timeN[8] += th_timer.sec();
//                    th_timer.start();
                    cell[thid] -> grad_ary( grad_array_x, grad_array_y, grad_array_z, p_id);

//                    th_timer.stop();
//                    timeN[9] += th_timer.sec();
//                    th_timer.start();
// resize による最適化テスト
                    size_t base_c = th_vertex_coords.size();
                    size_t base_n = th_vertex_normals.size();
                    size_t base_s = th_vertex_scalars.size();
                    size_t base_id = th_vertex_cellids.size();

                    th_vertex_coords.resize(base_c + p_id * 3);
                    th_vertex_normals.resize(base_n + p_id * 3);
                    th_vertex_scalars.resize(base_s + p_id);
                    th_vertex_cellids.resize(base_id + p_id);

#pragma simd
                    for( int j = 0; j < p_id; j++ ) 
                    {
// resize による最適化テスト
                        size_t c = base_c + 3*j;
                        th_vertex_coords[c+0] = local_coord_array[j].x();
                        th_vertex_coords[c+1] = local_coord_array[j].y();
                        th_vertex_coords[c+2] = local_coord_array[j].z();

                        th_vertex_scalars[base_s + j] = scalar_array[j];

                        size_t n = base_n + 3*j;
                        th_vertex_normals[n+0] = grad_array_x[j];
                        th_vertex_normals[n+1] = grad_array_y[j];
                        th_vertex_normals[n+2] = grad_array_z[j];

                        th_vertex_cellids[base_id + j] = cell_index[j];

                    }
//                        th_timer.stop();
//                        timeN[10] += th_timer.sec();
                    p_id = 0;
                    }


                } // end for Nparticle_I 
            }
    }
#else
#pragma omp for  
    for ( size_t index = 0; index < ncells; index ++ )
    {
                th_timer.start();
            cell[thid]->bindCell( index );
                th_timer.stop();
                timeN[0] += th_timer.sec();
                th_timer.start();
            // Calculate a number of particles in this cell.
            const float volume_of_cell = cell[thid]->volume();
                th_timer.stop();
                timeN[1] += th_timer.sec();
                th_timer.start();

            size_t nparticles_in_cell 
                = calculate_number_of_particles( max_density, volume_of_cell, &MT ) ;
            nparticles_in_cell *= repetitions;
                th_timer.stop();
                timeN[2] += th_timer.sec();

            // Generate a set of particles in this cell represented by v0,...,v3 and s0,...,s3.
            for ( size_t particle = 0; particle < nparticles_in_cell; ++particle )
            {
                // Calculate a coord. // ローカル座標
                th_timer.start();
                const kvs::Vector3f coord = cell[thid]->randomSampling_MT( &MT);
                th_timer.stop();
                timeN[3] += th_timer.sec();
                th_timer.start();
                cell[thid]->setLocalPoint(coord); 
                th_timer.stop();
                timeN[4] += th_timer.sec();
                // Calculate a color.
                th_timer.start();
                const float scalar = cell[thid]->scalar();
                th_timer.stop();
                timeN[5] += th_timer.sec();

                // Calculate a normal.
                /* NOTE: The gradient vector of the cell is reversed for shading on the rendering process.
                */
                th_timer.start();
                const kvs::Vector3f normal( -cell[thid]->gradient() );
                th_timer.stop();
                timeN[6] += th_timer.sec();

//                th_timer.start();
                // set coord, color, and normal to point object( this ).
                th_vertex_coords.push_back( coord.x() );
                th_vertex_coords.push_back( coord.y() );
                th_vertex_coords.push_back( coord.z() );

                th_vertex_scalars.push_back( scalar );

                th_vertex_normals.push_back( normal.x() );
                th_vertex_normals.push_back( normal.y() );
                th_vertex_normals.push_back( normal.z() );

                th_vertex_cellids.push_back( index );

                th_timer.stop();
                timeN[7] += th_timer.sec();
            } // end of 'paricle' for-loop

    } // end of 'cell' for-loop
#endif
    #pragma omp barrier
//    th_timer.start();
    #pragma omp critical
    {
        vertex_coords.insert (vertex_coords.end() , th_vertex_coords.begin() , th_vertex_coords.end());
        vertex_scalars.insert(vertex_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end());
        vertex_normals.insert(vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end());
        vertex_cellids.insert(vertex_cellids.end(), th_vertex_cellids.begin(), th_vertex_cellids.end());
//        vertex_normals_x.insert(vertex_normals_x.end(), th_vertex_normals_x.begin(), th_vertex_normals_x.end());
//        vertex_normals_y.insert(vertex_normals_y.end(), th_vertex_normals_y.begin(), th_vertex_normals_y.end());
//        vertex_normals_z.insert(vertex_normals_z.end(), th_vertex_normals_z.begin(), th_vertex_normals_z.end());
    }
//    th_timer.stop();
//    timeN[11] += th_timer.sec();
//    th_timer.start();
//        for (int i =0;i < 12; i++)
//        {
//            std::cout << mpi_rank <<  ": ave_sampling_time["<< i <<"] =" << timeN[i] << std::endl;
//        }
}  //end omp loop
    timer.stop();
    std::cout << mpi_rank <<  ": uniform_sampling_time =" << timer.sec() << std::endl;
    std::cout << mpi_rank <<  ": uniform_nparticles : " <<  vertex_scalars.size()  << " particle_density = " << particle_density << std::endl;
//     if(mpi_rank == 0)std::cout << " particle_density = " <<  particle_density << std::endl;

    // シフト処理
    //if (mpi_size > 1 )
    if (ens_number > 1 )
    {
//            timer.start();
        float timeN[20]= {0};
       // 送信データの個数 
       const int local_size = vertex_scalars.size(); 
  
       // 送信バッファ
       std::vector<float> send_buff_float;
       std::vector<int> send_buff_int;

        // 受信バッファ
        // 変数値
        std::vector<float> recv_scalars; 
        // 座標
        std::vector<float> recv_coords;
        //セルid
        std::vector<int> recv_cellids;
        // 法線
        std::vector<float> recv_normals;
        //平均値
        std::vector<float> recv_scalars_average;
        std::vector<float> recv_normals_average;
        
        std::vector<float> recv_buff_float;
        std::vector<int> recv_buff_int;


        // 送信先（自分の次のrank）、受信元（自分の前のrank）
//        int send_to = (mpi_rank + 1 ) % mpi_size;
//        int recv_from = (mpi_rank - 1 + mpi_size ) % mpi_size;
        int send_to   = (mpi_rank + MPIprocess_per_ensemble ) % mpi_size;
        int recv_from = (mpi_rank - MPIprocess_per_ensemble + mpi_size ) % mpi_size;
        float shift_exe_time = 0;
        float move_exe_time = 0;
        float time1=0, time2 =0, time3 =0, time4 = 0, time5 = 0;

        // 各ラウンドでリングを回していく（size-1回繰り返す） 
        for (int step = 0; step < ens_number - 1; step++) 
        {
            timer.start();
#if 1
            const int send_size = vertex_scalars.size();
            int recv_size = 0;
            MPI_Request reqs[2];

            // 非同期送受信 粒子数 
            MPI_Isend(&send_size, 1, MPI_INT,
                    send_to, 0, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(&recv_size, 1, MPI_INT,
                    recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

            // 通信完了待ち
            MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

#if 0
            // 4 size check
            auto die = [&](const char* msg){
                fprintf(stderr, "[rank %d] %s\n", mpi_rank, msg);
                MPI_Abort(MPI_COMM_WORLD, 999);
            };

            if ((int)vertex_cellids.size() != send_size) die("vertex_cellids.size() != send_size");
            if ((int)vertex_coords.size()  != 3*send_size) die("vertex_coords.size() != 3*send_size");
            if ((int)vertex_normals.size() != 3*send_size) die("vertex_normals.size() != 3*send_size");            
#endif          
            // 受け取った粒子数でメモリ確保
             recv_scalars.resize(recv_size);
             recv_coords.resize(3*recv_size);
             recv_cellids.resize(recv_size);
             recv_normals.resize(3*recv_size);
//             int tmp_recv_cellids[recv_size];

             // 送受信中にメモリ破壊が起きないよう送信用配列を宣言
             std::vector<int>   send_cellids = vertex_cellids; // defensive copy
             std::vector<float> send_scalars = vertex_scalars; // defensive copy
             std::vector<float> send_normals = vertex_normals; // defensive copy
             std::vector<float> send_coords  = vertex_coords; // defensive copy

            // 非同期送受信 cell_id
//            MPI_Isend(vertex_cellids.data(), send_size, MPI_INT,
            MPI_Isend(send_cellids.data(), send_size, MPI_INT,
                    send_to, 12, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(recv_cellids.data(), recv_size, MPI_INT,
                    recv_from, 12, MPI_COMM_WORLD, &reqs[1]);
            // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);


//            MPI_Isend(vertex_scalars.data(), send_size, MPI_FLOAT,
            MPI_Isend(send_scalars.data(), send_size, MPI_FLOAT,
                    send_to, 10, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(recv_scalars.data(), recv_size, MPI_FLOAT,
                    recv_from, 10, MPI_COMM_WORLD, &reqs[1]);

            // 通信完了待ち
            MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

            // 非同期送受信 座標
//            MPI_Isend(vertex_coords.data(), 3*send_size, MPI_FLOAT,
            MPI_Isend(send_coords.data(), 3*send_size, MPI_FLOAT,
                    send_to, 11, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(recv_coords.data(), 3*recv_size, MPI_FLOAT,
                    recv_from, 11, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
            MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
            // 非同期送受信 normal
//            MPI_Isend(vertex_normals.data(), 3*send_size, MPI_FLOAT,
            MPI_Isend(send_normals.data(), 3*send_size, MPI_FLOAT,
                    send_to, 13, MPI_COMM_WORLD, &reqs[0]);
            MPI_Irecv(recv_normals.data(), 3*recv_size, MPI_FLOAT,
                    recv_from, 13, MPI_COMM_WORLD, &reqs[1]);

            // 通信完了待ち
            MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

#else
#endif
            timer.stop();
            shift_exe_time += timer.sec();
            std::cout << mpi_rank <<  ": shift_time_step["<< step <<"] =" << timer.sec() << std::endl;

            // 受け取った座標情報で、recv先の条件下でのスカラー値を計算
            
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
                    
        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];

        float th_timeN[20]= {0};
//        kvs::Timer th_timer( kvs::Timer::Start );

           #pragma omp for  
            for(int i =0; i< recv_size ;i+= SIMD_BLK_SIZE)
            {
//                th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( recv_size - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: recv_size - i;
            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = recv_cellids[i + j];
                    }

//                th_timer.stop();
//                th_timeN[0] += th_timer.sec();
//                th_timer.start();

//            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        local_coord_array[j].x() = recv_coords[ 3*(i+j)+ 0];
                        local_coord_array[j].y() = recv_coords[ 3*(i+j)+ 1];
                        local_coord_array[j].z() = recv_coords[ 3*(i+j)+ 2];
                    }
//                th_timer.stop();
//                th_timeN[1] += th_timer.sec();
//                th_timer.start();
                    // Calculate a color.
                    cell[thid]->bindCellArray( remain_BLK, cell_index );
//                th_timer.stop();
//                th_timeN[2] += th_timer.sec();
//                th_timer.start();

                    cell[thid]->setLocalPointArray(  remain_BLK, local_coord_array );

//                th_timer.stop();
//                th_timeN[3] += th_timer.sec();
//                th_timer.start();
                    float scalar_array[remain_BLK];
                    float grad_array_x[remain_BLK];
                    float grad_array_y[remain_BLK];
                    float grad_array_z[remain_BLK];

//                    cell[thid]->CalcScalarGrad( remain_BLK,
//                            scalar_array,
//                            grad_array_x,
//                            grad_array_y,
//                            grad_array_z );
                    cell[thid] -> scalar_ary( scalar_array, remain_BLK);
                    cell[thid] -> grad_ary( grad_array_x, grad_array_y, grad_array_z, remain_BLK);
                        
//                th_timer.stop();
//                th_timeN[4] += th_timer.sec();
//                th_timer.start();
                    float recv_normals_array_x[remain_BLK];
                    float recv_normals_array_y[remain_BLK];
                    float recv_normals_array_z[remain_BLK];

//                th_timer.stop();
//                th_timeN[5] += th_timer.sec();
//                th_timer.start();
#if 1
            #pragma omp simd
                    //配列をAOSからSOAに
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        recv_normals_array_x[j] = recv_normals[3*(i+j)];
                        recv_normals_array_y[j] = recv_normals[3*(i+j)+1];
                        recv_normals_array_z[j] = recv_normals[3*(i+j)+2];
                    }


            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        // 算出データを受信データと足し合わせる
                        recv_scalars[i+j]     = recv_scalars[i+j] + scalar_array[j];
                        recv_normals_array_x[j] = recv_normals_array_x[j] + grad_array_x[j];
                        recv_normals_array_y[j] = recv_normals_array_y[j] + grad_array_y[j];
                        recv_normals_array_z[j] = recv_normals_array_z[j] + grad_array_z[j];
                    }
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        recv_normals[3*(i+j)]   = recv_normals_array_x[j];
                        recv_normals[3*(i+j)+1] = recv_normals_array_y[j];
                        recv_normals[3*(i+j)+2] = recv_normals_array_z[j];
                    }
//                th_timer.stop();
//                th_timeN[6] += th_timer.sec();
//                th_timer.start();

#else
#endif
            }
//#pragma omp barrier
//#pragma omp critical
//            {
//                for(int i =0; i <  7; i++ )
//                    timeN[i] += th_timeN[i]/nthreads;    
//            }

}
            // 次のラウンドでは受信したデータを送信対象に更新
            vertex_scalars = recv_scalars;
            vertex_normals = recv_normals;
            vertex_cellids = recv_cellids;
            vertex_coords = recv_coords;
            timer.stop();
            move_exe_time += timer.sec();
        } // end for loop shift step
        std::cout << mpi_rank <<  ": shift_exe_time =" << shift_exe_time << std::endl;
        std::cout << mpi_rank <<  ": move_exe_time =" << move_exe_time << std::endl;
//        for (int i =0;i < 7; i++)
//        {
//            std::cout << mpi_rank <<  ": ave_calc_time["<< i <<"] =" << timeN[i] << std::endl;
//        }



       //　平均化処理 シフト処理の回数分徐算
       const float invert_num = 1.f/float(ens_number); 
#pragma omp simd
       for (int i = 0; i < vertex_scalars.size(); i++ )
       {
           vertex_scalars[ i ]     *= invert_num; 
           vertex_normals[3*i + 0] *= -invert_num;
           vertex_normals[3*i + 1] *= -invert_num;
           vertex_normals[3*i + 2] *= -invert_num;
       }

    }
    //棄却法を適応する
#if 1

//// 常用対数を取るための計算
 std::vector<float> log_vertex_scalars(vertex_scalars.size());
     float delta = 1e-30;
       int N = 0; // OutputFuncで正規化したオーダー
#pragma omp simd
       for (int i = 0; i < vertex_scalars.size(); i++ )
       {
//           log_vertex_scalars[ i ]     = std::log10(vertex_scalars[ i ]); 
//           log_vertex_scalars[ i ]     =  vertex_scalars[ i ] > delta ? std::log10(vertex_scalars[ i ]) : -30; 
           log_vertex_scalars[ i ]     =  vertex_scalars[ i ] > delta ? std::log10(vertex_scalars[ i ]) -N : -30; 
       }

    timer.stop();
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
//
    kvs::MersenneTwister MT( 10 + mpi_rank );
//    kvs::MersenneTwister MT( vertex_scalars.size() +  thid + mpi_rank * nthreads );
    std::vector<kvs::Real32> th_vertex_coords;
    std::vector<kvs::Real32> th_vertex_normals;
    std::vector<kvs::UInt8>  th_vertex_colors;
    std::vector<kvs::Real32>  th_vertex_scalars;
    std::vector<int>  th_vertex_cellids;

    kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
    kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
    kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
    float opacity_array[ SIMD_BLK_SIZE ];
    float density_array[ SIMD_BLK_SIZE ];
    float th_timeN[20]= {0};
//    kvs::Timer th_timer( kvs::Timer::Start );

#pragma omp for  
            for(int i =0; i< vertex_scalars.size() ;i+= SIMD_BLK_SIZE)
            {
//                th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: vertex_scalars.size() - i;
                    // セル登録
            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = vertex_cellids[i + j];
                    }
                    cell[thid] -> bindCellArray(remain_BLK, cell_index);

//                th_timer.stop();
//                th_timeN[0] += th_timer.sec();
//                th_timer.start();

                //局所座標の詰め替え
//            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        local_coord_array[j].x() = vertex_coords[ 3*(i+j)+ 0];
                        local_coord_array[j].y() = vertex_coords[ 3*(i+j)+ 1];
                        local_coord_array[j].z() = vertex_coords[ 3*(i+j)+ 2];
                    }

//                th_timer.stop();
//                th_timeN[1] += th_timer.sec();
//                th_timer.start();
                //座標の登録
                cell[thid] -> setLocalPointArray(remain_BLK,local_coord_array);

//                th_timer.stop();
//                th_timeN[2] += th_timer.sec();
//                th_timer.start();
                //全体座標への変換
                cell[thid] -> transformLocalToGlobalArray(remain_BLK,local_coord_array,global_coord_array);

//                th_timer.stop();
//                th_timeN[3] += th_timer.sec();
//                th_timer.start();

                //opacity 計算 
                for( int j = 0; j < remain_BLK; j++ ) 
                {
                    opacity_array[j] = tf.opacityMap().at(vertex_scalars[ i+j ]);
//                    対数値を参照
//                    opacity_array[j] = tf.opacityMap().at(log_vertex_scalars[ i+j ]);
                }
//                th_timer.stop();
//                th_timeN[4] += th_timer.sec();
//                th_timer.start();

#pragma omp simd
                for( int j = 0; j < remain_BLK; j++ ) 
                {
                    density_array[j] = opacity_array[j] < max_opacity ? -std::log( 1.0f - opacity_array[j] ) * sampling_volume_inverse: max_density;  
                }
//                    th_timer.stop();
//                    th_timeN[5] += th_timer.sec();

                for( int j = 0; j < remain_BLK; j++ ) 
                {
//                    th_timer.start();
//                    float density  = Generator::CalculateDensity( opacity_array[j],
//                            sampling_volume_inverse,
//                            max_opacity, max_density );
//
//                    th_timer.start();
               const float R = MT.rand();
//                th_timer.stop();
//                th_timeN[6] += th_timer.sec();
               if ( density_array[j] > max_density * R )
               {
//                th_timer.start();
                   // Calculate a color.
                   const kvs::RGBColor color( tf.colorMap().at( vertex_scalars[ i+j ] ) );
//                    対数値を参照
//                   const kvs::RGBColor color( tf.colorMap().at( log_vertex_scalars[ i+j ] ) );
//                th_timer.stop();
//                th_timeN[7] += th_timer.sec();
//                th_timer.start();

                   // Calculate a normal.
//                   const kvs::Vector3f normal( vertex_normals[3*(i+j)+0], vertex_normals[3*(i+j)+1], vertex_normals[3*(i+j)+2] );
//                th_timer.stop();
//                th_timeN[8] += th_timer.sec();
//                th_timer.start();

                   // set coord, color, and normal to point object( this ).
                   th_vertex_coords.push_back( global_coord_array[j].x() );
                   th_vertex_coords.push_back( global_coord_array[j].y() );
                   th_vertex_coords.push_back( global_coord_array[j].z() );

                   th_vertex_colors.push_back( color.r() );
                   th_vertex_colors.push_back( color.g() );
                   th_vertex_colors.push_back( color.b() );

                   th_vertex_normals.push_back( vertex_normals[3*(i+j)+0] );
                   th_vertex_normals.push_back( vertex_normals[3*(i+j)+1] );
                   th_vertex_normals.push_back( vertex_normals[3*(i+j)+2] );

//                th_timer.stop();
//                th_timeN[9] += th_timer.sec();
               }
                }
            }
//                th_timer.start();
#pragma omp critical
       {
           average_coords.insert (average_coords.end() , th_vertex_coords.begin() , th_vertex_coords.end());
           average_colors.insert (average_colors.end(), th_vertex_colors.begin() , th_vertex_colors.end());
           average_normals.insert(average_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end());
//           average_scalars.insert(average_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end());
//           average_cellids.insert(average_cellids.end(), th_vertex_cellids.begin(), th_vertex_cellids.end());
       }
//                th_timer.stop();
//                th_timeN[10] += th_timer.sec();
//#pragma omp critical
//                {    
//                    for (int i =0;i < 11; i++)
//                    {
//                        std::cout << mpi_rank <<  ": ave_rejection_time["<< i <<"] =" << th_timeN[i] << std::endl;
//                    }
//                }

}
       timer.stop();
       std::cout << mpi_rank << ": rejection_exe_time =" << timer.sec() << std::endl;
       int size = average_coords.size();
#endif

    std::cout << mpi_rank <<  ": nparticles : " <<  size/3   << std::endl;

//    // 平均値データを集約する
    particleBase.m_sample_coords.insert(particleBase.m_sample_coords.end()  , average_coords.begin() , average_coords.end());
    particleBase.m_sample_colors.insert(particleBase.m_sample_colors.end()  , average_colors.begin() , average_colors.end());
    particleBase.m_sample_normals.insert(particleBase.m_sample_normals.end(), average_normals.begin(), average_normals.end());

#if 1
    // 分散を計算する
    // cityLBM用の minmax設定
     if( particleBase.m_tf.size() >1 ) tf = particleBase.m_tf[1];
       // 分散計算処理 
       //平均値 データ配列の受け渡し
          average_scalars = vertex_scalars;
          average_normals = vertex_normals;
          average_cellids = vertex_cellids;
          average_coords  = vertex_coords ;

          //  幾何標準偏差を計算する
          // 　足切り閾値
          float eps = 1e-5;
      //初回サンプリング 
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

    kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
    kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
    float th_timeN[20]= {0};
//    kvs::Timer th_timer( kvs::Timer::Start );
#pragma omp for  
            for(int i =0; i< vertex_scalars.size() ;i+= SIMD_BLK_SIZE)
            {
//                th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: vertex_scalars.size() - i;
                    // セル登録
            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = vertex_cellids[i + j];
                    }
//                th_timer.stop();
//                th_timeN[1] += th_timer.sec();
//                th_timer.start();
                    cell[thid] -> bindCellArray(remain_BLK, cell_index);

//                th_timer.stop();
//                th_timeN[2] += th_timer.sec();
//                th_timer.start();

                //局所座標の詰め替え
//            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        local_coord_array[j].x() = vertex_coords[ 3*(i+j)+ 0];
                        local_coord_array[j].y() = vertex_coords[ 3*(i+j)+ 1];
                        local_coord_array[j].z() = vertex_coords[ 3*(i+j)+ 2];
                    }
//                th_timer.stop();
//                th_timeN[3] += th_timer.sec();
//                th_timer.start();
                    cell[thid] -> setLocalPointArray(remain_BLK, local_coord_array);
//                th_timer.stop();
//                th_timeN[4] += th_timer.sec();
//                th_timer.start();

                    float scalar_array[remain_BLK];
                    float grad_array_x[remain_BLK];
                    float grad_array_y[remain_BLK];
                    float grad_array_z[remain_BLK];

                    cell[thid]->CalcScalarGrad( remain_BLK,
                            scalar_array,
                            grad_array_x,
                            grad_array_y,
                            grad_array_z );

//                th_timer.stop();
//                th_timeN[5] += th_timer.sec();
//                th_timer.start();
//                //丸め誤差の影響排除のための正規化
//            #pragma omp simd
//                    for (int j= 0; j <remain_BLK ;j++)
//                    {
//                        scalar_array[j]           *= 1e2;
//                        average_scalars[i+j]      *= 1e2;
//                        average_normals[3*(i+j)+0]*= 1e2;
//                        average_normals[3*(i+j)+1]*= 1e2;
//                        average_normals[3*(i+j)+2]*= 1e2;
//                        grad_array_x[j]           *= 1e2;
//                        grad_array_y[j]           *= 1e2;
//                        grad_array_z[j]           *= 1e2;
//                    }

            #pragma omp simd
                    for (int j= 0; j <remain_BLK ;j++)
                    {
//                        vertex_scalars[i+j]       = (scalar_array[j] - average_scalars[i+j])*(scalar_array[j] - average_scalars[i+j]);
//                        vertex_scalars[i+j]       =  (scalar_array[j] - average_scalars[i+j])  > eps ? (scalar_array[j] - average_scalars[i+j])*(scalar_array[j] - average_scalars[i+j]): 0 ;
//                        vertex_scalars[i+j]       =  average_scalars[i+j]  > eps ? (scalar_array[j] - average_scalars[i+j])*(scalar_array[j] - average_scalars[i+j]): 0 ;
                        vertex_scalars[i+j]       =  scalar_array[j] > eps ? (scalar_array[j] - average_scalars[i+j])*(scalar_array[j] - average_scalars[i+j]): average_scalars[i+j] ;
//                        vertex_scalars[i+j]       = scalar_array[j] > eps ? (std::log(scalar_array[j]) - std::log(average_scalars[i+j]))*(std::log(scalar_array[j]) - std::log(average_scalars[i+j])) : std::log(average_scalars[i+j])*std::log(average_scalars[i+j]) ;
                        vertex_normals[3*(i+j)+0] = (scalar_array[j] - average_scalars[i+j])*(grad_array_x[j] - average_normals[3*(i+j)+0]);
                        vertex_normals[3*(i+j)+1] = (scalar_array[j] - average_scalars[i+j])*(grad_array_y[j] - average_normals[3*(i+j)+1]);
                        vertex_normals[3*(i+j)+2] = (scalar_array[j] - average_scalars[i+j])*(grad_array_z[j] - average_normals[3*(i+j)+2]);
                    }
//                th_timer.stop();
//                th_timeN[6] += th_timer.sec();
//                th_timer.start();
            }
#if 0    
#pragma omp for  
          for(int i =0; i< average_scalars.size();i++)
          {
                th_timer.start();
              cell[thid]->bindCell( vertex_cellids[i] );
                th_timer.stop();
                th_timeN[0] += th_timer.sec();
                th_timer.start();
              const kvs::Vector3f local_coord(vertex_coords[3*i + 0], vertex_coords[3*i + 1], vertex_coords[3*i + 2]);
                th_timer.stop();
                th_timeN[1] += th_timer.sec();
                th_timer.start();

              // Calculate a color.
              cell[thid] -> setLocalPoint(local_coord);
                th_timer.stop();
                th_timeN[2] += th_timer.sec();
                th_timer.start();
              const float scalar = cell[thid]->scalar();
                th_timer.stop();
                th_timeN[3] += th_timer.sec();
                th_timer.start();

              // Calculate a normal.
              const kvs::Vector3f normal( -cell[thid]->gradient() );
                th_timer.stop();
                th_timeN[4] += th_timer.sec();
                th_timer.start();

              // 算出データを受信データと足し合わせる
              vertex_scalars[i]     = (scalar - average_scalars[i])*(scalar - average_scalars[i]);
              vertex_normals[3*i+0] = (scalar - average_scalars[i])*(normal.x() - average_normals[3*i+0]);
              vertex_normals[3*i+1] = (scalar - average_scalars[i])*(normal.y() - average_normals[3*i+1]);
              vertex_normals[3*i+2] = (scalar - average_scalars[i])*(normal.z() - average_normals[3*i+2]);
                th_timer.stop();
                th_timeN[5] += th_timer.sec();
                th_timer.start();
          }
#endif
//    #pragma omp critical
//        for (int i =0;i < 7; i++)
//        {
//            std::cout << mpi_rank <<  ": var_sampling_time["<< i <<"] =" << th_timeN[i] << std::endl;
//        }
}
           vertex_cellids = average_cellids;
           vertex_coords  = average_coords;
    timer.stop();
    std::cout << mpi_rank <<  ": var_uniform_sampling_time =" << timer.sec() << std::endl;
    // シフト処理
    //if (mpi_size > 1 )
    if (ens_number > 1 )
    {
           // 送信データの個数 
       const int local_size = vertex_scalars.size(); 
        float timeN[20]= {0};
        // 受信バッファ
        // 変数値
        std::vector<float> recv_scalars; 
        // 座標
        std::vector<float> recv_coords;
        //セルid
        std::vector<int> recv_cellids;
        // 法線
        std::vector<float> recv_normals;
        //平均値
        std::vector<float> recv_scalars_average;
        std::vector<float> recv_normals_average;


           // 送信先（自分の次のrank）、受信元（自分の前のrank）
        int send_to   = (mpi_rank + MPIprocess_per_ensemble ) % mpi_size;
        int recv_from = (mpi_rank - MPIprocess_per_ensemble + mpi_size ) % mpi_size;
        float shift_exe_time = 0;
        float move_exe_time = 0;

       // 各ラウンドでリングを回していく（size-1回繰り返す）
       for (int step = 0; step < ens_number - 1; step++) 
       {
            timer.start();
           const int send_size = average_scalars.size();
           int recv_size = 0;
           MPI_Request reqs[2];

           // 非同期送受信 粒子数 
           MPI_Isend(&send_size, 1, MPI_INT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(&recv_size, 1, MPI_INT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
           // 受け取った粒子数でメモリ確保
           recv_scalars.resize(recv_size);
           recv_coords.resize(3*recv_size);
           recv_cellids.resize(recv_size);
           recv_normals.resize(3*recv_size);
           recv_scalars_average.resize(recv_size);
           recv_normals_average.resize(3*recv_size);

           // 送受信中にメモリ破壊が起きないよう送信用配列を宣言
           std::vector<int>   send_cellids = vertex_cellids; // defensive copy
           std::vector<float> send_scalars = vertex_scalars; // defensive copy
           std::vector<float> send_normals = vertex_normals; // defensive copy
           std::vector<float> send_coords  = vertex_coords; // defensive copy
           std::vector<float> send_normals_average = average_normals; // defensive copy
           std::vector<float> send_scalars_average  = average_scalars; // defensive copy


           // 非同期送受信  変数
           MPI_Isend(vertex_scalars.data(), send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_scalars.data(), recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 座標
           //MPI_Isend(vertex_coords.data(), 3*send_size, MPI_FLOAT,
           MPI_Isend(send_coords.data(), 3*send_size, MPI_FLOAT,
                   send_to, 10, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_coords.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 10, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 cell_id
           //MPI_Isend(vertex_cellids.data(), send_size, MPI_INT,
           MPI_Isend(send_cellids.data(), send_size, MPI_INT,
                   send_to, 11, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_cellids.data(), recv_size, MPI_INT,
                   recv_from, 11, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 normal
           //MPI_Isend(vertex_normals.data(), 3*send_size, MPI_FLOAT,
           MPI_Isend(send_normals.data(), 3*send_size, MPI_FLOAT,
                   send_to, 12, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_normals.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 12, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 average_scalar
           //MPI_Isend(average_scalars.data(), send_size, MPI_FLOAT,
           MPI_Isend(send_scalars_average.data(), send_size, MPI_FLOAT,
                   send_to, 13, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_scalars_average.data(), recv_size, MPI_FLOAT,
                   recv_from, 13, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 average_normal
           //MPI_Isend(average_normals.data(), 3*send_size, MPI_FLOAT,
           MPI_Isend(send_normals_average.data(), 3*send_size, MPI_FLOAT,
                   send_to, 14, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_normals_average.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 14, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

            timer.stop();
            shift_exe_time += timer.sec();
            std::cout << mpi_rank <<  ": var_shift_time_step["<< step <<"] =" << timer.sec() << std::endl;
           // 受け取った座標情報で、recv先の条件下でのスカラー値を計算
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

        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];

        float th_timeN[20]= {0};
//        kvs::Timer th_timer( kvs::Timer::Start );
           #pragma omp for  
            for(int i =0; i< recv_size ;i+= SIMD_BLK_SIZE)
            {
//                th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( recv_size - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: recv_size - i;
            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = recv_cellids[i + j];
                    }

//                th_timer.stop();
//                th_timeN[0] += th_timer.sec();
//                th_timer.start();

//            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        local_coord_array[j].x() = recv_coords[ 3*(i+j)+ 0];
                        local_coord_array[j].y() = recv_coords[ 3*(i+j)+ 1];
                        local_coord_array[j].z() = recv_coords[ 3*(i+j)+ 2];
                    }

//                th_timer.stop();
//                th_timeN[1] += th_timer.sec();
//                th_timer.start();

                    // Calculate a color.
                    cell[thid]->bindCellArray( remain_BLK, cell_index );
//                th_timer.stop();
//                th_timeN[2] += th_timer.sec();
//                th_timer.start();
                    cell[thid]->setLocalPointArray(  remain_BLK, local_coord_array );
//                th_timer.stop();
//                th_timeN[3] += th_timer.sec();
//                th_timer.start();

                    float scalar_array[remain_BLK];
                    float grad_array_x[remain_BLK];
                    float grad_array_y[remain_BLK];
                    float grad_array_z[remain_BLK];

                    cell[thid]->CalcScalarGrad( remain_BLK,
                            scalar_array,
                            grad_array_x,
                            grad_array_y,
                            grad_array_z );

//                th_timer.stop();
//                th_timeN[4] += th_timer.sec();
//                th_timer.start();
                    float recv_normals_array_x[remain_BLK];
                    float recv_normals_array_y[remain_BLK];
                    float recv_normals_array_z[remain_BLK];
                    float recv_normals_average_array_x[remain_BLK];
                    float recv_normals_average_array_y[remain_BLK];
                    float recv_normals_average_array_z[remain_BLK];


                    //配列をAOSからSOAに
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        recv_normals_array_x[j] = recv_normals[3*(i+j)];
                        recv_normals_array_y[j] = recv_normals[3*(i+j)+1];
                        recv_normals_array_z[j] = recv_normals[3*(i+j)+2];
                        recv_normals_average_array_x[j] = recv_normals_average[3*(i+j)];
                        recv_normals_average_array_y[j] = recv_normals_average[3*(i+j)+1];
                        recv_normals_average_array_z[j] = recv_normals_average[3*(i+j)+2];
                    }
//                    //丸め誤差の影響排除のための正規化
//            #pragma omp simd
//                    for (int j= 0; j <remain_BLK ;j++)
//                    {
//                        scalar_array[j]           *= 1e2;
//                        grad_array_x[j]           *= 1e2;
//                        grad_array_y[j]           *= 1e2;
//                        grad_array_z[j]           *= 1e2;
//                    }

            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
//                        // 算出データを受信データと足し合わせる
                        recv_scalars[i+j]       =  scalar_array[j] > eps ? (scalar_array[j] - recv_scalars_average[i+j] )*(scalar_array[j] - recv_scalars_average[  i+j    ]) + recv_scalars[i+j] : recv_scalars[i+j];
                        recv_normals_array_x[j] = (scalar_array[j] - recv_scalars_average[i+j] )*(grad_array_x[j] - recv_normals_average_array_x[j]) + recv_normals_array_x[j];
                        recv_normals_array_y[j] = (scalar_array[j] - recv_scalars_average[i+j] )*(grad_array_y[j] - recv_normals_average_array_y[j]) + recv_normals_array_y[j];
                        recv_normals_array_z[j] = (scalar_array[j] - recv_scalars_average[i+j] )*(grad_array_z[j] - recv_normals_average_array_z[j]) + recv_normals_array_z[j];
//           if((i+j) == 0  && mpi_rank < 4 )std::cerr << "recv_scalars[j] =  " << recv_scalars[i+j]  << ", scalar_array[j] = " <<  scalar_array[j] << " recv_scalars_average[i+j] = " << recv_scalars_average[i+j] <<std::endl; 
                    }
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        recv_normals[3*(i+j)]   = recv_normals_array_x[j];
                        recv_normals[3*(i+j)+1] = recv_normals_array_y[j];
                        recv_normals[3*(i+j)+2] = recv_normals_array_z[j];
                    }

//                th_timer.stop();
//                th_timeN[5] += th_timer.sec();
//                th_timer.start();
            }
//#pragma omp critical
//            {
//                for(int i =0; i <  7; i++ )
//                    timeN[i] += th_timeN[i]/nthreads;    
//            }


}
           // 次のラウンドでは受信したデータを送信対象に更新
           vertex_scalars = recv_scalars;
           vertex_normals = recv_normals;
           vertex_cellids = recv_cellids;
           vertex_coords = recv_coords;
           average_cellids = recv_cellids;
           average_coords = recv_coords;
           average_scalars = recv_scalars_average;
           average_normals = recv_normals_average;
           timer.stop();
           move_exe_time += timer.sec();
       }
            
       std::cout << mpi_rank <<  ": var_shift_exe_time =" << shift_exe_time << std::endl;
       std::cout << mpi_rank <<  ": var_move_exe_time =" << move_exe_time << std::endl;

//#if 0      
       // 集計された分散値を計算
       //const float invert_num = 1.f/float(mpi_size);  
       const float invert_num = 1.f/float(ens_number); 
       #pragma omp simd
       for (int i = 0; i < vertex_scalars.size();  i++ )
       {
           vertex_scalars[ i ]   *=   invert_num; 
           vertex_normals[3*i + 0] *=  -2.f*invert_num;// 2.f は分散計算式の係数
           vertex_normals[3*i + 1] *=  -2.f*invert_num;// 2.f は分散計算式の係数
           vertex_normals[3*i + 2] *=  -2.f*invert_num;// 2.f は分散計算式の係数
       }

//       timer.stop();
//       std::cout << mpi_rank <<  ": var_shift_exe_time =" << timer.sec() << std::endl;
    } // end if(mpi_size ==1)

//        float delta = 1e-30;
        //　変動け異数に変換 平均が0の場合は計数値はほぼ0とする。
         std::vector<float> co_varietion(vertex_scalars.size());

       #pragma omp simd
       for (int i = 0; i < vertex_scalars.size();  i++ )
       {
//           if(average_scalars[i] > 1e-5 && vertex_scalars[ i ] > 1e-10 )
//           if(average_scalars[i] > 1e-6 && vertex_scalars[ i ] > 1e-12 )
//           if(average_scalars[i] > eps && vertex_scalars[ i ] > 1e-12 )
           if(average_scalars[i] > eps )
           co_varietion[ i ]   =  std::sqrt(vertex_scalars[ i ] )/ average_scalars[i];
           else
               co_varietion[ i ]   = delta;
//           co_varietion[ i ]   =  average_scalars[i] > 1e-5 ? std::sqrt(vertex_scalars[ i ] )/ average_scalars[i] : delta;
//           if(i%10000 == 0)std::cerr << "vertex_scalars[ i ] = " << std::sqrt(vertex_scalars[ i ] ) << " average_scalars[i] = " << average_scalars[i] <<std::endl; 
       }
 
// 常用対数を取るための計算
// std::vector<float> log_vertex_scalars(vertex_scalars.size());
   log_vertex_scalars.resize(vertex_scalars.size());
#pragma omp simd
       for (int i = 0; i < vertex_scalars.size(); i++ )
       {
//           log_vertex_scalars[ i ]     =  std::log10(vertex_scalars[ i ]); 
//           log_vertex_scalars[ i ]     =  vertex_scalars[ i ] > delta ? std::log10(vertex_scalars[ i ]): -30; 
           log_vertex_scalars[ i ]     =  co_varietion[ i ] > delta ? std::log10(co_varietion[ i ]) : -30; 
       }

//// 幾何標準偏差を取るための計算
// std::vector<float> gsd_scalars(vertex_scalars.size());
////   log_vertex_scalars.resize(vertex_scalars.size());
//#pragma omp simd
//       for (int i = 0; i < vertex_scalars.size(); i++ )
//       {
//           gsd_scalars[ i ]     =   std::exp(vertex_scalars[ i ]) ; 
//       }

    //分散用配列を宣言 
    std::vector<kvs::Real32> var_coords;
    std::vector<kvs::UInt8>  var_colors;
    std::vector<kvs::Real32> var_scalars;  // デバッグ用
    std::vector<kvs::Real32> var_normals; 
//    std::vector<int>         var_cellids;

//    //棄却法を適応する
    timer.stop();
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
//
    kvs::MersenneTwister MT( 10 + mpi_rank );
//    kvs::MersenneTwister MT( vertex_scalars.size() +  thid + mpi_rank * nthreads );
    std::vector<kvs::Real32> th_vertex_coords;
    std::vector<kvs::Real32> th_vertex_normals;
    std::vector<kvs::UInt8>  th_vertex_colors;
    std::vector<kvs::Real32>  th_vertex_scalars;
    std::vector<int>  th_vertex_cellids;

    kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
    kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
    kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
    float opacity_array[ SIMD_BLK_SIZE ];
    float density_array[ SIMD_BLK_SIZE ];
    float th_timeN[20]= {0};
//    kvs::Timer th_timer( kvs::Timer::Start );
#pragma omp for  
            for(int i =0; i< vertex_scalars.size() ;i+= SIMD_BLK_SIZE)
            {
//                th_timer.start();
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: vertex_scalars.size() - i;
                    // セル登録
            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = vertex_cellids[i + j];
                    }
                    cell[thid] -> bindCellArray(remain_BLK, cell_index);

//                th_timer.stop();
//                th_timeN[0] += th_timer.sec();
//                th_timer.start();

                //局所座標の詰め替え
//            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        local_coord_array[j].x() = vertex_coords[ 3*(i+j)+ 0];
                        local_coord_array[j].y() = vertex_coords[ 3*(i+j)+ 1];
                        local_coord_array[j].z() = vertex_coords[ 3*(i+j)+ 2];
                    }

//                th_timer.stop();
//                th_timeN[1] += th_timer.sec();
//                th_timer.start();
                //座標の登録
                cell[thid] -> setLocalPointArray(remain_BLK,local_coord_array);

//                th_timer.stop();
//                th_timeN[2] += th_timer.sec();
//                th_timer.start();
                //全体座標への変換
                cell[thid] -> transformLocalToGlobalArray(remain_BLK,local_coord_array,global_coord_array);

//                th_timer.stop();
//                th_timeN[3] += th_timer.sec();
//                th_timer.start();

                //opacity 計算 
                for( int j = 0; j < remain_BLK; j++ ) 
                {
                    opacity_array[j] = tf.opacityMap().at(co_varietion[ i+j ]);
//                    opacity_array[j] = tf.opacityMap().at(log_vertex_scalars[ i+j ]);
//                    opacity_array[j] = tf.opacityMap().at(gsd_scalars[ i+j ]);
//                    opacity_array[j] = tf.opacityMap().at(vertex_scalars[ i+j ]);
                }
//                th_timer.stop();
//                th_timeN[4] += th_timer.sec();
//                th_timer.start();

#pragma omp simd
                for( int j = 0; j < remain_BLK; j++ ) 
                {
                    density_array[j] = opacity_array[j] < max_opacity ? -std::log( 1.0f - opacity_array[j] ) * sampling_volume_inverse: max_density;  
                }
//                    th_timer.stop();
//                    th_timeN[5] += th_timer.sec();
                for( int j = 0; j < remain_BLK; j++ ) 
                {
//                    th_timer.start();
               const float R = MT.rand();
//                th_timer.stop();
//                th_timeN[6] += th_timer.sec();
               if ( density_array[j] > max_density * R )
               {
//                th_timer.start();
                   // Calculate a color.
                   const kvs::RGBColor color( tf.colorMap().at( co_varietion[ i+j ] ) );
//                   const kvs::RGBColor color( tf.colorMap().at( gsd_scalars[ i+j ] ) );
                   //const kvs::RGBColor color( tf.colorMap().at( log_vertex_scalars[ i+j ] ) );
//                   const kvs::RGBColor color( tf.colorMap().at( vertex_scalars[ i+j ] ) );
//                th_timer.stop();
//                th_timeN[7] += th_timer.sec();
//                th_timer.start();

                   // Calculate a normal.
//                th_timer.stop();
//                th_timeN[8] += th_timer.sec();
//                th_timer.start();

                   // set coord, color, and normal to point object( this ).
                   th_vertex_coords.push_back( global_coord_array[j].x() );
                   th_vertex_coords.push_back( global_coord_array[j].y() );
                   th_vertex_coords.push_back( global_coord_array[j].z() );

                   th_vertex_colors.push_back( color.r() );
                   th_vertex_colors.push_back( color.g() );
                   th_vertex_colors.push_back( color.b() );

                   th_vertex_normals.push_back( vertex_normals[3*(i+j)+0] );
                   th_vertex_normals.push_back( vertex_normals[3*(i+j)+1] );
                   th_vertex_normals.push_back( vertex_normals[3*(i+j)+2] );

//                th_timer.stop();
//                th_timeN[9] += th_timer.sec();
               }
                }
            }

//       th_timer.start();
#pragma omp critical
       {
           var_coords.insert (var_coords.end() , th_vertex_coords.begin() , th_vertex_coords.end());
           var_colors.insert (var_colors.end(), th_vertex_colors.begin() , th_vertex_colors.end());
           var_normals.insert(var_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end());
           var_scalars.insert(var_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end());
//           average_cellids.insert(average_cellids.end(), th_vertex_cellids.begin(), th_vertex_cellids.end());
       }
//       th_timer.stop();
//       th_timeN[10] += th_timer.sec();
//#pragma omp critical
//                {    
//                    for (int i =0;i < 11; i++)
//                    {
//                        std::cout << mpi_rank <<  ": var_rejection_time["<< i <<"] =" << th_timeN[i] << std::endl;
//                    }
//                }


}  //end omp

       timer.stop();
       std::cout << mpi_rank << ": var_rejection_exe_time =" << timer.sec() << std::endl;
//    // 分散データを集約する
//    std::cout << mpi_rank <<  ": nparticles : " <<  var_coords.size()/3   << std::endl;
    particleBase.m_varience_coords.insert( particleBase.m_varience_coords.end()  , var_coords.begin() , var_coords.end());
    particleBase.m_varience_colors.insert( particleBase.m_varience_colors.end()  , var_colors.begin() , var_colors.end());
    particleBase.m_varience_normals.insert(particleBase.m_varience_normals.end(), var_normals.begin(), var_normals.end());
#endif
# if 0
    // 歪度を計算する (法線計算は除外)

    //分散値データ配列の受け渡し
    var_scalars = vertex_scalars;
    var_normals = vertex_normals;
    //var_cellids = vertex_cellids;
    var_coords  = vertex_coords ;

      //初回サンプリング 
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

#pragma omp for  
          for(int i =0; i< average_scalars.size();i++)
          {
              cell[thid]->bindCell( vertex_cellids[i] );
              const kvs::Vector3f local_coord(vertex_coords[3*i + 0], vertex_coords[3*i + 1], vertex_coords[3*i + 2]);

              // Calculate a color.
              cell[thid] -> setLocalPoint(local_coord);
              const float scalar = cell[thid]->scalar();

              // Calculate a normal.
             const kvs::Vector3f normal( -cell[thid]->gradient() );
              //                    const kvs::Vector3f normal( interpolator.gradient<float>() );

              //float tmp =  kvs::Math::Abs(scalar - average_scalars[i]) > 1e-6 ? (scalar - average_scalars[i])/std::sqrt(var_scalars[i]) : 0; 
              float tmp =  (scalar - average_scalars[i])/std::sqrt(var_scalars[i]); 
              float tmp2 = tmp* tmp;
              float invert_var = 1/var_scalars[i];
              float invert_scalar = kvs::Math::Abs(scalar - average_scalars[i] ) > 1e-6 ? 1/(scalar - average_scalars[i]) : 1e-6;
              // 算出データを受信データと足し合わせる
              vertex_scalars[i]     = tmp2*tmp;
              vertex_normals[3*i+0] = tmp2 *(0.5 * var_normals[3*i+0]*invert_var - (normal.x() - average_normals[3*i+0])*invert_scalar);
              vertex_normals[3*i+1] = tmp2 *(0.5 * var_normals[3*i+1]*invert_var - (normal.y() - average_normals[3*i+1])*invert_scalar);
              vertex_normals[3*i+2] = tmp2 *(0.5 * var_normals[3*i+2]*invert_var - (normal.z() - average_normals[3*i+2])*invert_scalar);
//              std::cout << "var_normals =" << var_normals[i] << ", var_scalars[i] =" << var_scalars[i]  << std::endl; //debug 
//              std::cout << "sk_scalars =" << vertex_scalars[i] << ", scalars[i] =" << scalar  << ", average_scalars =" << average_scalars[i]   << std::endl; //debug 
          }
}
    timer.stop();
    std::cout << mpi_rank <<  ": ske_uniform_sampling_time =" << timer.sec() << std::endl;
    // シフト処理
    //if (mpi_size > 1 )
    if (ens_number > 1 )
    {
//            timer.start();
           // 送信データの個数 
       const int local_size = vertex_scalars.size(); 

        // 受信バッファ
        // 変数値
        std::vector<float> recv_scalars; 
        // 座標
        std::vector<float> recv_coords;
        //セルid
        std::vector<int> recv_cellids;
        // 法線
        std::vector<float> recv_normals;
        //平均値
        std::vector<float> recv_scalars_average;
        std::vector<float> recv_normals_average;

        //平均値
        std::vector<float> recv_scalars_var;
        std::vector<float> recv_normals_var;

           // 送信先（自分の次のrank）、受信元（自分の前のrank）
        int send_to   = (mpi_rank + MPIprocess_per_ensemble ) % mpi_size;
        int recv_from = (mpi_rank - MPIprocess_per_ensemble + mpi_size ) % mpi_size;
        float shift_exe_time = 0;
        float move_exe_time = 0;

       // 各ラウンドでリングを回していく（size-1回繰り返す）
       for (int step = 0; step < ens_number - 1; step++) 
       {
           timer.start();
           const int send_size = average_scalars.size();
           int recv_size = 0;
           MPI_Request reqs[2];

           // 非同期送受信 粒子数 
           MPI_Isend(&send_size, 1, MPI_INT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(&recv_size, 1, MPI_INT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
           // 受け取った粒子数でメモリ確保
           recv_scalars.resize(recv_size);
           recv_coords.resize(3*recv_size);
           recv_cellids.resize(recv_size);
           recv_normals.resize(3*recv_size);
           recv_scalars_average.resize(recv_size);
           recv_normals_average.resize(3*recv_size);
           recv_scalars_var.resize(recv_size);
           recv_normals_var.resize(3*recv_size);

           // 非同期送受信  変数
           MPI_Isend(vertex_scalars.data(), send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_scalars.data(), recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 座標
           MPI_Isend(vertex_coords.data(), 3*send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_coords.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 cell_id
           MPI_Isend(vertex_cellids.data(), send_size, MPI_INT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_cellids.data(), recv_size, MPI_INT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
//           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 normal
           MPI_Isend(vertex_normals.data(), 3*send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_normals.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
//           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 average_scalar
           MPI_Isend(average_scalars.data(), send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_scalars_average.data(), recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
//           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 average_normal
           MPI_Isend(average_normals.data(), 3*send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_normals_average.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 var_scalar
           MPI_Isend(var_scalars.data(), send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_scalars_var.data(), recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

           // 非同期送受信 var_normal
           MPI_Isend(var_normals.data(), 3*send_size, MPI_FLOAT,
                   send_to, 0, MPI_COMM_WORLD, &reqs[0]);
           MPI_Irecv(recv_normals_var.data(), 3*recv_size, MPI_FLOAT,
                   recv_from, 0, MPI_COMM_WORLD, &reqs[1]);

           // 通信完了待ち
           MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);

            timer.stop();
            shift_exe_time += timer.sec();
           // 受け取った座標情報で、recv先の条件下でのスカラー値を計算
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
        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];
        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
           #pragma omp for  
            for(int i =0; i< recv_size ;i+= SIMD_BLK_SIZE)
            {
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( recv_size - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: recv_size - i;
            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = recv_cellids[i + j];
                    }


//            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
//                        local_coord_array[j][0] = recv_coords[ 3*(i+j)+ 0];
//                        local_coord_array[j][1] = recv_coords[ 3*(i+j)+ 1];
//                        local_coord_array[j][2] = recv_coords[ 3*(i+j)+ 2];
                          local_coord_array[j].set(recv_coords[ 3*(i+j)+ 0],recv_coords[ 3*(i+j)+ 1],recv_coords[ 3*(i+j)+ 2]);
                    }
//                const kvs::Vector3f coord(recv_coords[3*i + 0], recv_coords[3*i + 1], recv_coords[3*i + 2]);

                    // Calculate a color.
                    //cell[thid]->bindCell( recv_cellids[i] );
                    cell[thid]->bindCellArray( remain_BLK, cell_index );
                    cell[thid]->setLocalPointArray(  remain_BLK, local_coord_array );
//                cell[thid] -> setLocalPoint(coord);
//                const float scalar = cell[thid]->scalar();
//                    // Calculate a normal.
//                const kvs::Vector3f normal( -cell[thid]->gradient() );

                    float scalar_array[remain_BLK];
                    float grad_array_x[remain_BLK];
                    float grad_array_y[remain_BLK];
                    float grad_array_z[remain_BLK];
                    cell[thid]->CalcScalarGrad( remain_BLK,
                            scalar_array,
                            grad_array_x,
                            grad_array_y,
                            grad_array_z );

                    float recv_normals_array_x[remain_BLK];
                    float recv_normals_array_y[remain_BLK];
                    float recv_normals_array_z[remain_BLK];
                    float recv_normals_average_array_x[remain_BLK];
                    float recv_normals_average_array_y[remain_BLK];
                    float recv_normals_average_array_z[remain_BLK];
                    float recv_normals_var_array_x[remain_BLK];
                    float recv_normals_var_array_y[remain_BLK];
                    float recv_normals_var_array_z[remain_BLK];


                    //配列をAOSからSOAに
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        recv_normals_array_x[j] = recv_normals[3*(i+j)];
                        recv_normals_array_y[j] = recv_normals[3*(i+j)+1];
                        recv_normals_array_z[j] = recv_normals[3*(i+j)+2];
                        recv_normals_average_array_x[j] = recv_normals_average[3*(i+j)];
                        recv_normals_average_array_y[j] = recv_normals_average[3*(i+j)+1];
                        recv_normals_average_array_z[j] = recv_normals_average[3*(i+j)+2];
                        recv_normals_var_array_x[j] = recv_normals_var[3*(i+j)];
                        recv_normals_var_array_y[j] = recv_normals_var[3*(i+j)+1];
                        recv_normals_var_array_z[j] = recv_normals_var[3*(i+j)+2];
                    }


            #pragma omp simd
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        float tmp = (scalar_array[j] - recv_scalars_average[i+j])/std::sqrt(recv_scalars_var[i+j]); 
                        float tmp2 = tmp* tmp;
                        float invert_var = 1/recv_scalars_average[i+j];
                        float invert_scalar = kvs::Math::Abs(scalar_array[j] - recv_scalars_average[i+j] ) > 1e-6 ? 1/(scalar_array[j] - recv_scalars_average[i+j]) : 1e-6;
//                        // 算出データを受信データと足し合わせる
                        recv_scalars[i+j]       = tmp2*tmp + recv_scalars[i+j] ;
                        recv_normals_array_x[j] = tmp2*(0.5 * recv_normals_var_array_x[j]*invert_var - (grad_array_x[j] - recv_normals_average_array_x[j])*invert_scalar) + recv_normals_array_x[j] ;
                        recv_normals_array_y[j] = tmp2*(0.5 * recv_normals_var_array_y[j]*invert_var - (grad_array_y[j] - recv_normals_average_array_y[j])*invert_scalar) + recv_normals_array_y[j] ;
                        recv_normals_array_z[j] = tmp2*(0.5 * recv_normals_var_array_z[j]*invert_var - (grad_array_z[j] - recv_normals_average_array_z[j])*invert_scalar) + recv_normals_array_z[j] ;
//                        recv_normals[3*(i+j)  ] = tmp2*(0.5 * recv_normals_var[3*(i+j)  ]*invert_var - (grad_array_x[j] - recv_normals_average[3*(i+j)  ])*invert_scalar) + recv_normals[3*(i+j)  ] ;
//                        recv_normals[3*(i+j)+1] = tmp2*(0.5 * recv_normals_var[3*(i+j)+1]*invert_var - (grad_array_y[j] - recv_normals_average[3*(i+j)+1])*invert_scalar) + recv_normals[3*(i+j)+1] ;
//                        recv_normals[3*(i+j)+2] = tmp2*(0.5 * recv_normals_var[3*(i+j)+2]*invert_var - (grad_array_z[j] - recv_normals_average[3*(i+j)+2])*invert_scalar) + recv_normals[3*(i+j)+2] ;
                    }

                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        recv_normals[3*(i+j)]   = recv_normals_array_x[j];
                        recv_normals[3*(i+j)+1] = recv_normals_array_y[j];
                        recv_normals[3*(i+j)+2] = recv_normals_array_z[j];
                    }

            }


//           #pragma omp for 
//           for(int i =0; i< recv_size ;i++)
//           {
//               cell[thid]->bindCell( recv_cellids[i] );
//               const kvs::Vector3f coord(recv_coords[3*i + 0], recv_coords[3*i + 1], recv_coords[3*i + 2]);
//
//               // Calculate a color.
//               cell[thid] -> setLocalPoint(coord);
//               const float scalar = cell[thid]->scalar();
//
//               // Calculate a normal.
//               const kvs::Vector3f normal( -cell[thid]->gradient() );
//               // 算出データを受信データと足し合わせる
//              float tmp = (scalar - recv_scalars_average[i])/std::sqrt(recv_scalars_var[i]); 
//              float tmp2 = tmp* tmp;
//              float invert_var = 1/recv_scalars_average[i];
//              float invert_scalar = kvs::Math::Abs(scalar - recv_scalars_average[i] ) > 1e-6 ? 1/(scalar - recv_scalars_average[i]) : 1e-6;
//              // 算出データを受信データと足し合わせる
//
//               recv_scalars[i]     = tmp2*tmp + recv_scalars[i];
//               recv_normals[3*i+0] = tmp2 *(0.5 * recv_normals_var[3*i+0]*invert_var - (normal.x() - recv_normals_average[3*i+0])*invert_scalar)+recv_normals[3*i+0];
//               recv_normals[3*i+1] = tmp2 *(0.5 * recv_normals_var[3*i+1]*invert_var - (normal.y() - recv_normals_average[3*i+1])*invert_scalar)+recv_normals[3*i+1];
//               recv_normals[3*i+2] = tmp2 *(0.5 * recv_normals_var[3*i+2]*invert_var - (normal.z() - recv_normals_average[3*i+2])*invert_scalar)+recv_normals[3*i+2];
//           }
}
           // 次のラウンドでは受信したデータを送信対象に更新
           vertex_scalars = recv_scalars;
           vertex_normals = recv_normals;
           vertex_cellids = recv_cellids;
           vertex_coords = recv_coords;
           average_cellids = recv_cellids;
           average_coords = recv_coords;
           average_scalars = recv_scalars_average;
           average_normals = recv_normals_average;
           var_scalars     = recv_scalars_var;
           var_normals     = recv_normals_var;
            timer.stop();
            move_exe_time += timer.sec();
       }
       std::cout << mpi_rank <<  ": ske_shift_exe_time =" << shift_exe_time << std::endl;
       std::cout << mpi_rank <<  ": ske_move_exe_time =" << move_exe_time << std::endl;

//#if 0      
       // 集計された分散値を計算
       //const float invert_num = 1.f/(float(mpi_size));  
       const float invert_num = 1.f/float(ens_number); 
            #pragma omp simd
       for (int i = 0; i < vertex_scalars.size();  i++ )
       {
           vertex_scalars[ i ]   *=   invert_num; 
           vertex_normals[3*i + 0] *= - 3.f*invert_num;// 3.f は分散計算式の係数
           vertex_normals[3*i + 1] *= - 3.f*invert_num;// 3.f は分散計算式の係数
           vertex_normals[3*i + 2] *= - 3.f*invert_num;// 3.f は分散計算式の係数
       }
    } // end if(mpi_size ==1)


    //歪度用配列を宣言 
    std::vector<kvs::Real32> skewness_coords;
    std::vector<kvs::UInt8>  skewness_colors;
    std::vector<kvs::Real32> skewness_scalars;  // デバッグ用
    std::vector<kvs::Real32> skewness_normals; 
//    std::vector<int>         var_cellids;

//    //棄却法を適応する
    timer.stop();
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
//
    kvs::MersenneTwister MT( 10 + mpi_rank );
//    kvs::MersenneTwister MT( vertex_scalars.size() +  thid + mpi_rank * nthreads );
    std::vector<kvs::Real32> th_vertex_coords;
    std::vector<kvs::Real32> th_vertex_normals;
    std::vector<kvs::UInt8>  th_vertex_colors;
    std::vector<kvs::Real32>  th_vertex_scalars;
    std::vector<int>  th_vertex_cellids;

//#pragma omp for schedule( dynamic ) nowait
#pragma omp for
       for(int i =0; i< vertex_scalars.size() ;i++)
       {
            cell[thid] -> bindCell(vertex_cellids[i]); 
               const kvs::Vector3f coord(vertex_coords[3*i+0], vertex_coords[3*i+1], vertex_coords[3*i+2]);
               cell[thid] -> setLocalPoint(coord);
               // local 座標からglobal座標へ変換
               const kvs::Vector3f global_coord = cell[thid] -> transformLocalToGlobal(coord);
               // density の計算
               float opacity = tf.opacityMap().at(vertex_scalars[ i ]);
               float density  = Generator::CalculateDensity( opacity,
                       sampling_volume_inverse,
                       max_opacity, max_density );
               const float R = MT.rand();

               if ( density > max_density * R )
               {
                   // Calculate a color.
                   const kvs::RGBColor color( tf.colorMap().at( vertex_scalars[ i ] ) );
                   // Calculate a normal.
                   const kvs::Vector3f normal( vertex_normals[3*i+0], vertex_normals[3*i+1], vertex_normals[3*i+2] );

                   // set coord, color, and normal to point object( this ).
                   th_vertex_coords.push_back( global_coord.x() );
                   th_vertex_coords.push_back( global_coord.y() );
                   th_vertex_coords.push_back( global_coord.z() );

                   th_vertex_scalars.push_back(vertex_scalars[ i ]);

                   th_vertex_colors.push_back( color.r() );
                   th_vertex_colors.push_back( color.g() );
                   th_vertex_colors.push_back( color.b() );

                   th_vertex_normals.push_back( normal.x() );
                   th_vertex_normals.push_back( normal.y() );
                   th_vertex_normals.push_back( normal.z() );

               }
       }
#pragma omp critical
       {
           skewness_coords.insert (skewness_coords.end() , th_vertex_coords.begin() , th_vertex_coords.end());
           skewness_colors.insert (skewness_colors.end(), th_vertex_colors.begin() , th_vertex_colors.end());
           skewness_normals.insert(skewness_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end());
           skewness_scalars.insert(skewness_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end());
       }

}  //end omp

       timer.stop();
       std::cout << mpi_rank << ": ske_rejection_exe_time =" << timer.sec() << std::endl;
//    // 歪度データを集約する
//    std::cout << mpi_rank <<  ": nparticles : " <<  skewness_coords.size()/3   << std::endl;
    particleBase.m_skewness_coords.insert( particleBase.m_skewness_coords.end()  , skewness_coords.begin() , skewness_coords.end());
    particleBase.m_skewness_colors.insert( particleBase.m_skewness_colors.end()  , skewness_colors.begin() , skewness_colors.end());
    particleBase.m_skewness_normals.insert(particleBase.m_skewness_normals.end() , skewness_normals.begin(), skewness_normals.end());
#endif
#endif

    std::cout << mpi_rank << ": end sampling "  << std::endl;
    int tf_number = particleBase.m_tf.size();
    int nbins =256;

    kvs::ValueArray<int> o_histogram( tf_number * nbins );//不透明度ヒストグラムの配列
    kvs::ValueArray<int> c_histogram( tf_number * nbins );//色ヒストグラムの配列

    o_histogram_recv.allocate(tf_number * nbins);
    c_histogram_recv.allocate(tf_number * nbins);

    O_min_recv.allocate(tf_number);
    O_max_recv.allocate(tf_number);
    C_min_recv.allocate(tf_number);
    C_max_recv.allocate(tf_number);
    o_histogram.fill(0x00);
    c_histogram.fill(0x00);

    for( int i = 0; i < tf_number; i++ )
    {
        //不透明度
        particleBase.m_O_min[i] = particleBase.m_tf[i].minValue();
        particleBase.m_O_max[i] = particleBase.m_tf[i].maxValue();
        //色
        particleBase.m_C_min[i] = particleBase.m_tf[i].minValue();
        particleBase.m_C_max[i] = particleBase.m_tf[i].maxValue();

//        //不透明度
//        particleBase.m_O_min[i] = min_value_tf;
//        particleBase.m_O_max[i] = max_value_tf;
//        //色
//        particleBase.m_C_min[i] = min_value_tf;
//        particleBase.m_C_max[i] = max_value_tf;
//        std::cout << mpi_rank <<" : particleBase.m_C_min["<< i << "] = " << particleBase.m_C_min[i] << std::endl;
//        std::cout << mpi_rank <<" : particleBase.m_C_max["<< i << "] = " << particleBase.m_C_max[i] << std::endl;
    }

    for ( int i = 0; i < max_threads; i++ )
    {
             if (cell[i] != NULL)delete cell[i];
    }
    std::cerr << mpi_rank << ", "  << __LINE__  << std::endl;
}

void EnsembleTest( int time_step,
                         domain_parameters dom,
                         Type** values, int nvariables,
                         float* coordinates, int ncoords,
                         unsigned int* connections, int ncells, const pbvr::VolumeObjectBase::CellType& celltype, pbvr_parameters& particleBase) //celltype  enum 型に変更
{
}


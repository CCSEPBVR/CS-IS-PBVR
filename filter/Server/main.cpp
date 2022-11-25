/*****************************************************************************/
/**
 *  main.cpp
 *  CPU Server Program without CUDA, without KVS, without OpenGL.
 */
/*****************************************************************************/
#ifndef WIN32
#include <unistd.h>
#endif

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include "PointObject.h"
#include <kvs/CommandLine>
#include <kvs/Camera>
#include "TransferFunction.h"
#include <kvs/Matrix33>
#include <kvs/RotationMatrix33>

#include "common.h"

#include "PointObjectGenerator.h"

#include "Argument.h"

#include <kvs/AVSField>
#include <kvs/Timer>
#include "KVSMLObjectPointWriter.h"
//#include "KVSMLObjectPointMPIWriter.h"
#include "JobDispatcher.h"
#include "PbvrJobDispatcher.h"
#ifndef CPU_VER
#include "JobCollector.h"
#endif
#include "FilterInformation.h"
#include "ParameterFile.h"
#ifndef CPU_VER
#include "mpi.h"
#endif
#ifdef KMATH
#include <kmath_random.h>
#endif
#include <iostream>

#include <cassert>
#include <signal.h> /* 140319 for client stop by Ctrl+c */
#include <sys/stat.h>
#if (defined(KVS_PLATFORM_LINUX) || defined(KVS_PLATFORM_MACOSX))
#include <execinfo.h>
#endif
#include <kvs/File>

#include "ExtendedTransferFunction.h"
#include "TransferFunctionSynthesizerCreator.h"
#include "VariableRange.h"
#include "RangeEstimater.h"

#include "timer_simple.h"

#include <kvs/Compiler>
#ifdef KVS_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

#include "UnstructuredVolumeImporter.h"
#include "CellByCellParticleGenerator.h"

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;

// add by @hira at 2016/12/01
#include "PbvrFilterInformation.h"
#include "PbvrPointObjectGenerator.h"
#include "Server.h"

bool useAllNodes = true;
#ifdef KMATH
KMATH_Random km_random;
#endif

inline const size_t GetRevisedSubpixelLevel(
    const size_t m_subpixel_level,
    const size_t repetition_level )
{
    return static_cast<size_t>( m_subpixel_level * std::sqrt( ( double )repetition_level ) + 0.5f );
}

class PointObjectCreator
{
private:

    pbvr::UnstructuredVolumeObject* m_volume;

    pbvr::PointObjectGenerator *m_generator;

    int m_mpi_rank;

    const FilterInformationFile* m_fi;

    std::string m_xcSynthStr;
    std::string m_ycSynthStr;
    std::string m_zcSynthStr;

public:

    PointObjectCreator()
        : m_volume(NULL), m_generator(NULL), m_mpi_rank(0), m_fi(NULL) {}

    ~PointObjectCreator()
    {
        if (m_generator != NULL) delete m_generator;
    }

    void setFilterInfo( const FilterInformationFile* fi )
    {
        m_fi = fi;
        if (m_generator == NULL) {
            m_generator = factoryPointObjectGenerator(m_fi);
        }
    }

    void setGT5D()
    {
#if 0 //TEST_DELETE
        this->read_GT5D_connections_and_values();
        m_volume->setCoords( m_field->coords() );
        m_volume->updateMinMaxCoords();
#endif
    }

    void setGT5D_full( const int rank )
    {
#if 0 //TEST_DELETE
        m_mpi_rank = rank;
        this->read_GT5D_connections_and_values();

        const kvs::Vector3ui resol = m_field->dim();
        const kvs::UInt32 nnodes = resol.x() * resol.y() * resol.z();
        const kvs::Vector3ui elem = resol - kvs::Vector3ui( 1 );
        const kvs::UInt32 nelem = elem.x() * elem.y() * elem.z();

        kvs::ValueArray<kvs::Real32> coords( nnodes * 3 );
        kvs::Matrix33f mat( kvs::YRotationMatrix33<float>( ( float )rank * 60.0 ) );
        size_t i3 = 0;
        for ( size_t i = 0; i < nnodes; i++, i3 += 3 )
        {
            kvs::Vector3f vec;
            vec.x() = m_field->coords()[i3];
            vec.y() = m_field->coords()[i3 + 1];
            vec.z() = m_field->coords()[i3 + 2];

            vec = mat * vec;
            coords[i3] = vec.x();
            coords[i3 + 1] = vec.y();
            coords[i3 + 2] = vec.z();

        }

        m_volume->setCoords( coords );
        m_volume->updateMinMaxCoords();
        std::cout << rank << ":ucd vol\n" << *m_volume << std::endl;
#endif
    }

    void progressValues()
    {
#if 0 //TEST_DELETE
        m_field->progress();
        m_volume->setValues( kvs::AnyValueArray( m_field->values( 0 ) ) );
        m_volume->updateMinMaxValues();

#endif
    }

public:

    pbvr::PointObject* run( const Argument& param, const kvs::Camera& camera, const int timeStep, const int st = 1 )
    {
        // add by @hira at 2016/12/01
        m_generator->releaseObject();

        m_generator->setFinlterInfo( m_fi );
        m_generator->setCoordSynthTS( st );

        struct stat s;
        if ( stat( param.m_input_data.c_str(), &s ) )
        {
            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
            exit( 1 );
        }
        m_generator->createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step );

        pbvr::PointObject* po = m_generator->getPointObject();
        return po;
    }

    pbvr::PointObject* run( const Argument& param, const kvs::Camera& camera, const int timeStep, const int st, const int vl )
    {
        // add by @hira at 2016/12/01
        m_generator->releaseObject();

        m_generator->setFinlterInfo( m_fi );
        m_generator->setCoordSynthTS( st );
        m_generator->createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step, st, vl );
        pbvr::PointObject* po = m_generator->getPointObject();
        return po;
    }

    void setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss )
    {
        m_xcSynthStr = xss;
        m_ycSynthStr = yss;
        m_zcSynthStr = zss;

        pbvr::CoordSynthesizerStrings css( 0, xss, yss, zss );
        m_generator->setCoordSynthStrs( css );
    }

    void releaseObject() {

        if (m_generator != NULL) {
            m_generator->releaseObject();
        }
        return;
    }

protected:

    bool apply_coordinate_synthesizer( pbvr::PointObject* po, const int timeStep )
    {
        if ( ! po ) return false;
        if ( m_xcSynthStr.empty() && m_ycSynthStr.empty() && m_zcSynthStr.empty() )
            return true;
        register size_t i, j, nv = po->nvertices();
        if ( nv < 1 ) return true;
        FuncParser::Variables synth_vars;
        FuncParser::Variable X;
        X.tag( "X" );
        synth_vars.push_back( X );
        FuncParser::Variable x;
        x.tag( "x" );
        synth_vars.push_back( x );
        FuncParser::Variable Y;
        Y.tag( "Y" );
        synth_vars.push_back( Y );
        FuncParser::Variable y;
        y.tag( "y" );
        synth_vars.push_back( y );
        FuncParser::Variable Z;
        Z.tag( "Z" );
        synth_vars.push_back( Z );
        FuncParser::Variable z;
        z.tag( "z" );
        synth_vars.push_back( z );
        FuncParser::Variable T;
        T.tag( "T" );
        synth_vars.push_back( T );
        FuncParser::Variable t;
        t.tag( "t" );
        synth_vars.push_back( t );

        T = t = ( float )timeStep;
        const kvs::ValueArray<kvs::Real32>& org_coords = po->coords();
        kvs::ValueArray<kvs::Real32> new_coords;
        new_coords.deepCopy( org_coords );

        if ( ! m_xcSynthStr.empty() )
        {
            FunctionParser synth_func_parse( m_xcSynthStr, m_xcSynthStr.size() + 1 );
            FuncParser::Function synth_func;
            FunctionParser::Error err = synth_func_parse.express( synth_func, synth_vars );
            if ( err != FunctionParser::ERR_NONE )
            {
                std::stringstream ess;
                ess << "Function : " << synth_func.str() << " error: "
                    << FunctionParser::error_type_to_string[err].c_str() << std::endl;
                throw TransferFunctionSynthesizer::NumericException( ess.str() );
            }
            for ( i = 0; i < nv; i++ )
            {
                j = i * 3;
                X = x = org_coords[j];
                Y = y = org_coords[j + 1];
                Z = z = org_coords[j + 2];
                float d = ( float )synth_func.eval();
                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                new_coords[j] = d;
            }
        }
        if ( ! m_ycSynthStr.empty() )
        {
            FunctionParser synth_func_parse( m_ycSynthStr, m_ycSynthStr.size() + 1 );
            FuncParser::Function synth_func;
            FunctionParser::Error err = synth_func_parse.express( synth_func, synth_vars );
            if ( err != FunctionParser::ERR_NONE )
            {
                std::stringstream ess;
                ess << "Function : " << synth_func.str() << " error: "
                    << FunctionParser::error_type_to_string[err].c_str() << std::endl;
                throw TransferFunctionSynthesizer::NumericException( ess.str() );
            }
            for ( i = 0; i < nv; i++ )
            {
                j = i * 3;
                X = x = org_coords[j];
                Y = y = org_coords[j + 1];
                Z = z = org_coords[j + 2];
                float d = ( float )synth_func.eval();
                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                new_coords[j + 1] = d;
            }
        }
        if ( ! m_zcSynthStr.empty() )
        {
            FunctionParser synth_func_parse( m_zcSynthStr, m_zcSynthStr.size() + 1 );
            FuncParser::Function synth_func;
            FunctionParser::Error err = synth_func_parse.express( synth_func, synth_vars );
            if ( err != FunctionParser::ERR_NONE )
            {
                std::stringstream ess;
                ess << "Function : " << synth_func.str() << " error: "
                    << FunctionParser::error_type_to_string[err].c_str() << std::endl;
                throw TransferFunctionSynthesizer::NumericException( ess.str() );
            }
            for ( i = 0; i < nv; i++ )
            {
                j = i * 3;
                X = x = org_coords[j];
                Y = y = org_coords[j + 1];
                Z = z = org_coords[j + 2];
                float d = ( float )synth_func.eval();
                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                new_coords[j + 2] = d;
            }
        }
        po->setCoords( new_coords );
        return true;
    }

    void read_gt5d_connections_and_values()
    {
#if 0 //TEST_DELETE
        std::string header = "/home/kawamura/Data/GT5D_large/610x610x128x100-2/pd3d.fld";
        std::string coord = "/home/kawamura/Data/GT5D_large/610x610x128x100-2/co3d.dat";
        std::string variable = "/home/kawamura/Data/GT5D_large/610x610x128x100-2/pd3d.dat";

        m_field = new kvs::AVSField( header, coord, variable );
        m_field->progress();

        m_volume = new pbvr::UnstructuredVolumeObject;

        const kvs::Vector3ui resol = m_field->dim();
        const kvs::UInt32 nnodes = resol.x() * resol.y() * resol.z();
        const kvs::Vector3ui elem = resol - kvs::Vector3ui( 1 );
        const kvs::UInt32 nelem = elem.x() * elem.y() * elem.z();

        m_volume->setVeclen( 1 );
        m_volume->setNNodes( nnodes );
        m_volume->setNCells( nelem );
        m_volume->setCellType( pbvr::UnstructuredVolumeObject::Hexahedra );
        m_volume->setValues( kvs::AnyValueArray( m_field->values( 0 ) ) );

        const kvs::UInt32 line_size = resol.x();
        const kvs::UInt32 slice_size = line_size * resol.y();
        kvs::ValueArray<kvs::UInt32> connections( nelem * 8 );

        kvs::UInt32 m_index[8];
        kvs::UInt32* connec = connections.pointer();
        std::cout << m_mpi_rank << ":start of gen. conne." << std::endl;
        for ( size_t k = 0; k < elem.z(); k++ )
        {
            for ( size_t j = 0; j < elem.y(); j++ )
            {
                for ( size_t i = 0; i < elem.x(); i++ )
                {
                    m_index[0] = i + j * line_size + k * slice_size;
                    m_index[1] = m_index[0] + 1;
                    m_index[2] = m_index[1] + line_size;
                    m_index[3] = m_index[0] + line_size;
                    m_index[4] = m_index[0] + slice_size;
                    m_index[5] = m_index[1] + slice_size;
                    m_index[6] = m_index[2] + slice_size;
                    m_index[7] = m_index[3] + slice_size;

                    for ( size_t p = 0; p < 8; p++ )
                    {
                        *connec = m_index[p];
                        connec++;
                    }
                }
            }
        }
        m_volume->setConnections( connections );
        m_volume->updateMinMaxValues();
#endif
    }


    pbvr::PointObjectGenerator* factoryPointObjectGenerator(const FilterInformationFile* info)
    {
        pbvr::PointObjectGenerator* generator = NULL;
        if (info->isPbvrFilterInfo()) {
            generator = new pbvr::PbvrPointObjectGenerator();
        }
        else {
            generator = new pbvr::PointObjectGenerator();
        }

        return generator;
    }
};

/* 140319 for client stop by Ctrl+c */
bool SigServer = false;
void SignalHandler( const int sig )
{
    SigServer = true;
}
/* 140319 for client stop by Ctrl+c */

bool IsDirectory( const std::string directory_path )
{
#if defined ( WIN32 )
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA( directory_path.c_str(), &find_data );
    if ( hFind == INVALID_HANDLE_VALUE )
    {
        return false;
    }
    return ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
#else
    struct stat filestat;
    if ( stat( directory_path.c_str(), &filestat ) )
    {
        return false;
    }
    return filestat.st_mode & S_IFDIR;
#endif
}

inline pbvr::UnstructuredVolumeObject* CreateVolumeData(
                    const Argument& param,
                    const FilterInformationFile *fi,
                    const int& steps,
                    const int& subvols )
{
    pbvr::UnstructuredVolumeObject* volume = NULL;

    if (fi->isPbvrFilterInfo()) {
        const PbvrFilterInformationFile *pbvr_fi = dynamic_cast<const PbvrFilterInformationFile *>(fi);
        if (pbvr_fi != NULL && pbvr_fi->getPbvrFilter() != NULL) {
            volume = new pbvr::UnstructuredVolumeImporter(
                            pbvr_fi, steps, subvols );
        }
    }
    else if ( fi->m_file_type == 1 || fi->m_file_type == 2 )
    {
        kvs::File ifpx( fi->m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
        volume = new pbvr::UnstructuredVolumeImporter( path_base,
                              fi->m_file_type, steps, subvols );
    }
    else
    {
        std::stringstream suffix;
        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( steps )
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( subvols + 1 )
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << fi->m_number_subvolumes;

        //std::string input_data = param.input_data_base + suffix.str() + ".kvsml";
        kvs::File ifpx( fi->m_file_path );
        std::string m_input_data = ifpx.pathName() + ifpx.Separator()
                                 + ifpx.baseName() + suffix.str() + ".kvsml";
        volume = new pbvr::UnstructuredVolumeImporter( m_input_data );
    }

    if (volume != NULL) {
        volume->setMinMaxValues( fi->m_min_value, fi->m_max_value );
        volume->setMinMaxObjectCoords( fi->m_min_object_coord, fi->m_max_object_coord );
        volume->setMinMaxExternalCoords( fi->m_min_object_coord, fi->m_max_object_coord );
    }
    return volume;
}


inline float CalculateSamplingStep( const FilterInformationList *fil )
{
    float max_coord_length = kvs::Math::Max<float>( fil->m_total_max_object_coord.x() - fil->m_total_min_object_coord.x(),
                                                    fil->m_total_max_object_coord.y() - fil->m_total_min_object_coord.y(),
                                                    fil->m_total_max_object_coord.z() - fil->m_total_min_object_coord.z() );
    return 0.1 * max_coord_length;
}

//kawamura2: This calculates optimized subpixel level.
inline size_t CalculateSubpixelLevel( const Argument& param,
                                      const FilterInformationList *fil,
                                      const kvs::Camera& camera )
{
    namespace Generator = pbvr::CellByCellParticleGenerator;
    pbvr::UnstructuredVolumeObject* volume;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    int steps = fil->m_total_start_steps;
    int subvols = 0;

    //Total Volume Calculation
#ifndef CPU_VER
    int rank;
    int nprocs;
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
    int nprocs = 1;
#endif

    // add by @hira at 2016/12/01
    // ステップの物理量の読込みを行う。
    PBVR_TIMER_STA( 501 );
    fil->loadMpiSubvolume(steps);
    PBVR_TIMER_END( 501 );

    for ( subvols = 0; subvols < fil->m_total_number_subvolumes; subvols++ )
    {
        int xvl, fidx;
        fidx = fil->getFileIndex( subvols, &xvl );
        const FilterInformationFile *fi = fil->m_list[fidx];

        // modify by @hira at 2016/12/01
        //if ( subvols % nprocs == rank )
        if (fi->isPbvrSubvolume(subvols, rank, nprocs))
        {
            PBVR_TIMER_STA( 16 );
            volume = CreateVolumeData( param, fi, steps, xvl );
            PBVR_TIMER_END( 16 );

            PBVR_TIMER_STA( 17 );
            double local_volume = Generator::CalculateTotalVolume( volume );
            PBVR_TIMER_END( 17 );

            PBVR_TIMER_STA( 18 );
            density_lev1 += Generator::CalculateGreatDensity( camera, *volume, 1,
                                                              param.m_sampling_step ) * local_volume;
            PBVR_TIMER_END( 18 );

            total_volume += local_volume;
            delete volume;
        }
    }

    // add by @hira at 2016/12/01
    // ステップの物理量の読込後処理を行う。
    fil->releaseMpiSubvolume();

#ifndef CPU_VER
    PBVR_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, &density_lev1, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, &total_volume, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    PBVR_TIMER_END( 19 );
#endif

    density_lev1 /= total_volume;

    //kawamura2 : Calculation of optimized subpixel level
    float particle_limit = param.m_particle_limit;
    float nparticles_lev1 = total_volume * density_lev1;
    float subpixel_level = sqrt( particle_limit / nparticles_lev1 );

    if ( subpixel_level < 1 ) subpixel_level = 1;
    else if ( subpixel_level > 12 ) subpixel_level = 12;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

inline float CalculateDensityFactor( const Argument& param,
                                     const FilterInformationFile* fi,
                                     const kvs::Camera& camera )
{
    namespace Generator = pbvr::CellByCellParticleGenerator;
    pbvr::UnstructuredVolumeObject* volume;
    double total_volume = 0.0;
    float great_density;
    int steps = fi->m_start_step;
    int subvols = 0;
#ifndef CPU_VER
    int rank;
    int nprocs;
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
    int nprocs = 1;
#endif

    if ( rank == 0 )
    {
        PBVR_TIMER_STA( 16 );
        volume = CreateVolumeData( param, fi, steps, subvols );
        PBVR_TIMER_END( 16 );
        PBVR_TIMER_STA( 17 );
        total_volume += Generator::CalculateTotalVolume( volume );
        PBVR_TIMER_END( 17 );
        PBVR_TIMER_STA( 18 );
        great_density = Generator::CalculateGreatDensity( camera, *volume, param.m_subpixel_level,
                                                          param.m_sampling_step );
        PBVR_TIMER_END( 18 );

        delete volume;
    }
#ifndef CPU_VER
    PBVR_TIMER_STA( 19 );
    MPI_Bcast( &great_density, 1, MPI_FLOAT, 0, MPI_COMM_WORLD );
    PBVR_TIMER_END( 19 );
#endif


    for ( subvols = subvols + 1; subvols < fi->m_number_subvolumes; subvols++ )
    {
        if ( subvols % nprocs == rank )
        {
            PBVR_TIMER_STA( 16 );
            volume = CreateVolumeData( param, fi, steps, subvols );
            PBVR_TIMER_END( 16 );
            PBVR_TIMER_STA( 17 );
            total_volume += Generator::CalculateTotalVolume( volume );
            PBVR_TIMER_END( 17 );

            delete volume;
        }
    }
#ifndef CPU_VER
    PBVR_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, &total_volume, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    PBVR_TIMER_END( 19 );
#endif

    float total_nparticles = great_density * static_cast<float>( total_volume );
    float m_density_factor = static_cast<float>( param.m_particle_limit ) * 1000000 / total_nparticles;

    if ( m_density_factor > 1.0 ) m_density_factor = 1.0;

    return m_density_factor;
}

/**
 * メイン処理:
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char** argv )
{

// debug by @hira
#ifdef _DEBUG
//    sleep(10);
#endif

#ifndef CPU_VER
    MPI_Init( &argc, &argv );
#endif
    PBVR_TIMER_INIT();
    PBVR_TIMER_STA( 1 );
#ifdef KMATH
#ifndef CPU_VER
    km_random.init( MPI_COMM_WORLD );
#else
    km_random.init();
#endif
    km_random.seed( 1 );
#endif
    Argument param( argc, argv );
    FilterInformationList *fil = NULL;
    TransferFunctionSynthesizerCreator transfunc_creator;

    kvs::Camera camera;
    kvs::Timer timer;
    int retval = 0;
    int mpi_rank = 0;
    std::vector<PointObjectCreator*> point_creator_lst;
    pbvr::PointObject* object = NULL;
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
    int rank = 0;
    int mpi_size = 1;
#endif

    // add by @hira
    param.m_rank = rank;
    Server *server = new Server(&param);


#ifndef CPU_VER
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    if ( param.m_batch == true )
    {
        PBVR_TIMER_STA( 2 );
        PBVR_TIMER_STA( 11 );
        if ( getenv( pout.c_str() ) )
        {
            outdir = std::string( getenv( pout.c_str() ) );
        }
        else
        {
            outdir = "";
        }

        kvs::File outdir_pfx( param.m_output_data_base );
#if defined ( WIN32 )
        outdir = outdir_pfx.pathName();
#else
        outdir = outdir_pfx.pathName() + outdir_pfx.Separator();
#endif
        if ( IsDirectory( outdir ) )
        {
#if defined ( WIN32 )
            outdir = outdir + outdir_pfx.Separator();
#endif
            if ( outdir_pfx.fileName() != "" )
            {
                output = outdir + std::string( outdir_pfx.fileName().c_str() );
            }
            else
            {
                std::cerr << "Error \"-pout\" : " << "Prefix does not exist" << std::endl;
#ifdef KMATH
                km_random.finalize();
#endif
#ifndef CPU_VER
                MPI_Finalize();
#endif
                return 0;
            }
        }
        else
        {
            if ( rank == 0 ) std::cerr << outdir << " : " << "The directory does not exist" << std::endl;
#ifdef KMATH
            km_random.finalize();
#endif
#ifndef CPU_VER
            MPI_Finalize();
#endif
            return 0;
        }

        PbvrJobDispatcher jd;
        // JobDispatcher jd;
#ifndef CPU_VER
        JobCollector  jc( &jd );
        if ( ! param.m_batch_join_flag ) jc.setBatch( true );
#endif

        PBVR_TIMER_STA( 12 );
        bool pfi_Exist = false;

        // フィルタファイルリスト情報クラスをロードする。
        fil = server->loadFilterInformationList();
        if (!server->isLoadPbvrFilter()) {
            param.m_input_data_base = server->getLoadFilterFile();
        }

        if (fil != NULL) {
            if ( param.m_batch_spec_time_step )
                fil->cropTimeStep( param.m_batch_time_step[0], param.m_batch_time_step[1] );
                if (param.m_parameter_file != "")
                {
                    ParameterFile pa;
                    if (pa.LoadIN(param.m_parameter_file))
                    {
                        if (pa.getState("COORD1_SYNTH")) param.m_x_synthesis = pa.getString("COORD1_SYNTH");
                        if (pa.getState("COORD2_SYNTH")) param.m_y_synthesis = pa.getString("COORD2_SYNTH");
                        if (pa.getState("COORD3_SYNTH")) param.m_z_synthesis = pa.getString("COORD3_SYNTH");
                    }
                }

            if ( fil->m_list.size() > 0 )
            {
                for (std::vector<PointObjectCreator*>::iterator itr=point_creator_lst.begin();itr!=point_creator_lst.end();itr++) {
                    if (*itr == NULL) continue;
                    (*itr)->releaseObject();
                    delete (*itr);
                }
                point_creator_lst.clear();

                for ( int idx = 0; idx < fil->m_list.size(); idx++ )
                {
                    PointObjectCreator *point_creator = new PointObjectCreator();
                    if ( param.m_gt5d == true ) point_creator->setGT5D();
                    point_creator->setFilterInfo( fil->m_list[idx] );
                    point_creator->setCoordSynthStr( param.m_x_synthesis,
                                                param.m_y_synthesis, param.m_z_synthesis );
                    point_creator_lst.push_back( point_creator );
                }

                transfunc_creator.setFilterInfo( fil->m_list[0] );

                if ( rank == 0 )
                {
                    std::cout << " start step = "         << fil->m_total_start_steps
                            << " end step = "           << fil->m_total_end_step
                            << " time step = "          << fil->m_total_number_steps
                            << " subvolume division = " << fil->m_total_number_subvolumes
                            << std::endl;
                }

                if( rank == 0 )
                {
                    if ( param.hasOption( "pout" ) )
                    {
                        std::string minmax_file = output + "_pfi_coords_minmax.txt";
                        FILE* fp = fopen( minmax_file.c_str(), "w" );
                        if ( fp )
                        {
                            fprintf( fp, "%f %f %f %f %f %f\n",
                                     fil->m_total_min_object_coord[0],
                                     fil->m_total_min_object_coord[1],
                                     fil->m_total_min_object_coord[2],
                                     fil->m_total_max_object_coord[0],
                                     fil->m_total_max_object_coord[1],
                                     fil->m_total_max_object_coord[2] );
                            fclose( fp );
                        }
                    }
                }

                // modify by @hira at 2016/12/01
                // transfunc_creator.setRange( "t1", fil->m_total_ingredient[0].m_min, fil->m_total_ingredient[0].m_max );
                transfunc_creator.setInitializeRange( fil->m_total_ingredient[0].m_min, fil->m_total_ingredient[0].m_max );

                pfi_Exist = true;
            }
        }
        PBVR_TIMER_END( 12 );

        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
        int st, vl, wid = 0;

        PBVR_TIMER_STA( 13 );
        if ( param.m_parameter_file != "" )
        {
            ParameterFile pa;
            if ( pa.LoadIN( param.m_parameter_file ) )
            {
                if ( !param.hasOption( "sl" ) && pa.getState( "SUB_PIXEL_LEVEL" ) )
                    param.m_subpixel_level = ( size_t )pa.getInt( "SUB_PIXEL_LEVEL" );
                if ( !param.hasOption( "rl" ) && pa.getState( "REPEAT_LEVEL" ) )
                    param.m_repeat_level = ( size_t )pa.getInt( "REPEAT_LEVEL" );
                if ( !param.hasOption( "plimit" ) && pa.getState( "PARTICLE_LIMIT" ) )
                    param.m_particle_limit = pa.getInt( "PARTICLE_LIMIT" );
                if ( pa.getState( "RESOLUTION_WIDTH" ) ) param.m_window_width = pa.getInt( "RESOLUTION_WIDTH" );
                if ( pa.getState( "RESOLUTION_HEIGHT" ) ) param.m_window_height = pa.getInt( "RESOLUTION_HEIGHT" );

                if ( !pfi_Exist )
                {
                    std::string input = pa.getString( "PFI_PATH_SERVER" );
                    std::string pbvrfilter_file = pa.getString( PBVR_FILTER_PARAMFILENAME );
                    size_t pos;
                    if (!pbvrfilter_file.empty()) {
                        param.m_filter_parameter_filename = pbvrfilter_file;
                    }
                    else if (!input.empty()) {
                        param.m_input_data_base = input;
                    }
                    else {
                        if ( rank == 0 ) {
                            std::cerr << "Error: not found Parameter Filename in pa." << std::endl;
                        }
#ifdef KMATH
                        km_random.finalize();
#endif
#ifndef CPU_VER
                        MPI_Finalize();
#endif
                        return 0;
                    }

                    // フィルタファイルリスト情報クラスをロードする。
                    fil = server->loadFilterInformationList();
                    if (!server->isLoadPbvrFilter()) {
                        param.m_input_data_base = server->getLoadFilterFile();
                    }

                    if ( fil == NULL ) {
                        if ( rank == 0 ) {
                            std::cerr << "Error: parameter file (" << server->getLoadFilterFile() << ") "
                                    " doesn't exist" << std::endl;
                        }
#ifdef KMATH
                        km_random.finalize();
#endif
#ifndef CPU_VER
                        MPI_Finalize();
#endif
                        return 0;
                    }
                    if ( param.m_batch_spec_time_step )
                        fil->cropTimeStep( param.m_batch_time_step[0], param.m_batch_time_step[1] );

                    if ( fil->m_list.size() > 0 )
                    {
                        for (std::vector<PointObjectCreator*>::iterator itr=point_creator_lst.begin();itr!=point_creator_lst.end();itr++) {
                            (*itr)->releaseObject();
                            delete (*itr);
                        }
                        point_creator_lst.clear();

                        for ( int idx = 0; idx < fil->m_list.size(); idx++ )
                        {
                            PointObjectCreator *point_creator = new PointObjectCreator();
                            if ( param.m_gt5d == true ) point_creator->setGT5D();
                            point_creator->setFilterInfo( fil->m_list[idx] );
                            point_creator->setCoordSynthStr( param.m_x_synthesis,
                                                            param.m_y_synthesis, param.m_z_synthesis );
                            point_creator_lst.push_back( point_creator );
                        }

                        transfunc_creator.setFilterInfo( fil->m_list[0] );

                        if ( rank == 0 )
                        {
                            std::cout << " time step = "          << fil->m_total_number_steps
                                      << " subvolume division = " << fil->m_total_number_subvolumes
                                      << std::endl;
                        }

                        if ( param.hasOption( "pout" ) )
                        {
                            std::string minmax_file = output + "_pfi_coords_minmax.txt";
                            FILE* fp = fopen( minmax_file.c_str(), "w" );
                            if ( fp )
                            {
                                fprintf( fp, "%f %f %f %f %f %f\n",
                                         fil->m_total_min_object_coord[0],
                                         fil->m_total_min_object_coord[1],
                                         fil->m_total_min_object_coord[2],
                                         fil->m_total_max_object_coord[0],
                                         fil->m_total_max_object_coord[1],
                                         fil->m_total_max_object_coord[2] );
                                fclose( fp );
                            }
                        }

                        // modify by @hira at 2016/12/01
                        // transfunc_creator.setRange( "t1", fil->m_total_ingredient[0].m_min, fil->m_total_ingredient[0].m_max );
                        transfunc_creator.setInitializeRange( fil->m_total_ingredient[0].m_min, fil->m_total_ingredient[0].m_max );
                        pfi_Exist = true;
                    }
                }

                if ( !pfi_Exist )
                {
                    if ( rank == 0 ) std::cerr << "Error: pfifile doesn't exist" << std::endl;
#ifdef KMATH
                    km_random.finalize();
#endif
#ifndef CPU_VER
                    MPI_Finalize();
#endif
                    return 0;
                }

                if ( !param.hasOption( "crop" ) && pa.getState( "CROP_TYPE" ) )
                {
                    switch ( pa.getInt( "CROP_TYPE" ) )
                    {
                    case 1:
                    {
                        float crop_tmp[6];
                        crop_tmp[0] = fil->m_total_min_object_coord.x();
                        crop_tmp[1] = fil->m_total_min_object_coord.y();
                        crop_tmp[2] = fil->m_total_min_object_coord.z();
                        crop_tmp[3] = fil->m_total_max_object_coord.x();
                        crop_tmp[4] = fil->m_total_max_object_coord.y();
                        crop_tmp[5] = fil->m_total_max_object_coord.z();
                        if ( pa.getState( "CROP_CXMIN" ) ) crop_tmp[0] = pa.getFloat( "CROP_CXMIN" );
                        if ( pa.getState( "CROP_CYMIN" ) ) crop_tmp[1] = pa.getFloat( "CROP_CYMIN" );
                        if ( pa.getState( "CROP_CZMIN" ) ) crop_tmp[2] = pa.getFloat( "CROP_CZMIN" );
                        if ( pa.getState( "CROP_CXMAX" ) ) crop_tmp[3] = pa.getFloat( "CROP_CXMAX" );
                        if ( pa.getState( "CROP_CYMAX" ) ) crop_tmp[4] = pa.getFloat( "CROP_CYMAX" );
                        if ( pa.getState( "CROP_CZMAX" ) ) crop_tmp[5] = pa.getFloat( "CROP_CZMAX" );
                        param.m_crop.setEnable( pa.getInt( "CROP_TYPE" ) );
                        param.m_crop.set( crop_tmp );
                        // printf("min(%f,%f,%f)\n", crop_tmp[0], crop_tmp[1], crop_tmp[2]);
                        // printf("max(%f,%f,%f)\n", crop_tmp[3], crop_tmp[4], crop_tmp[5]);
                    }
                    break;

                    case 2:
                    {
                        float crop_tmp[6];
                        crop_tmp[0] = ( fil->m_total_min_object_coord.x() + fil->m_total_max_object_coord.x() ) / 2.0;
                        crop_tmp[1] = ( fil->m_total_min_object_coord.y() + fil->m_total_max_object_coord.y() ) / 2.0;
                        crop_tmp[2] = ( fil->m_total_min_object_coord.z() + fil->m_total_max_object_coord.z() ) / 2.0;
                        float tx = fil->m_total_max_object_coord.x() - crop_tmp[0];
                        float ty = fil->m_total_max_object_coord.y() - crop_tmp[1];
                        float tz = fil->m_total_max_object_coord.z() - crop_tmp[2];
                        crop_tmp[3] = sqrt( tx * tx + ty * ty + tz * tz );
                        if ( pa.getState( "CROP_SCENTERX" ) ) crop_tmp[0] = pa.getFloat( "CROP_SCENTERX" );
                        if ( pa.getState( "CROP_SCENTERY" ) ) crop_tmp[1] = pa.getFloat( "CROP_SCENTERY" );
                        if ( pa.getState( "CROP_SCENTERZ" ) ) crop_tmp[2] = pa.getFloat( "CROP_SCENTERZ" );
                        if ( pa.getState( "CROP_SRADIUS" ) ) crop_tmp[3] = pa.getFloat( "CROP_SRADIUS" );
                        crop_tmp[4] = 0.0;
                        crop_tmp[5] = 0.0;
                        param.m_crop.setEnable( pa.getInt( "CROP_TYPE" ) );;
                        param.m_crop.set( crop_tmp );
                        // printf("(%f,%f,%f) %f\n", crop_tmp[0], crop_tmp[1], crop_tmp[2], crop_tmp[3]);
                    }
                    break;

                    case 3:         // XY
                    {
                        float crop_tmp[6];
                        crop_tmp[0] = ( fil->m_total_min_object_coord.x() + fil->m_total_max_object_coord.x() ) / 2.0;
                        crop_tmp[1] = ( fil->m_total_min_object_coord.y() + fil->m_total_max_object_coord.y() ) / 2.0;
                        crop_tmp[2] = fil->m_total_min_object_coord.z();
                        float tx = fil->m_total_max_object_coord.x() - crop_tmp[0];
                        float ty = fil->m_total_max_object_coord.y() - crop_tmp[1];
                        crop_tmp[3] = sqrt( tx * tx + ty * ty );
                        crop_tmp[4] = fil->m_total_max_object_coord.z() - fil->m_total_min_object_coord.z();
                        if ( pa.getState( "CROP_PCENTERX" ) ) crop_tmp[0] = pa.getFloat( "CROP_PCENTERX" );
                        if ( pa.getState( "CROP_PCENTERY" ) ) crop_tmp[1] = pa.getFloat( "CROP_PCENTERY" );
                        if ( pa.getState( "CROP_PCENTERZ" ) ) crop_tmp[2] = pa.getFloat( "CROP_PCENTERZ" );
                        if ( pa.getState( "CROP_PRADIUS" ) ) crop_tmp[3] = pa.getFloat( "CROP_PRADIUS" );
                        if ( pa.getState( "CROP_PHEIGHT" ) ) crop_tmp[4] = pa.getFloat( "CROP_PHEIGHT" );
                        crop_tmp[5] = 0.0;
                        param.m_crop.setEnable( pa.getInt( "CROP_TYPE" ) );;
                        param.m_crop.set( crop_tmp );
                        // printf("xy (%f,%f,%f) %f, %f\n", crop_tmp[0], crop_tmp[1], crop_tmp[2],
                        //        crop_tmp[3], crop_tmp[4]);
                    }
                    break;

                    case 4:         // YZ
                    {
                        float crop_tmp[6];
                        crop_tmp[0] = fil->m_total_min_object_coord.x();
                        crop_tmp[1] = ( fil->m_total_min_object_coord.y() + fil->m_total_max_object_coord.y() ) / 2.0;
                        crop_tmp[2] = ( fil->m_total_min_object_coord.z() + fil->m_total_max_object_coord.z() ) / 2.0;
                        float ty = fil->m_total_max_object_coord.y() - crop_tmp[1];
                        float tz = fil->m_total_max_object_coord.z() - crop_tmp[2];
                        crop_tmp[3] = sqrt( ty * ty + tz * tz );
                        crop_tmp[4] = fil->m_total_max_object_coord.x() - fil->m_total_min_object_coord.x();
                        if ( pa.getState( "CROP_PCENTERX" ) ) crop_tmp[0] = pa.getFloat( "CROP_PCENTERX" );
                        if ( pa.getState( "CROP_PCENTERY" ) ) crop_tmp[1] = pa.getFloat( "CROP_PCENTERY" );
                        if ( pa.getState( "CROP_PCENTERZ" ) ) crop_tmp[2] = pa.getFloat( "CROP_PCENTERZ" );
                        if ( pa.getState( "CROP_PRADIUS" ) ) crop_tmp[3] = pa.getFloat( "CROP_PRADIUS" );
                        if ( pa.getState( "CROP_PHEIGHT" ) ) crop_tmp[4] = pa.getFloat( "CROP_PHEIGHT" );
                        crop_tmp[5] = 0.0;
                        param.m_crop.setEnable( pa.getInt( "CROP_TYPE" ) );;
                        param.m_crop.set( crop_tmp );
                        // printf("yz (%f,%f,%f) %f, %f\n", crop_tmp[0], crop_tmp[1], crop_tmp[2],
                        //        crop_tmp[3], crop_tmp[4]);
                    }
                    break;

                    case 5:         // XZ
                    {
                        float crop_tmp[6];
                        crop_tmp[0] = ( fil->m_total_min_object_coord.x() + fil->m_total_max_object_coord.x() ) / 2.0;
                        crop_tmp[1] = fil->m_total_min_object_coord.y();
                        crop_tmp[2] = ( fil->m_total_min_object_coord.z() + fil->m_total_max_object_coord.z() ) / 2.0;
                        float tx = fil->m_total_max_object_coord.x() - crop_tmp[0];
                        float tz = fil->m_total_max_object_coord.z() - crop_tmp[2];
                        crop_tmp[3] = sqrt( tx * tx + tz * tz );
                        crop_tmp[4] = fil->m_total_max_object_coord.y() - fil->m_total_min_object_coord.y();
                        if ( pa.getState( "CROP_PCENTERX" ) ) crop_tmp[0] = pa.getFloat( "CROP_PCENTERX" );
                        if ( pa.getState( "CROP_PCENTERY" ) ) crop_tmp[1] = pa.getFloat( "CROP_PCENTERY" );
                        if ( pa.getState( "CROP_PCENTERZ" ) ) crop_tmp[2] = pa.getFloat( "CROP_PCENTERZ" );
                        if ( pa.getState( "CROP_PRADIUS" ) ) crop_tmp[3] = pa.getFloat( "CROP_PRADIUS" );
                        if ( pa.getState( "CROP_PHEIGHT" ) ) crop_tmp[4] = pa.getFloat( "CROP_PHEIGHT" );
                        crop_tmp[5] = 0.0;
                        param.m_crop.setEnable( pa.getInt( "CROP_TYPE" ) );;
                        param.m_crop.set( crop_tmp );
                        // printf("xz (%f,%f,%f) %f, %f\n", crop_tmp[0], crop_tmp[1], crop_tmp[2],
                        //        crop_tmp[3], crop_tmp[4]);
                    }
                    break;

                    default:
                        break;
                    }
                }
                if ( pa.getState( "SERVER_LATENCY" ) ) param.m_latency_threshold = ( double )pa.getFloat( "SERVER_LATENCY" );
                if ( pa.getState( "SERVER_JOB_NUMBER" ) ) param.m_job_id_pack_size = pa.getInt( "SERVER_JOB_NUMBER" );

                transfunc_creator.setParameterFile( pa );

                // Coordinate Synth
                if ( pa.getState( "COORD1_SYNTH" ) ) param.m_x_synthesis = pa.getString( "COORD1_SYNTH" );
                if ( pa.getState( "COORD2_SYNTH" ) ) param.m_y_synthesis = pa.getString( "COORD2_SYNTH" );
                if ( pa.getState( "COORD3_SYNTH" ) ) param.m_z_synthesis = pa.getString( "COORD3_SYNTH" );

            }
        } // end of if( param.m_parameter_file != "" )

        if ( !pfi_Exist ) {
            if ( rank == 0 ) {
                std::cerr << "Error: parameter file (" << server->getLoadFilterFile() << ") "
                        " doesn't exist" << std::endl;
            }
#ifdef KMATH
            km_random.finalize();
#endif
#ifndef CPU_VER
            MPI_Finalize();
#endif
            return 0;
        }

        PBVR_TIMER_END( 13 );

        transfunc_creator.setAsisTransferFunction ( param.m_transfer_function );
        // delete by @hira at 2016/12/01 : "t"オプションは存在しない。
        // if ( param.hasOption( "t" ) ) transfunc_creator.setTransferFunction ( "t1", param.m_transfer_function );
        param.m_transfunc_synthesizer = transfunc_creator.create();
        param.m_transfunc_synthesizer->optimize2();

        camera.setWindowSize( param.m_window_width, param.m_window_height );
        PBVR_TIMER_STA( 15 );
        //kawamura2 : It calculates subpixel level from particle limit.
        param.m_sampling_step = CalculateSamplingStep( fil );
        param.m_subpixel_level = CalculateSubpixelLevel( param, fil, camera );
        PBVR_TIMER_END( 15 );

        PBVR_TIMER_END( 11 );

        std::cout << "subpixel level = " << param.m_subpixel_level << std::endl;
        std::cout << "sampling_step = " << param.m_sampling_step << std::endl;

        // フィルタファイルリスト情報を設定する：add by @hira at 2016/12/01
        jd.setFilterInformationList(fil);

        if ( param.m_batch_join_flag )
        {
            //
            // batch fjoin mode: integrate into a single file for each time step
            //
            pbvr::PointObject joined_obj;
            int32_t stp;
#if defined(CPU_VER)
            PBVR_TIMER_STA( 490 );
            for ( stp = fil->m_total_start_steps; stp <= fil->m_total_end_step; stp++ )
            {
                // add by @hira at 2016/12/01
                // ステップの物理量の読込みを行う。
                PBVR_TIMER_STA( 500 );
                fil->loadMpiSubvolume(stp);
                PBVR_TIMER_END( 500 );

                joined_obj.clear();
                PBVR_TIMER_STA( 14 );
                if ( param.m_crop.isEnabled() )
                {
                    jd.initialize( stp, stp, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size,
                                   param.m_crop.getMinCoord(),
                                   param.m_crop.getMaxCoord() );
                }
                else
                {
                    jd.initialize( stp, stp, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size );
                }
                PBVR_TIMER_END( 14 );

                PBVR_TIMER_STA( 20 );
                while ( jd.dispatchNext( wid, &st, &vl ) )
                {
                    PBVR_TIMER_STA( 90 );

                    int xvl, fidx;
                    fidx = fil->getFileIndex( vl, &xvl );
                    FilterInformationFile* fi = fil->m_list[fidx];

                    std::stringstream suffix, suffix2;
                    suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << st
                           << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                           << '_' << std::setw( 7 ) << std::setfill( '0' ) << fi->m_number_subvolumes;
                    //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                    kvs::File ifpx( fi->m_file_path );
                    param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                       + ifpx.baseName() + suffix.str() + ".kvsml";

                    int timeStep = 1;
                    if (fi->isPbvrFilterInfo()) {
                        PBVR_TIMER_STA( 99 );
                        object = point_creator_lst[fidx]->run( param, camera, timeStep, st, xvl );
                        PBVR_TIMER_END( 99 );
                    }

                    else if ( fi->m_file_type == 1 || fi->m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                        PBVR_TIMER_STA( 99 );
                        object = point_creator_lst[fidx]->run( param, camera, timeStep, st, xvl );
                        PBVR_TIMER_END( 99 );
                    }
                    else            // filetype: kvsml
                    {
                        PBVR_TIMER_STA( 99 );
                        object = point_creator_lst[fidx]->run( param, camera, timeStep, st );
                        PBVR_TIMER_END( 99 );
                    }

                    if ( object )
                    {
                        int nvertices = object->coords().size() / 3;
                        if ( nvertices > 0 )
                        {
                            joined_obj.add( *object );
                        }
                    }
                    // add by @hira at 2016/12/01
                    //point_creator_lst[fidx]->releaseObject();

                    PBVR_TIMER_END( 90 );
                } // END OF WHILE(DispatchNext)Job
                PBVR_TIMER_END( 20 );

                //printf( "  %lu perticles generated\n", joined_obj.coords().size() / 3);
				printf("  %zu perticles generated\n", joined_obj.coords().size() / 3);

                // WRITE joined_obj
                std::stringstream suffix2;
                suffix2 << '_' << std::setw( 5 ) << std::setfill( '0' ) << stp
                        << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1
                        << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1;
                std::string basename = output + suffix2.str();
                joined_obj.setSize( static_cast<kvs::Real32>( param.m_subpixel_level ) );
                KVSMLObjectPointWriter( joined_obj, basename );

                // add by @hira at 2016/12/01
                // ステップの物理量の読込後処理を行う。
                fil->releaseMpiSubvolume();

            } // end of for(stp)
            PBVR_TIMER_END( 490 );

#else // ! CPU_VER
            PBVR_TIMER_STA( 490 );
            for ( stp = fil->m_total_start_steps; stp <= fil->m_total_end_step; stp++ )
            {
                // add by @hira at 2016/12/01
                // ステップの物理量の読込みを行う。
              
                PBVR_TIMER_STA( 500 );
                fil->loadMpiSubvolume(stp);
                PBVR_TIMER_END( 500 );

                joined_obj.clear();
                PBVR_TIMER_STA( 14 );
                if ( param.m_crop.isEnabled() )
                {
                    jd.initialize( stp, stp, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size,
                                   param.m_crop.getMinCoord(),
                                   param.m_crop.getMaxCoord() );
                }
                else
                {
                    jd.initialize( stp, stp, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size );
                }
                PBVR_TIMER_END( 14 );

                PBVR_TIMER_STA( 20 );
                while ( jd.dispatchNext( wid, &st, &vl ) )
                {
                    int xvl, fidx;
                    fidx = fil->getFileIndex( vl, &xvl );
                    FilterInformationFile *fi = fil->m_list[fidx];

                    // modify by @hira at 2016/12/01
                    if ( rank > 0 || mpi_size == 1)
                    {
                        //--------------------- WORKER --------------------
                        PBVR_TIMER_STA( 90 );

                        std::stringstream suffix, suffix2;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << st
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << fi->m_number_subvolumes;
                        //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                        kvs::File ifpx( fi->m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                           + ifpx.baseName() + suffix.str() + ".kvsml";

                        int timeStep = 1;
                        try
                        {
                        if (fi->isPbvrFilterInfo()) {
                            PBVR_TIMER_STA( 99 );
                            object = point_creator_lst[fidx]->run( param, camera, timeStep, st, xvl );
                            PBVR_TIMER_END( 99 );
                        }
                        else if ( fi->m_file_type == 1 || fi->m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                        {
                            PBVR_TIMER_STA( 99 );
                            object = point_creator_lst[fidx]->run( param, camera, timeStep, st, xvl );
                            PBVR_TIMER_END( 99 );
                        }
                        else            // filetype: kvsml
                        {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, camera, timeStep, st );
                                PBVR_TIMER_END( 99 );
                        }
                        }
                        catch ( const std::runtime_error& e )
                        {
                            std::cerr << e.what();
                            nan_error = true;
                        }
                        PBVR_TIMER_STA( 400 );

                        if (mpi_size > 1) {
                            VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
                            jc.jobCollect( object, p_vr, &nan_error, &wid );
                        }

                        PBVR_TIMER_END( 400 );
                        PBVR_TIMER_END( 90 );
                    }
                    if ( rank == 0 || mpi_size == 1)
                    {
                        // ---------------------- MASTER ---------------------
                        PBVR_TIMER_STA( 50 );
                        pbvr::PointObject obj;
                        PBVR_TIMER_STA( 55 );

                        if (mpi_size > 1) {
                            VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
                            jc.jobCollect( &obj, p_vr, &nan_error, &wid );
                            jd.setWorkerStatus( wid, 1 );
                        }
                        else {
                            obj = *object;
                        }
                        int nvertices = obj.coords().size() / 3;
                        if ( nvertices > 0 )
                        {
                            joined_obj.add( obj );
                        }

                        PBVR_TIMER_END( 55 );
                        PBVR_TIMER_END( 50 );
                    }
                } // END OF WHILE(DispatchNext)Job
                PBVR_TIMER_END( 20 );

                if ( rank == 0 )
                {
                    printf( "  %lu perticles generated\n", joined_obj.coords().size() / 3);

                    // WRITE joined_obj
                    std::stringstream suffix2;
                    suffix2 << '_' << std::setw( 5 ) << std::setfill( '0' ) << stp
                            << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1
                            << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1;
                    std::string basename = output + suffix2.str();
                    joined_obj.setSize( static_cast<kvs::Real32>( param.m_subpixel_level ) );
                    KVSMLObjectPointWriter( joined_obj, basename );
                }

                // add by @hira at 2016/12/01
                // ステップの物理量の読込後処理を行う。
                fil->releaseMpiSubvolume();

            } // end of for(stp)
            PBVR_TIMER_END( 490 );

#endif // ! CPU_VER
        }
        else
        {
            //
            // NOT fjoin mode: file will be created apart in each subvolume
            //
            int32_t stp;
            PBVR_TIMER_STA( 490 );
            for ( stp = fil->m_total_start_steps; stp <= fil->m_total_end_step; stp++ )
            {
                // add by @hira at 2016/12/01
                // ステップの物理量の読込みを行う。

                PBVR_TIMER_STA( 500 );
                fil->loadMpiSubvolume(stp);
                PBVR_TIMER_END( 500 );

                PBVR_TIMER_STA( 14 );
#if 0		// modify by @hira at 2016/12/01
                if ( param.m_crop.isEnabled() )
                {
                    jd.initialize( fil->m_total_start_steps, fil->m_total_end_step, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size,
                                   param.m_crop.getMinCoord(),
                                   param.m_crop.getMaxCoord() );
                }
                else
                {
                    jd.initialize( fil->m_total_start_steps, fil->m_total_end_step, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size );
                }
#endif
                if ( param.m_crop.isEnabled() )
                {
                    jd.initialize( stp, stp, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size,
                                   param.m_crop.getMinCoord(),
                                   param.m_crop.getMaxCoord() );
                }
                else
                {
                    jd.initialize( stp, stp, fil->m_total_number_subvolumes,
                                   fil->m_total_min_subvolume_coord,
                                   fil->m_total_max_subvolume_coord,
                                   param.m_latency_threshold, param.m_job_id_pack_size );
                }
                PBVR_TIMER_END( 14 );

                PBVR_TIMER_STA( 20 );
                while ( jd.dispatchNext( wid, &st, &vl ) )
                {

#ifdef CPU_VER
                    if ( rank == 0 )
#else
                    // modify by @hira at 2016/12/01
                    if ( rank > 0 || mpi_size == 1)
#endif
                    {
                        //--------------------- WORKER --------------------
                        PBVR_TIMER_STA( 90 );

                        int xvl, fidx;
                        fidx = fil->getFileIndex( vl, &xvl );
                        FilterInformationFile *fi = fil->m_list[fidx];

                        std::stringstream suffix, suffix2;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << st
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << fi->m_number_subvolumes;
                        suffix2 << '_' << std::setw( 5 ) << std::setfill( '0' ) << st
                                << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( vl + 1 )
                                << '_' << std::setw( 7 ) << std::setfill( '0' ) << fil->m_total_number_subvolumes;
                        //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                        kvs::File ifpx( fi->m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                           + ifpx.baseName() + suffix.str() + ".kvsml";
                        std::string basename = output + suffix2.str();

                        int timeStep = 1;
                        try
                        {
                            if ( fi->isPbvrFilterInfo()) {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, camera, timeStep, st, xvl );
                                PBVR_TIMER_END( 99 );
                            }
                            else if ( fi->m_file_type == 1 || fi->m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, camera, timeStep, st, xvl );
                                PBVR_TIMER_END( 99 );
                            }
                            else            // filetype: kvsml
                            {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, camera, timeStep, st );
                                PBVR_TIMER_END( 99 );
                            }
                            PBVR_TIMER_STA( 80 );
                            //printf( "  %lu perticles generated\n", object->coords().size() / 3);
							printf("  %zu perticles generated\n", object->coords().size() / 3);
							//サイズの代わりにサブピクセルレベルを代入.
                            object->setSize( static_cast<kvs::Real32>( param.m_subpixel_level ) );
                            KVSMLObjectPointWriter( *object, basename );
                            PBVR_TIMER_END( 80 );
                        }
                        catch ( const std::runtime_error& e )
                        {
                            std::cerr << e.what();
                            nan_error = true;
                        }
                        PBVR_TIMER_STA( 400 );
#ifndef CPU_VER
                        // modify by @hira at 2016/12/01
                        if ( mpi_size > 1) {
                            VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
                            jc.jobCollect( object, p_vr, &nan_error, &wid );
                        }
#endif

                        point_creator_lst[fidx]->releaseObject();
                        object = NULL;

                        PBVR_TIMER_END( 400 );
                        PBVR_TIMER_END( 90 );
                    }
                    else
                    {
                        // ---------------------- MASTER ---------------------
                        PBVR_TIMER_STA( 50 );
                        pbvr::PointObject obj;
                        PBVR_TIMER_STA( 55 );
#ifndef CPU_VER
                        // modify by @hira at 2016/12/01
                        if ( mpi_size > 1) {
                            VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
                            jc.jobCollect( &obj, p_vr, &nan_error, &wid );
                            jd.setWorkerStatus( wid, 1 );
                        }
#endif
                        PBVR_TIMER_END( 55 );
                        PBVR_TIMER_END( 50 );
                    }

                } // END OF WHILE(DispatchNext)Job

                // add by @hira at 2016/12/01
                // ステップの物理量の読込後処理を行う。
                fil->releaseMpiSubvolume();

                PBVR_TIMER_END( 20 );
            } // end of for(stp)
            PBVR_TIMER_END( 490 );
        } // end of ! m_batch_join_flag

        delete param.m_transfunc_synthesizer;

        PBVR_TIMER_END( 2 );
    } // end of batch mode

    else  //=================== client-server mode ===================
    {

        char* buf;
        int bsz = 0;
        // JobDispatcher jd;
        PbvrJobDispatcher jd;
#ifndef CPU_VER
        JobCollector  jc( &jd );
#endif
        int st, vl, wid = 0;

        int c_bins_size = 0;
        int o_bins_size = 0;
        kvs::UInt64* tmp_c_bins;
        kvs::UInt64* tmp_o_bins;
        fil = NULL;

        if ( rank > 0 )
        {
            //--------------------- WORKER --------------------
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new kvs::Camera();

            bool loop = true;

            while ( loop )
            {
                static int timer_count = 0;

                // recv cltMes from process 0 >>
                bsz = server->recv_bcast(&clntMes);
                if ( bsz < 0 ) {
                    loop = false;
                    break; // terminate server
                }

                std::cout << "Rank " << rank << ": Recv Client Message" << std::endl;
                // recv cltMes from process 0 <<
                param.m_input_data_base = clntMes.m_input_directory;
                param.m_filter_parameter_filename = clntMes.m_filter_parameter_filename;

                // modify by @hira at 2016/12/01
                if ( fil == NULL || clntMes.m_initialize_parameter == -3 ) {
                    if (fil != NULL) delete fil;
                    // フィルタファイルリスト情報クラスをロードする。
                    fil = server->loadFilterInformationList();
                    if (!server->isLoadPbvrFilter()) {
                        param.m_input_data_base = server->getLoadFilterFile();
                    }
                    continue;
                }

                if ( clntMes.m_time_parameter == 0 )
                {
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                }
                else if (fil != NULL && fil->m_list.size() > 0)
                {
//                  param.m_transfer_function = pbvr::TransferFunction(); // *( clntMes.m_transfer_function );
                    timer_count++;
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_component_Id = clntMes.m_rendering_id;
                    param.m_crop.setEnable( clntMes.m_enable_crop_region );
                    param.m_crop.set( clntMes.m_crop_region );
                    param.m_input_data_base = clntMes.m_input_directory;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;
                    // add by @hira at 2016/12/01
                    param.m_filter_parameter_filename = clntMes.m_filter_parameter_filename;

                    for (std::vector<PointObjectCreator*>::iterator itr=point_creator_lst.begin();itr!=point_creator_lst.end();itr++) {
                        (*itr)->releaseObject();
                        delete (*itr);
                    }
                    point_creator_lst.clear();

                    for ( int idx = 0; idx < fil->m_list.size(); idx++ )
                    {
                        PointObjectCreator *point_creator = new PointObjectCreator();
                        if ( param.m_gt5d == true ) point_creator->setGT5D();
                        point_creator->setFilterInfo( fil->m_list[idx] );
                        point_creator->setCoordSynthStr( clntMes.m_x_synthesis,
                                                        clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                        point_creator_lst.push_back( point_creator );
                    }

                    transfunc_creator.setFilterInfo( fil->m_list[0] );
                    transfunc_creator.setProtocol( clntMes );
                    transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_synthesizer->optimize2();

                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    // フィルタファイルリスト情報を設定する：add by @hira at 2016/12/01
                    jd.setFilterInformationList(fil);

                    if ( param.m_crop.isEnabled() )
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, fil->m_total_number_subvolumes,
                                       fil->m_total_min_subvolume_coord,
                                       fil->m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size,
                                       param.m_crop.getMinCoord(),
                                       param.m_crop.getMaxCoord() );
                    }
                    else
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, fil->m_total_number_subvolumes,
                                       fil->m_total_min_subvolume_coord,
                                       fil->m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size );
                    }

                    param.m_sampling_step = CalculateSamplingStep( fil );
                    param.m_subpixel_level = CalculateSubpixelLevel( param, fil, *clntMes.m_camera );
                    param.m_particle_limit_pre = param.m_particle_limit;

                    int tf_count = clntMes.m_transfer_function.size();
                    int c_nbins = DEFAULT_NBINS;
                    int o_nbins = DEFAULT_NBINS;

                    c_bins_size = 0;
                    o_bins_size = 0;

                    for ( int tf = 0; tf < tf_count; tf++ )
                    {
                        c_bins_size += c_nbins;
                        o_bins_size += o_nbins;
                    }

                    tmp_c_bins = new kvs::UInt64[c_bins_size];
                    tmp_o_bins = new kvs::UInt64[o_bins_size];

                    for ( int tf = 0; tf < c_bins_size; tf++ )
                    {
                        tmp_c_bins[tf] = 0;
                    }

                    for ( int tf = 0; tf < o_bins_size; tf++ )
                    {
                        tmp_o_bins[tf] = 0;
                    }

                    // add by @hira at 2016/12/01
                    // ステップの物理量の読込みを行う。
                    PBVR_TIMER_STA( 500 );
                    fil->loadMpiSubvolume(clntMes.m_step);
                    PBVR_TIMER_END( 500 );

                    while ( jd.dispatchNext( wid, &st, &vl ) )
                    {
                        int xvl, fidx;
                        fidx = fil->getFileIndex( vl, &xvl );
                        FilterInformationFile *fi = fil->m_list[fidx];

                        std::stringstream suffix;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << fi->m_number_subvolumes;
                        //param.input_data = param.input_data_base + suffix.str() + ".kvsml";
                        kvs::File ifpx( fi->m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                           + ifpx.baseName() + suffix.str() + ".kvsml";
                        int timeStep = 1;
                        try
                        {
                            if (fi->isPbvrFilterInfo()) {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, *clntMes.m_camera, timeStep, st, xvl );
                                PBVR_TIMER_END( 99 );
                            }
                            else if ( fi->m_file_type == 1 || fi->m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, *clntMes.m_camera, timeStep, st, xvl );
                                PBVR_TIMER_END( 99 );
                            }
                            else     // filetype: kvsml
                            {
                                PBVR_TIMER_STA( 99 );
                                object = point_creator_lst[fidx]->run( param, *clntMes.m_camera, timeStep, st );
                                PBVR_TIMER_END( 99 );
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

                        for ( int tf = 0; tf < object->getColorHistogram().size(); tf++ )
                        {
                            c_nbins = object->getColorHistogram()[ tf ].nbins();
                            for ( int res = 0; res < c_nbins; res++ )
                            {
                                tmp_c_bins[c_count] += static_cast<kvs::UInt64>( object->getColorHistogram()[ tf ][res] );
                                c_count++;
                            }
                        }

                        for ( int tf = 0; tf < object->getOpacityHistogram().size(); tf++ )
                        {
                            o_nbins = object->getOpacityHistogram()[ tf ].nbins();
                            for ( int res = 0; res < o_nbins; res++ )
                            {
                                tmp_o_bins[o_count] += static_cast<kvs::UInt64>( object->getOpacityHistogram()[ tf ][res] );
                                o_count++;
                            }
                        }

                    } // end of while(DispatchNext)

                    // add by @hira at 2016/12/01
                    // ステップの物理量の読込後処理を行う。
                    fil->releaseMpiSubvolume();

#ifndef CPU_VER

                    MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
                    MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
                    delete[] tmp_c_bins;
                    delete[] tmp_o_bins;
#endif
                    if ( timer_count == PBVR_TIMER_COUNT_NUM )
                    {
                        PBVR_TIMER_END( 1 );
                        PBVR_TIMER_FIN();
                    }
                    delete param.m_transfunc_synthesizer;
                }
            } // end of while(loop)
            delete clntMes.m_camera;
        }
        else                    // rank == 0
        {
            //--------------------- MASTER --------------------
            int ptss;

            assert( jpv::ParticleTransferUtils::isLittleEndian() );
            timer.start();
            PBVR_TIMER_STA( 10 );
            PBVR_TIMER_END( 10 );
            timer.stop();
            std::cout << "first reading time[ms]:" << timer.msec() << std::endl;

            jpv::ParticleTransferServer pts;
            ptss = pts.initializeServer( param.m_port );
#if 0
            if ( ptss == 0 )
            {
                pts.acceptServer();
                jpv::ParticleTransferServerMessage servMes;
                jpv::ParticleTransferClientMessage clntMes;

                pts.recvMessage( &clntMes );
                param.m_input_data_base = clntMes.m_input_directory;
                // add by @hira at 2016/12/01
                param.m_filter_parameter_filename = clntMes.m_filter_parameter_filename;

                // send cltMes to all worker process >>
                bsz = server->send_bcast(&clntMes);
                if ( fil == NULL || clntMes.m_initialize_parameter == -3 ) {
                    if (fil != NULL) delete fil;
                    // フィルタ情報クラスをロードする。
                    fil = server->loadFilterInformationList();
                    if (!server->isLoadPbvrFilter()) {
                        param.m_input_data_base = server->getLoadFilterFile();
                    }

                    if ( fil->m_list.size() <= 0 ) {
                        std::cerr << "Error: parameter file (" << server->getLoadFilterFile() << ") "
                                  " doesn't exist" << std::endl;
                        bsz = -1;
#ifndef CPU_VER
                        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
#ifdef KMATH
                        km_random.finalize();
#endif
#ifndef CPU_VER
                        MPI_Finalize();
#endif
                        return 0;
                    }
                }

                for (std::vector<PointObjectCreator*>::iterator itr=point_creator_lst.begin();itr!=point_creator_lst.end();itr++) {
//                        (*itr)->releaseObject();
//                        delete (*itr);
                }
                point_creator_lst.clear();
                for ( int idx = 0; idx < fil->m_list.size(); idx++ )
                {
                    PointObjectCreator *point_creator = new PointObjectCreator();
                    if ( param.m_gt5d == true ) point_creator->setGT5D();
                    point_creator->setFilterInfo( fil->m_list[idx] );
                    point_creator->setCoordSynthStr( clntMes.m_x_synthesis,
                                                    clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                    point_creator_lst.push_back( point_creator );
                }

                transfunc_creator.setFilterInfo( fil->m_list[0] );

                std::cout << " start step = "         << fil->m_total_start_steps
                          << " end step = "           << fil->m_total_end_step
                          << " time step = "          << fil->m_total_number_steps
                          << " subvolume division = " << fil->m_total_number_subvolumes
                          << std::endl;

                transfunc_creator.setProtocol( clntMes );
                TransferFunctionSynthesizer* tfs = transfunc_creator.create();
                VariableRange range = RangeEstimater::EstimationList( 0, fil, *tfs );
                delete tfs;

                strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                servMes.m_server_status = 0; // Add 2016.1.4
                servMes.m_number_particle = 0;
                servMes.m_transfer_function_count = 0;
                servMes.m_number_volume_divide = fil->m_total_number_subvolumes;
                servMes.m_time_step = fil->m_total_start_steps;
                servMes.m_start_step = fil->m_total_start_steps;
                servMes.m_end_step = fil->m_total_end_step;
                servMes.m_number_step = fil->m_total_number_steps;
                servMes.m_min_object_coord[0] = fil->m_total_min_object_coord[0];
                servMes.m_min_object_coord[1] = fil->m_total_min_object_coord[1];
                servMes.m_min_object_coord[2] = fil->m_total_min_object_coord[2];
                servMes.m_max_object_coord[0] = fil->m_total_max_object_coord[0];
                servMes.m_max_object_coord[1] = fil->m_total_max_object_coord[1];
                servMes.m_max_object_coord[2] = fil->m_total_max_object_coord[2];
                servMes.m_min_value = fil->m_total_min_value;
                servMes.m_max_value = fil->m_total_max_value;
                servMes.m_number_nodes = fil->m_total_number_nodes;
                servMes.m_number_elements = fil->m_total_number_elements;
                servMes.m_element_type = fil->m_list[0]->m_elem_type;
                servMes.m_file_type = fil->m_list[0]->m_file_type;
                servMes.m_number_ingredients = fil->m_list[0]->m_number_ingredients;
                servMes.m_variable_range = range;
                servMes.m_flag_send_bins = 0;

                servMes.m_message_size = servMes.byteSize();
                pts.sendMessage( servMes );
            }
#endif
            jpv::ParticleTransferServerMessage servMes;
            jpv::ParticleTransferClientMessage clntMes;
            clntMes.m_camera = new kvs::Camera();

            // クライアント接続待ち
            pts.acceptServer();

            while ( ( ptss != -1 ) && ( pts.good() ) )
            {
                static int timer_count = 0;

                ptss = pts.recvMessage( &clntMes );
                if ( ptss == -1 ) break;
                /* 140319 for client stop by Ctrl+c */
                signal( SIGABRT, SignalHandler );
                signal( SIGTERM, SignalHandler );
                signal( SIGINT, SignalHandler ); /* SIGINT is invalid here, because mpiexec uses it. */
//              signal( SIGSEGV, SignalHandler );

                // debug by @hira at 2016/12/01
                double server_start = GetTime();

                if ( SigServer )
                {
                    clntMes.m_initialize_parameter = -2;
                    std::cout << "*** SigServer" << clntMes.m_initialize_parameter << std::endl;
                }
                else
                {
                    /* 140319 for client stop by Ctrl+c */
                    if ( clntMes.m_initialize_parameter != -3 )
                    {
                        clntMes.m_input_directory = param.m_input_data_base;
                        clntMes.m_filter_parameter_filename = param.m_filter_parameter_filename;
                    }
                }

                std::cout << "Recieve message initParam = " << clntMes.m_initialize_parameter << std::endl;
                if ( clntMes.m_initialize_parameter == -1 )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
                    servMes.m_flag_send_bins = 0;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                    pts.disconnect();

                    pts.acceptServer();
                }
                else if ( clntMes.m_initialize_parameter == -2 )
                {

                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    // ADD by FEAST 2015.12.24
                    servMes.m_server_status = 0;
                    // ADD END 2015.12.24
                    servMes.m_number_particle = 0;
                    servMes.m_flag_send_bins = 0;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                    break;
                }
                else if ( clntMes.m_initialize_parameter == -3 ) // change PFI file.
                {
                    param.m_input_data_base = clntMes.m_input_directory;
                    // add by @hira at 2016/12/01
                    param.m_filter_parameter_filename = clntMes.m_filter_parameter_filename;

                    // send cltMes to all worker process >>
                    bsz = server->send_bcast(&clntMes);

                    // add by @hira at 2016/12/01
                    if (fil != NULL) delete fil;
                    // フィルタ情報クラスをロードする。
                    fil = server->loadFilterInformationList();
                    if (!server->isLoadPbvrFilter()) {
                        param.m_input_data_base = server->getLoadFilterFile();
                    }
                    if (fil == NULL || fil->m_list.size() <= 0 ) {
                        if ( rank == 0 )
                            std::cerr << "Error: parameter file (" << server->getLoadFilterFile() << ") "
                                  " doesn't exist" << std::endl;
                        bsz = -1;
#ifndef CPU_VER
                        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
#ifdef KMATH
                        km_random.finalize();
#endif
#ifndef CPU_VER
                        MPI_Finalize();
#endif
                        return 0;
                    }

                    if ( clntMes.m_step > fil->m_total_end_step )
                    {
                        clntMes.m_step = fil->m_total_end_step;
                    }
                    else if ( clntMes.m_step < fil->m_total_start_steps )
                    {
                        clntMes.m_step = fil->m_total_start_steps;
                    }

                    if ( fil->m_list.size() > 0 )
                    {
                        point_creator_lst.clear();
                        for ( int idx = 0; idx < fil->m_list.size(); idx++ )
                        {
                            PointObjectCreator *point_creator = new PointObjectCreator();
                            if ( param.m_gt5d == true ) point_creator->setGT5D();
                            point_creator->setFilterInfo( fil->m_list[idx] );
                            point_creator->setCoordSynthStr( clntMes.m_x_synthesis,
                                                            clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                            point_creator_lst.push_back( point_creator );
                        }

                        transfunc_creator.setFilterInfo( fil->m_list[0] );

                        std::cout << " time step = "          << fil->m_total_number_steps
                                  << " subvolume division = " << fil->m_total_number_subvolumes
                                  << std::endl;

                    }

                    transfunc_creator.setProtocol( clntMes );
                    TransferFunctionSynthesizer* tfs = transfunc_creator.create();
                    VariableRange range = RangeEstimater::EstimationList( 0, fil, *tfs );
                    delete tfs;

                    strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
                    servMes.m_number_volume_divide = fil->m_total_number_subvolumes;
                    servMes.m_time_step = fil->m_total_start_steps;
                    servMes.m_start_step = fil->m_total_start_steps;
                    servMes.m_end_step = fil->m_total_end_step;
                    servMes.m_number_step = fil->m_total_number_steps;
                    servMes.m_min_object_coord[0] = fil->m_total_min_object_coord[0];
                    servMes.m_min_object_coord[1] = fil->m_total_min_object_coord[1];
                    servMes.m_min_object_coord[2] = fil->m_total_min_object_coord[2];
                    servMes.m_max_object_coord[0] = fil->m_total_max_object_coord[0];
                    servMes.m_max_object_coord[1] = fil->m_total_max_object_coord[1];
                    servMes.m_max_object_coord[2] = fil->m_total_max_object_coord[2];
                    servMes.m_min_value = fil->m_total_min_value;
                    servMes.m_max_value = fil->m_total_max_value;
                    servMes.m_number_nodes = fil->m_total_number_nodes;
                    servMes.m_number_elements = fil->m_total_number_elements;
                    servMes.m_element_type = fil->m_list[0]->m_elem_type;
                    servMes.m_file_type = fil->m_list[0]->m_file_type;
                    servMes.m_number_ingredients = fil->m_list[0]->m_number_ingredients;
                    servMes.m_variable_range = range;
                    servMes.m_flag_send_bins = 0;

                    servMes.m_message_size = servMes.byteSize();
                    pts.sendMessage( servMes );
                } // end of change PFI
                else
                {
                    timer_count++;
                    if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                    {
                        PBVR_TIMER_STA( 461 );
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

                    std::cout << "initParam = " << clntMes.m_initialize_parameter << std::endl;
                    if ( clntMes.m_initialize_parameter == 1 )
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
                        std::cout << "endTime = " << clntMes.m_end_time << std::endl;
                        std::cout << "memorySize = " << clntMes.m_memory_size << std::endl;
                    }
                    else if ( clntMes.m_time_parameter == 2 )
                    {
                        std::cout << "step = " << clntMes.m_step << std::endl;
                    }
                    std::cout << "transParam = " << clntMes.m_trans_Parameter << std::endl;
                    if ( clntMes.m_trans_Parameter == 1 )
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
                        servMes.m_flag_send_bins = 0;

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
                        servMes.m_flag_send_bins = 0;

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

                        // if (mpi_size == 1) {
                            transfunc_creator.setProtocol( clntMes );
                            transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                            param.m_transfunc_synthesizer = transfunc_creator.create();
                            param.m_transfunc_synthesizer->optimize2();
                        // }

                        std::vector<std::string> transfer_function_names;
                        std::vector<std::string> transfunc_synthesizer_color_names;
                        std::vector<std::string> transfunc_synthesizer_opacity_names;
                        std::vector<NamedTransferFunctionParameter>::iterator itr;
                        for (itr=clntMes.m_transfer_function.begin();itr!=clntMes.m_transfer_function.end(); itr++) {
                            transfer_function_names.push_back((*itr).m_name);
                        }

                        for ( TransferFunctionMap::const_iterator itr = param.m_transfunc_synthesizer->colorTransferFunctionMap().begin();
                                itr != param.m_transfunc_synthesizer->colorTransferFunctionMap().end();
                                itr++ ) {
                            std::string fn = itr->first;
                            transfunc_synthesizer_color_names.push_back( fn );
                        }

                        for ( TransferFunctionMap::const_iterator itr = param.m_transfunc_synthesizer->opacityTransferFunctionMap().begin();
                                itr != param.m_transfunc_synthesizer->opacityTransferFunctionMap().end();
                                itr++ ) {
                            std::string fn = itr->first;
                            transfunc_synthesizer_opacity_names.push_back( fn );
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
                        if ( param.m_gt5d == true || param.m_gt5d_full == true )
                        {
                            int timeStep = servMes.m_time_step;

                            if ( servMes.m_time_step > 1 )
                            {
                                for ( int nf = 0; nf < point_creator_lst.size(); nf++ )
                                    point_creator_lst[nf]->progressValues();
                            }
                        }

                        if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                        // フィルタファイルリスト情報を設定する：add by @hira at 2016/12/01
                        jd.setFilterInformationList(fil);

                        if ( param.m_crop.isEnabled() )
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, fil->m_total_number_subvolumes,
                                           fil->m_total_min_subvolume_coord,
                                           fil->m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size,
                                           param.m_crop.getMinCoord(),
                                           param.m_crop.getMaxCoord() );
                            servMes.m_number_volume_divide = jd.getCountVolumes();
                        }
                        else
                        {
                            jd.initialize( clntMes.m_step, clntMes.m_step, fil->m_total_number_subvolumes,
                                           fil->m_total_min_subvolume_coord,
                                           fil->m_total_max_subvolume_coord,
                                           param.m_latency_threshold, param.m_job_id_pack_size );
                            servMes.m_number_volume_divide = fil->m_total_number_subvolumes;
                        }

                        if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                        {
                            PBVR_TIMER_STA( 470 );
                        }

                        param.m_sampling_step = CalculateSamplingStep( fil );
                        param.m_subpixel_level = CalculateSubpixelLevel( param, fil, *clntMes.m_camera );

                        VariableRange vr;

                        pts.sendMessage( servMes );

                        // 関数の領域確保、初期化を行う : by @hira 2016/12/01
                        servMes.initializeTransferFunction(clntMes.m_transfer_function.size(), DEFAULT_NBINS);

                        c_bins_size = 0;
                        o_bins_size = 0;
                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            c_bins_size += servMes.m_color_nbins[tf];
                            o_bins_size += servMes.m_opacity_bins_number[tf];
                        }

                        tmp_c_bins = new kvs::UInt64[c_bins_size];
                        tmp_o_bins = new kvs::UInt64[o_bins_size];

                        for ( int tf = 0; tf < c_bins_size; tf++ )
                        {
                            tmp_c_bins[tf] = 0;
                        }

                        for ( int tf = 0; tf < o_bins_size; tf++ )
                        {
                            tmp_o_bins[tf] = 0;
                        }

                        // add by @hira at 2016/12/01
                        // ステップの物理量の読込みを行う。
                        PBVR_TIMER_STA( 500 );
                        fil->loadMpiSubvolume(clntMes.m_step);
                        PBVR_TIMER_END( 500 );

                        while ( jd.dispatchNext( wid, &st, &vl ) )
                        {
                            if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                            {
                                PBVR_TIMER_STA( 471 );
                            }

                            pbvr::PointObject* originalObject = new pbvr::PointObject;

                            if (mpi_size == 1) {
                                int xvl, fidx;
                                fidx = fil->getFileIndex( vl, &xvl );
                                FilterInformationFile *fi = fil->m_list[fidx];

                                pbvr::PointObject* tmp_obj = NULL;
                                std::stringstream suffix;
                                suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                                       << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                                       << '_' << std::setw( 7 ) << std::setfill( '0' ) << fi->m_number_subvolumes;
                                kvs::File ifpx( fil->m_list[fidx]->m_file_path );
                                param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                                   + ifpx.baseName() + suffix.str() + ".kvsml";
                                int timeStep = 1;
                                try
                                {
                                    point_creator_lst[fidx]->setCoordSynthStr( clntMes.m_x_synthesis,
                                                                              clntMes.m_y_synthesis, clntMes.m_z_synthesis );

                                    if (fi->isPbvrFilterInfo()) {
                                        PBVR_TIMER_STA( 99 );
                                        tmp_obj = point_creator_lst[fidx]->run( param, *clntMes.m_camera, timeStep, st, xvl );
                                        PBVR_TIMER_END( 99 );
                                    }
                                    else if ( fi->m_file_type == 1 || fi->m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                                    {
                                        PBVR_TIMER_STA( 99 );
                                        tmp_obj = point_creator_lst[fidx]->run( param, *clntMes.m_camera, timeStep, st, xvl );
                                        PBVR_TIMER_END( 99 );
                                    }
                                    else     // filetype: kvsml
                                    {
                                        PBVR_TIMER_STA( 99 );
                                        tmp_obj = point_creator_lst[fidx]->run( param, *clntMes.m_camera, timeStep, st );
                                        PBVR_TIMER_END( 99 );
                                    }

                                    size_t nmemb = tmp_obj->nvertices() * 3;
                                    kvs::ValueArray<kvs::Real32> coords_array ( tmp_obj->coords().pointer(), nmemb );
                                    kvs::ValueArray<kvs::UInt8>  colors_array ( tmp_obj->colors().pointer(), nmemb );
                                    kvs::ValueArray<kvs::Real32> normals_array( tmp_obj->normals().pointer(), nmemb );

                                    originalObject->clear();
                                    originalObject->setCoords( coords_array );
                                    originalObject->setColors( colors_array );
                                    originalObject->setNormals( normals_array );

                                    // modify by @hira at 2016/12/01
                                    int c_count = 0;
                                    for ( int tf = 0; tf < tmp_obj->getColorHistogram().size(); tf++ )
                                    {
                                        int c_nbins = tmp_obj->getColorHistogram()[ tf ].nbins();
                                        for ( int res = 0; res < c_nbins; res++ )
                                        {
                                            tmp_c_bins[c_count++] += static_cast<kvs::UInt64>( tmp_obj->getColorHistogram()[ tf ][res] );
                                        }
                                    }
                                    int o_count = 0;
                                    for ( int tf = 0; tf < tmp_obj->getOpacityHistogram().size(); tf++ )
                                    {
                                        int o_nbins = tmp_obj->getOpacityHistogram()[ tf ].nbins();
                                        for ( int res = 0; res < o_nbins; res++ )
                                        {
                                            tmp_o_bins[o_count++] += static_cast<kvs::UInt64>( tmp_obj->getOpacityHistogram()[ tf ][res] );
                                        }
                                    }

#if 0			// delete by @hira at 2016/12/011
                                    for ( int tf = 0; tf < tmp_obj->getColorHistogram().size(); tf++ )
                                    {
                                        servMes.m_color_nbins[tf] = tmp_obj->getColorHistogram()[ tf ].nbins();
                                        for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                                        {
                                            servMes.m_color_bins[tf][res] += static_cast<kvs::UInt64>( tmp_obj->getColorHistogram()[ tf ][res] );
                                        }
                                    }

                                    for ( int tf = 0; tf < tmp_obj->getOpacityHistogram().size(); tf++ )
                                    {
                                        servMes.m_opacity_bins_number[tf] = tmp_obj->getOpacityHistogram()[ tf ].nbins();
                                        for ( int res = 0; res < servMes.m_opacity_bins_number[tf]; res++ )
                                        {
                                            servMes.m_opacity_bins[tf][res] += static_cast<kvs::UInt64>( tmp_obj->getOpacityHistogram()[ tf ][res] );
                                        }
                                    }
#endif
                                }
                                catch ( const std::runtime_error& e )
                                {
#ifdef _DEBUG		// debug by @hira
                                    printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                                    std::cerr << e.what();
                                    nan_error = true;
                                }
                                vr = param.m_transfunc_synthesizer->variableRange();
                            }

                            // add by @hira at 2016/12/01
                            // ステップの物理量の読込後処理を行う。
                            fil->releaseMpiSubvolume();

#ifndef CPU_VER
                            if (mpi_size > 1) {
                                jc.jobCollect( originalObject, &vr, &nan_error, &wid );
                            }
#endif
                            int nvertices = originalObject->coords().size() / 3;

                            pbvr::PointObject* object = originalObject;
                            //printf( "  %lu perticles generated\n", object->coords().size() / 3);
							printf("  %zu perticles generated\n", object->coords().size() / 3);

                            if ( originalObject != object ) delete originalObject;
                            servMes.m_number_particle = object->coords().size() / 3;
                            if ( servMes.m_number_particle > 0 )
                            {
                                servMes.m_positions = new float[3 * servMes.m_number_particle];
                                servMes.m_normals = new float[3 * servMes.m_number_particle];
                                servMes.m_colors = new unsigned char[3 * servMes.m_number_particle];
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
                            servMes.m_variable_range = vr;
                            if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                            {
                                PBVR_TIMER_END( 471 );
                            }
                            if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                            {
                                PBVR_TIMER_STA( 472 );
                            }
                            servMes.m_message_size = servMes.byteSize();
                            pts.sendMessage( servMes );
                            if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                            {
                                PBVR_TIMER_END( 472 );
                            }
                            if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                            {
                                PBVR_TIMER_STA( 473 );
                            }
                            delete[] servMes.m_positions;
                            delete[] servMes.m_normals;
                            delete[] servMes.m_colors;
                            delete object;
                            if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                            {
                                PBVR_TIMER_END( 473 );
                            }
                        } // end of while(DispatchNext)
#ifndef CPU_VER

                        if (mpi_size > 1) {
                            MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
                            MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
#if 0			// delete by @hira at 2016/12/011
                            int c_count = 0;
                            int o_count = 0;
                            for ( int tf = 0; tf < servMes.m_transfer_function_count ; tf++ )
                            {
                                for ( int res = 0; res < servMes.m_color_nbins[tf]; res++ )
                                {
                                    servMes.m_color_bins[tf][res] = tmp_c_bins[c_count];
                                    c_count++;
                                }

                                for ( int res = 0; res < servMes.m_opacity_bins_number[tf]; res++ )
                                {
                                    servMes.m_opacity_bins[tf][res] = tmp_o_bins[o_count];
                                    o_count++;
                                }
                            }
#endif
                        }
#endif

                        servMes.setColorHistogramBins(
                                transfunc_synthesizer_color_names.size(),
                                DEFAULT_NBINS,
                                tmp_c_bins,
                                transfer_function_names,
                                transfunc_synthesizer_color_names);

                        servMes.setOpacityHistogramBins(
                                transfunc_synthesizer_opacity_names.size(),
                                DEFAULT_NBINS,
                                tmp_o_bins,
                                transfer_function_names,
                                transfunc_synthesizer_opacity_names);

                        // TEST START 2015.1.14
                        if ( nan_error )
                        {
                            strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
                            servMes.m_server_status = 1;
                            servMes.m_number_particle = 0;
                            servMes.m_flag_send_bins = 0;
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
                        delete[] servMes.m_opacity_bins_number;
                        servMes.m_transfer_function_count = 0;
                        servMes.m_flag_send_bins = 0;
                        delete[] tmp_c_bins;
                        delete[] tmp_o_bins;

                        delete param.m_transfunc_synthesizer;

                        if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                        {
                            PBVR_TIMER_END( 470 );
                        }
                    } // end of timeParam == 2
                    else
                    {
                        break;
                    }
                    if ( timer_count <= PBVR_TIMER_COUNT_NUM )
                    {
                        PBVR_TIMER_END( 461 );
                    }
                    if ( timer_count == PBVR_TIMER_COUNT_NUM )
                    {
                        PBVR_TIMER_END( 1 );
                        PBVR_TIMER_FIN();
                    }
                } // end of initParam >= 0

                // debug by @hira at 2016/12/01
                double server_end = GetTime();
                printf("SERVER-LOOP[timer_count=%d] :: time = %lf(ms)\n",
                        timer_count, server_end - server_start);
            } // end of while (pts.good)

            delete clntMes.m_camera;

            bsz = -1;
#ifndef CPU_VER
            MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

            pts.termServer();
        }		// rank == 0
    }		// client-server mode
#ifdef KMATH
    km_random.finalize();
#endif
    if ( param.m_batch == true )
    {
        PBVR_TIMER_END( 1 );
        PBVR_TIMER_FIN();
    }
#ifndef CPU_VER
    MPI_Finalize();
#endif

    for (std::vector<PointObjectCreator*>::iterator itr=point_creator_lst.begin();itr!=point_creator_lst.end();itr++) {
        if (*itr == NULL) continue;
        (*itr)->releaseObject();
        delete (*itr);
    }
    point_creator_lst.clear();

    if (fil != NULL) delete fil;
    if (server != NULL) delete server;

    return retval;
}

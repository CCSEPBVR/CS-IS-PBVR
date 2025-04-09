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
#include "JobCollector.h"
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

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;

bool useAllNodes = true;

inline const size_t GetRevisedSubpixelLevel(
    const size_t subPixelLevel,
    const size_t repetition_level )
{
    return static_cast<size_t>( subPixelLevel * std::sqrt( ( double )repetition_level ) + 0.5f );
}

class PointObjectCreator
{
private:

    vismodule::UnstructuredVolumeObject* m_volume;

    vismodule::PointObjectGenerator m_generator;

    int m_mpi_rank;

    const MultiVolumeProperty* m_mvp;

    std::string m_xcSynthStr;
    std::string m_ycSynthStr;
    std::string m_zcSynthStr;

public:

    PointObjectCreator()
        : m_volume(NULL), m_mpi_rank(0), m_mvp(NULL) {}

    ~PointObjectCreator()
    {
    }

    void setFilterInfo( const MultiVolumeProperty& mvp )
    {
        m_mvp = &mvp;
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

        const vismodule::Vector3ui resol = m_field->dim();
        const vismodule::UInt32 nnodes = resol.x() * resol.y() * resol.z();
        const vismodule::Vector3ui elem = resol - vismodule::Vector3ui( 1 );
        const vismodule::UInt32 nelem = elem.x() * elem.y() * elem.z();

        vismodule::ValueArray<vismodule::Real32> coords( nnodes * 3 );
        vismodule::Matrix33f mat( vismodule::YRotationMatrix33<float>( ( float )rank * 60.0 ) );
        size_t i3 = 0;
        for ( size_t i = 0; i < nnodes; i++, i3 += 3 )
        {
            vismodule::Vector3f vec;
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
        m_volume->setValues( vismodule::AnyValueArray( m_field->values( 0 ) ) );
        m_volume->updateMinMaxValues();

#endif
    }

public:

    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int timeStep,  const int st = 1 )
    {
        m_generator.setFinlterInfo( m_mvp );
        m_generator.setCoordSynthTS( st );

        struct stat s;
        if ( stat( param.m_input_data.c_str(), &s ) )
        {
            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
            exit( 1 );
        }
        m_generator.createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step );

        vismodule::PointObject* po = m_generator.getPointObject();
        return po;
    }

    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int timeStep, const int st, const int vl)
    {
        m_generator.setFinlterInfo( m_mvp );
        m_generator.setCoordSynthTS( st );
        m_generator.createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step, st, vl );
        vismodule::PointObject* po = m_generator.getPointObject();
        return po;
    }

    void setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss )
    {
        m_xcSynthStr = xss;
        m_ycSynthStr = yss;
        m_zcSynthStr = zss;

        vismodule::CoordSynthesizerStrings css( 0, xss, yss, zss );
        m_generator.setCoordSynthStrs( css );
    }

    //2023 shimomura
    //void setCoordSynthTkn( const EquationToken & xst, const EquationToken & yst, const EquationToken & zst )
    void setCoordSynthTkn( const jpv::ParticleTransferClientMessage::EquationToken & xst,
                           const jpv::ParticleTransferClientMessage::EquationToken & yst,
                           const jpv::ParticleTransferClientMessage::EquationToken & zst )
    {
//        m_xcSynthStr = xss;
//        m_ycSynthStr = yss;
//        m_zcSynthStr = zss;

        vismodule::EquationToken xst_tmp;
        vismodule::EquationToken yst_tmp;
        vismodule::EquationToken zst_tmp;

        for(int i=0; i<128; i++ )
        {
            xst_tmp.exp_token[i] = xst.exp_token[i];
            xst_tmp.var_name[i]  = xst.var_name[i] ;
            xst_tmp.val_array[i] = xst.value_array[i];
            yst_tmp.exp_token[i] = yst.exp_token[i];
            yst_tmp.var_name[i]  = yst.var_name[i] ;
            yst_tmp.val_array[i] = yst.value_array[i];
            zst_tmp.exp_token[i] = zst.exp_token[i];
            zst_tmp.var_name[i]  = zst.var_name[i] ;
            zst_tmp.val_array[i] = zst.value_array[i];
        }

        vismodule::CoordSynthesizerTokens cst(xst_tmp, yst_tmp, zst_tmp );
        m_generator.setCoordSynthTkns( cst );
    }

protected:

    bool apply_coordinate_synthesizer( vismodule::PointObject* po, const int timeStep )
    {
        if ( ! po ) return false;
        if ( m_xcSynthStr.empty() && m_ycSynthStr.empty() && m_zcSynthStr.empty() )
            return true;
        size_t i, j, nv = po->nvertices();
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
        const vismodule::ValueArray<vismodule::Real32>& org_coords = po->coords();
        vismodule::ValueArray<vismodule::Real32> new_coords;
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

        m_field = new vismodule::AVSField( header, coord, variable );
        m_field->progress();

        m_volume = new vismodule::UnstructuredVolumeObject;

        const vismodule::Vector3ui resol = m_field->dim();
        const vismodule::UInt32 nnodes = resol.x() * resol.y() * resol.z();
        const vismodule::Vector3ui elem = resol - vismodule::Vector3ui( 1 );
        const vismodule::UInt32 nelem = elem.x() * elem.y() * elem.z();

        m_volume->setVeclen( 1 );
        m_volume->setNNodes( nnodes );
        m_volume->setNCells( nelem );
        m_volume->setCellType( vismodule::UnstructuredVolumeObject::Hexahedra );
        m_volume->setValues( vismodule::AnyValueArray( m_field->values( 0 ) ) );

        const vismodule::UInt32 line_size = resol.x();
        const vismodule::UInt32 slice_size = line_size * resol.y();
        vismodule::ValueArray<vismodule::UInt32> connections( nelem * 8 );

        vismodule::UInt32 m_index[8];
        vismodule::UInt32* connec = connections.pointer();
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

//inline vismodule::UnstructuredVolumeObject* CreateVolumeData( const Argument& param,
inline vismodule::VolumeObjectBase* CreateVolumeData( const Argument& param,
                                                         const MultiVolumeProperty& mvp,
                                                         const int& steps, const int& subvols )
{
    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 )
    {
        vismodule::File ifpx( mvp.m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
        //vismodule::UnstructuredVolumeObject* volume = new vismodule::UnstructuredVolumeImporter( path_base,
        vismodule::VolumeObjectBase* volume = new vismodule::UnstructuredVolumeImporter( path_base, mvp.m_file_type, steps, subvols );
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );

        return volume;
    }
#ifdef EXTEND_FILE_FORMAT 
    else if ( mvp.m_file_type == 3 )
    {
        std::string path_base = mvp.m_file_path;
        int file_type = mvp.m_file_type;
        int cell_type = mvp.m_elem_type;
        vismodule::VolumeObjectBase* volume = new vismodule::UnstructuredVolumeImporter( path_base, file_type, cell_type, steps, subvols );
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        return volume;
    }
#endif
    else
    {
        std::stringstream suffix;
        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( steps )
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( subvols + 1 )
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;

        //std::string m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
        vismodule::File ifpx( mvp.m_file_path );
        std::string m_input_data = ifpx.pathName() + ifpx.Separator()
                                   + ifpx.baseName() + suffix.str() + ".kvsml";
        //vismodule::UnstructuredVolumeObject* volume = new vismodule::UnstructuredVolumeImporter( m_input_data );

        vismodule::VolumeObjectBase* volume = nullptr;

        if      ( vismoduleview::FileChecker::ImportableStructuredVolume( m_input_data ))
        {
            std::cout << "Structured !" <<std::endl;
            volume = new vismodule::StructuredVolumeImporter( m_input_data ); 
        } 
        else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( m_input_data))
        {
            std::cout << "Unstructured !" <<std::endl;
            volume = new vismodule::UnstructuredVolumeImporter( m_input_data );  
        }
        else 
        {
            visModuleMessageError("%s is not volume data.", m_input_data.c_str());
            //return false;
        }

            volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
            volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
            volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        return volume;
    }
}


inline float CalculateSamplingStep( const MultiVolumePropertyList& mvpl )
{
    float max_coord_length = vismodule::Math::Max<float>( mvpl.m_total_max_object_coord.x() - mvpl.m_total_min_object_coord.x(),
                                                    mvpl.m_total_max_object_coord.y() - mvpl.m_total_min_object_coord.y(),
                                                    mvpl.m_total_max_object_coord.z() - mvpl.m_total_min_object_coord.z() );
    return 0.1 * max_coord_length;
}

//kawamura2: This calculates optimized subpixel level.
inline size_t CalculateSubpixelLevel( const Argument& param,
                                      const MultiVolumePropertyList& mvpl,
                                      const vismodule::Camera& camera )
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    //vismodule::UnstructuredVolumeObject* volume;
    vismodule::VolumeObjectBase* volume;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    int steps = mvpl.m_total_start_steps;
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

    for ( subvols = 0; subvols < mvpl.m_total_number_subvolumes; subvols++ )
    {
        int xvl, fidx;
        fidx = mvpl.getFileIndex( subvols, &xvl );
        const MultiVolumeProperty& mvp = mvpl.m_list[fidx];

        if ( subvols % nprocs == rank )
        {
            VIS_MODULE_TIMER_STA( 16 );
            volume = CreateVolumeData( param, mvp, steps, xvl );
            VIS_MODULE_TIMER_END( 16 );

            VIS_MODULE_TIMER_STA( 17 );
            double local_volume = Generator::CalculateTotalVolume( *volume );
            VIS_MODULE_TIMER_END( 17 );

            VIS_MODULE_TIMER_STA( 18 );
            density_lev1 += Generator::CalculateGreatDensity( camera, *volume, 1,
                                                              param.m_sampling_step ) * local_volume;
            VIS_MODULE_TIMER_END( 18 );

            total_volume += local_volume;
            delete volume;
        }
    }

#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, &density_lev1, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, &total_volume, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
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

inline VariableRange Calculate_minmax( const Argument& param,
                                      const MultiVolumePropertyList& mvpl)
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    //vismodule::UnstructuredVolumeObject* volume;
    vismodule::VolumeObjectBase* volume;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    int steps = mvpl.m_total_start_steps;
    int subvols = 0;

    vismodule::Real64 tmp_min, tmp_max;
    std::vector<vismodule::Real64> min_vec, max_vec;
    int nvariable = mvpl.m_total_number_ingredients;
    min_vec.resize(nvariable);
    max_vec.resize(nvariable);
    for(int i = 0 ;i < nvariable ; i++)
    {
        min_vec[i] = FLT_MAX; 
        max_vec[i] = FLT_MIN; 
    } 
# if 0
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

    for ( steps = mvpl.m_total_start_steps; steps <= mvpl.m_total_last_step; steps++ )
    {
        for ( subvols = 0; subvols < mvpl.m_total_number_subvolumes; subvols++ )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( subvols, &xvl );
            const MultiVolumeProperty& mvp = mvpl.m_list[fidx];

            if ( subvols % nprocs == rank )
            {
                volume = CreateVolumeData( param, mvp, steps, xvl );
                //volume->updateMinMaxValues();
                int nnodes = volume->nnodes();
                for (int n =0; n< nvariable; n++) 
                {
                    tmp_min = volume->values().at<float>(0+n*nnodes); 
                    tmp_max = volume->values().at<float>(0+n*nnodes); 
                    for (int i = 1; i< nnodes; i++)
                    {
                        tmp_min = tmp_min < volume->values().at<float>(i+n*nnodes) ? tmp_min : volume->values().at<float>(i+n*nnodes) ; 
                        tmp_max = tmp_max > volume->values().at<float>(i+n*nnodes) ? tmp_max : volume->values().at<float>(i+n*nnodes) ; 
                    }
                    min_vec[n]=min_vec[n] < tmp_min ? min_vec[n] : tmp_min;
                    max_vec[n]=max_vec[n] > tmp_max ? max_vec[n] : tmp_max;
                }
                delete volume;
            }
        }
    }

#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, min_vec.data(), nvariable, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, max_vec.data(), nvariable, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif

   VariableRange vr;
   for (int n =0; n< nvariable; n++) 
   {
        std::stringstream ss; 
        ss << (n + 1); 
        const std::string idxbuf = ss.str();
        vr.setValue( "t" + idxbuf + "_var_o", max_vec[n]);
        vr.setValue( "t" + idxbuf + "_var_o", min_vec[n]);
        vr.setValue( "t" + idxbuf + "_var_c", max_vec[n]);
        vr.setValue( "t" + idxbuf + "_var_c", min_vec[n]);
   }
#endif

   VariableRange vr;
   for (int n =0; n< nvariable; n++) 
   {
        std::stringstream ss; 
        ss << (n + 1); 
        const std::string idxbuf = ss.str();
        vr.setValue( "t" + idxbuf + "_var_o", 1);
        vr.setValue( "t" + idxbuf + "_var_o", 0);
        vr.setValue( "t" + idxbuf + "_var_c", 1);
        vr.setValue( "t" + idxbuf + "_var_c", 0);
   }

//   std::cout << "vr_max = " << vr.max( "t1_var_c" ) << std::endl;     
//   std::cout << "vr_min = " << vr.min( "t1_var_c" ) << std::endl;     

   return vr;
}


void Calculate_minmax_glyph( const Argument& param,
                                      const MultiVolumePropertyList& mvpl,
                                      jpv::ParticleTransferClientMessage& clntMes)
{


    namespace Generator = vismodule::CellByCellParticleGenerator;
    vismodule::VolumeObjectBase* volume = nullptr;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    //int steps = mvpl.m_total_start_steps;
    int steps = clntMes.m_step;
    int subvols = 0;

    vismodule::Real64 tmp_min, tmp_max;
    std::vector<float> min_vec, max_vec;
    int nvariable = mvpl.m_total_number_ingredients;
    int nvariablep2 = 2;
    min_vec.resize(nvariablep2);
    max_vec.resize(nvariablep2);
    for(int i = 0 ;i < nvariablep2 ; i++)
    {
        min_vec[i] = FLT_MAX; 
        max_vec[i] = FLT_MIN; 
    } 
# if 1
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

    // color
    min_vec[0] = 0;
    max_vec[0] = 0;
    std::vector<int> color_data_variables;
    if( clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable  )
    {
        for (int i =0 ; i< clntMes.m_color_data_variable.size(); i++)
        {
            color_data_variables.push_back( std::atoi(clntMes.m_color_data_variable[i].substr(1).c_str()) - 1); 
        }

    }
  // size
    min_vec[1] = 0;
    max_vec[1] = 0;
    std::vector<int> size_variables;
    if( clntMes.m_size_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_size_sampling_method == jpv::DataDefines::SingleVariable  )
    {
        for (int i =0 ; i< clntMes.m_size_variable.size(); i++)
        {
            size_variables.push_back( std::atoi(clntMes.m_size_variable[i].substr(1).c_str()) - 1); 
        }

    }

      if( clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable 
          || clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable  )
      {
//    for ( steps = mvpl.m_total_start_steps; steps <= mvpl.m_total_start_step; steps++ ) //初回ステップのみ
//    {
        for ( subvols = 0; subvols < mvpl.m_total_number_subvolumes; subvols++ )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( subvols, &xvl );
            const MultiVolumeProperty& fi = mvpl.m_list[fidx];

            if ( subvols % nprocs == rank )
            {
                volume = CreateVolumeData( param, fi, steps, xvl );
                int nnodes = volume->nnodes();
                // color
                if( clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable  )
                {
                    tmp_min = FLT_MAX;
                    tmp_max = FLT_MIN;
                    for (int i = 0; i< nnodes; i++)
                    {
                        float tmp = 0;
                        for(int k = 0 ; k< clntMes.m_color_data_variable.size() ; k++)
                        {
                            tmp += vismodule::Math::Square(volume->values().at<float>( i+ color_data_variables[k]*nnodes)) ;
                        }

                        tmp = std::sqrt(tmp);
                        tmp_min = tmp_min < tmp ? tmp_min : tmp ; 
                        tmp_max = tmp_max > tmp ? tmp_max : tmp ; 
                    }
                    min_vec[0]=min_vec[0] < tmp_min ? min_vec[0] : tmp_min;
                    max_vec[0]=max_vec[0] > tmp_max ? max_vec[0] : tmp_max;
            }
            // size
            if( clntMes.m_size_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_size_sampling_method == jpv::DataDefines::SingleVariable  )
            {
                tmp_min = FLT_MAX;
                tmp_max = FLT_MIN;
                for (int i = 0; i< nnodes; i++)
                {
                    float tmp = 0;
                    for(int k = 0 ; k< clntMes.m_size_variable.size() ; k++)
                    {
                        tmp += vismodule::Math::Square(volume->values().at<float>( i+ size_variables[k]*nnodes)) ;
                    }
                    tmp = std::sqrt(tmp);
                    tmp_min = tmp_min < tmp ? tmp_min : tmp ; 
                    tmp_max = tmp_max > tmp ? tmp_max : tmp ; 
                }
                min_vec[1]=min_vec[1] < tmp_min ? min_vec[1] : tmp_min;
                max_vec[1]=max_vec[1] > tmp_max ? max_vec[1] : tmp_max;

            }
            delete volume;
            }
        }
//    }
#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, min_vec.data(), nvariablep2, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, max_vec.data(), nvariablep2, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif
      }
      if( clntMes.m_color_data_sampling_method == jpv::DataDefines::Constant )
      {
                min_vec[0]=0;
                max_vec[0]=1;
      }
      if( clntMes.m_size_sampling_method == jpv::DataDefines::Constant )
      {
                min_vec[1]=0;
                max_vec[1]=1;
      }

  clntMes.m_glyph_color_max = max_vec[0] ;
  clntMes.m_glyph_color_min = min_vec[0] ;
  clntMes.m_glyph_size_max  = max_vec[1] ;
  clntMes.m_glyph_size_min  = min_vec[1] ;
#endif
}


inline float CalculateDensityFactor( const Argument& param,
                                     const MultiVolumeProperty& mvp,
                                     const vismodule::Camera& camera )
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    //vismodule::UnstructuredVolumeObject* volume;
    vismodule::VolumeObjectBase* volume;
    double total_volume = 0.0;
    float great_density;
    int steps = mvp.m_start_step;
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
        VIS_MODULE_TIMER_STA( 16 );
        volume = CreateVolumeData( param, mvp, steps, subvols );
        VIS_MODULE_TIMER_END( 16 );
        VIS_MODULE_TIMER_STA( 17 );
        total_volume += Generator::CalculateTotalVolume( *volume );
        VIS_MODULE_TIMER_END( 17 );
        VIS_MODULE_TIMER_STA( 18 );
        great_density = Generator::CalculateGreatDensity( camera, *volume, param.m_subpixel_level,
                                                          param.m_sampling_step );
        VIS_MODULE_TIMER_END( 18 );

        delete volume;
    }
#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Bcast( &great_density, 1, MPI_FLOAT, 0, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif


    for ( subvols = subvols + 1; subvols < mvp.m_number_subvolumes; subvols++ )
    {
        if ( subvols % nprocs == rank )
        {
            VIS_MODULE_TIMER_STA( 16 );
            volume = CreateVolumeData( param, mvp, steps, subvols );
            VIS_MODULE_TIMER_END( 16 );
            VIS_MODULE_TIMER_STA( 17 );
            total_volume += Generator::CalculateTotalVolume( *volume );
            VIS_MODULE_TIMER_END( 17 );

            delete volume;
        }
    }
#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, &total_volume, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif

    float total_nparticles = great_density * static_cast<float>( total_volume );
    float m_density_factor = static_cast<float>( param.m_particle_limit ) * 1000000 / total_nparticles;

    if ( m_density_factor > 1.0 ) m_density_factor = 1.0;

    return m_density_factor;
}

VariableRange  setVariablerange2(const float* tmp_max, const float* tmp_min, const int cnt )
{
    VariableRange vr;
    
    for ( int tf = 0; tf < cnt; tf++ )
    {
        std::stringstream ss; 
        ss << (tf + 1); 
        const std::string idxbuf = ss.str();
        vr.setValue( "t" + idxbuf + "_var_o", tmp_max[2*tf  ]);
        vr.setValue( "t" + idxbuf + "_var_o", tmp_min[2*tf  ]);
        vr.setValue( "t" + idxbuf + "_var_c", tmp_max[2*tf+1]);
        vr.setValue( "t" + idxbuf + "_var_c", tmp_min[2*tf+1]);
    }   
    return vr;
}

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
    int rank = 0;
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
                    timer_count++;
//                  param.m_transfer_function = vismodule::TransferFunction(); // *( clntMes.m_transfer_function );
                    param.m_sampling_method = 'h';
                    param.m_component_Id = clntMes.m_rendering_id;
                    clntMes.m_enable_crop_region = 0;
                    param.m_crop.setEnable( clntMes.m_enable_crop_region );
                    param.m_crop.set( clntMes.m_crop_region );

                    param.m_input_data_base = clntMes.m_input_directory;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;

                    std::string pfifile, pflfile;
                    if ( param.m_input_data_base.substr( param.m_input_data_base.size() - 3 ) == "pfl" )
                    {
                        pflfile = param.m_input_data_base;
                        param.m_input_data_base = pflfile.substr( 0, pflfile.size() - 4 );
                        vismodule::File pfl( pflfile );
                        if ( pfl.isExisted() )
                        {
                            mvpl.loadPFL( pflfile );
                        }
                    }
                    else
                    {
#if 0
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
						pflfile = param.m_input_data_base;
						vismodule::File pfl( pflfile );
						if ( pfl.isExisted() )
						{
							mvpl.loadPFL( pflfile );
						}
#endif
                    }

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
                    int nvariable;
                    VariableRange range = Calculate_minmax( param, mvpl); 
                    if( !clntMes.m_import_flag ) 
                    {
                        std::cout << "defalt parameter " << std::endl;
                        nvariable = mvpl.m_total_number_ingredients;
                        transfunc_creator.setInitialProtocol( nvariable, range );
                    }
                    else
                    {
                        std::cout << "user define parameter " << std::endl;
                        nvariable = clntMes.m_transfer_function.size();
                        transfunc_creator.setProtocol(clntMes);
                    }
                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());

                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }
                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                    if ( param.m_crop.isEnabled() )
                    {
                        jd.initialize( mvpl.m_total_start_steps, mvpl.m_total_start_steps, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size,
                                       param.m_crop.getMinCoord(),
                                       param.m_crop.getMaxCoord() );
                    }
                    else
                    {
                        jd.initialize( mvpl.m_total_start_steps, mvpl.m_total_start_steps, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size );
                    }

                    param.m_sampling_step = CalculateSamplingStep( mvpl );
                    //param.m_sampling_step = 1;
                    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );
                    param.m_particle_limit_pre = param.m_particle_limit;

                    clntMes.show();
                    int tf_count = nvariable;
                    int c_nbins = DEFAULT_NBINS;
                    int o_nbins = DEFAULT_NBINS;

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
                        tmp_max[tf] = 0;
                        tmp_min[tf] = 0;
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

                        std::stringstream suffix;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                        //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                        vismodule::File ifpx( mvp.m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                             + ifpx.baseName() + suffix.str() + ".kvsml";
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                object = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl );
                            }
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
                        VariableRange* p_vr = &range;
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
                            tmp_max[2*tf] = vismodule::Math::Max( tmp_max[2*tf] ,param.m_transfunc_synthesizer-> m_c_max[tf]);
                            tmp_min[2*tf] = vismodule::Math::Min( tmp_min[2*tf] ,param.m_transfunc_synthesizer-> m_c_min[tf]);
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
               else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
               {


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

                    std::string pfifile, pflfile;
                    if ( param.m_input_data_base.substr( param.m_input_data_base.size() - 3 ) == "pfl" )
                    {
                        pflfile = param.m_input_data_base;
                        param.m_input_data_base = pflfile.substr( 0, pflfile.size() - 4 );
                        vismodule::File pfl( pflfile );
                        if ( pfl.isExisted() )
                        {
                            mvpl.loadPFL( pflfile );
                        }
                    }
                    else
                    {
#if 0
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
						pflfile = param.m_input_data_base;
						vismodule::File pfl( pflfile );
						if ( pfl.isExisted() )
						{
							mvpl.loadPFL( pflfile );
						}
#endif
                    }

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

                        std::stringstream suffix;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                        //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                        vismodule::File ifpx( mvp.m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                             + ifpx.baseName() + suffix.str() + ".kvsml";
                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                object = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl );

                            }
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

                    std::string pfifile, pflfile;
                    if ( param.m_input_data_base.substr( param.m_input_data_base.size() - 3 ) == "pfl" )
                    {
                        pflfile = param.m_input_data_base;
                        param.m_input_data_base = pflfile.substr( 0, pflfile.size() - 4 );
                        vismodule::File pfl( pflfile );
                        if ( pfl.isExisted() )
                        {
                            mvpl.loadPFL( pflfile );
                        }
                    }
                    else
                    {
#if 0
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
						pflfile = param.m_input_data_base;
						vismodule::File pfl( pflfile );
						if ( pfl.isExisted() )
						{
							mvpl.loadPFL( pflfile );
						}
#endif
                    }

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

                        vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;
                        std::stringstream suffix;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                        //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                        vismodule::File ifpx( mvp.m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                             + ifpx.baseName() + suffix.str() + ".kvsml";
                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 

                            }
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

                    std::string pfifile, pflfile;
                    if ( param.m_input_data_base.substr( param.m_input_data_base.size() - 3 ) == "pfl" )
                    {
                        pflfile = param.m_input_data_base;
                        param.m_input_data_base = pflfile.substr( 0, pflfile.size() - 4 );
                        vismodule::File pfl( pflfile );
                        if ( pfl.isExisted() )
                        {
                            mvpl.loadPFL( pflfile );
                        }
                    }
                    else
                    {
						pflfile = param.m_input_data_base;
						vismodule::File pfl( pflfile );
						if ( pfl.isExisted() )
						{
							mvpl.loadPFL( pflfile );
						}
                    }

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

                        //vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;
                        vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
                        std::stringstream suffix;
                        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                               << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                        //param.m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
                        vismodule::File ifpx( mvp.m_file_path );
                        param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                             + ifpx.baseName() + suffix.str() + ".kvsml";
                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
//                                //object = glyph_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl );
//                                *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 
                            }
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
                    std::string target = "¥";
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
                    param.m_input_data_base = clntMes.m_input_directory;

                    bool open_flag = true; 
                    std::ifstream fin( param.m_input_data_base, std::ios::in);
                    /*
                    if (!fin.is_open()) 
                    {
                        std::cout << "ファイルを開けませんでした: " << param.m_input_data_base << std::endl;
                        open_flag = false;
                    }
                    */
      
                    bool ExtendFileFormat_flag = true;
                    bool pfi_flag = true;
#ifndef EXTEND_FILE_FORMAT 
                    ExtendFileFormat_flag = false;
                    
                    {          
                        pfi_flag = false;
                        size_t found_pfl = param.m_input_data_base.find(".pfl");
                        size_t found_pfi = param.m_input_data_base.find(".pfi");
                        if (found_pfl != std::string::npos) pfi_flag = true;
                        if (found_pfi != std::string::npos) pfi_flag = true;
                    }
#endif
                    strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                    // ADD by FEAST 2015.12.24
                    //servMes.m_server_status = 0;
                    // ADD END 2015.12.24
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_flag_send_bins = 1;
                    servMes.m_message_size = servMes.byteSize();
                    servMes.m_transfer_function_count = 0;
                    std::cout << "open_flag = " << open_flag << ", ExtendFileFormat_flag = " << ExtendFileFormat_flag << ", pfi_flag = " << pfi_flag << std::endl; 
                    if (open_flag == true && pfi_flag == true) servMes.m_file_enable_flag = jpv::FileEnableFlag::Enable_VTK ;
                    if (open_flag == true && ExtendFileFormat_flag == false && pfi_flag == false) servMes.m_file_enable_flag = jpv::FileEnableFlag::NotEnable_VTK;
                    if (open_flag == false) servMes.m_file_enable_flag = jpv::FileEnableFlag::NoFile ;
                    pts.sendMessage( servMes );
 
                    if(servMes.m_file_enable_flag == jpv::FileEnableFlag::NotEnable_VTK || servMes.m_file_enable_flag == jpv::FileEnableFlag::NoFile) 
                    //if( warning_flag == 2) 
                    {
                        if ( rank == 0 ) std::cerr << "Error: pfifile doesn't exist" << std::endl;
                        bsz = -1;
#ifndef CPU_VER
                        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif
 
//#ifndef CPU_VER               // 開けなくても停止しないよう変更  予定 
//                        MPI_Finalize();
//#endif
//                        return 0;
                     
                        continue;
                    }
                   

#if 0
                    std::string pfifile = param.m_input_data_base + ".pfi";
                    vismodule::File pfi( pfifile );
                    std::string pflfile = param.m_input_data_base + ".pfl";
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
                    // MultiVolumePropertyクラスに情報を格納する
                    // .pflファイル,.pfiファイルはこれまで通り
                    // 変換前ファイルの場合にも対応できるように拡張する
                    size_t found_pfl = param.m_input_data_base.find(".pfl");
                    size_t found_pfi = param.m_input_data_base.find(".pfi");
                    size_t found_vtm = param.m_input_data_base.find(".vtm");
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
			            mvpl.loadVtm( vtmfile );
                    }		    
#endif    
                    else
                    {
                        std::string pre_conversion_file_path = param.m_input_data_base;
                        std::cout << "このファイルは現在対応していません" << std::endl;
                    }
#endif
                    if ( mvpl.m_list.size() > 0 )
                    {
                        point_creator_lst.clear();
                        for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                        {
                            PointObjectCreator point_creator;
                            if ( param.m_gt5d == true ) point_creator.setGT5D();
                            point_creator.setFilterInfo( mvpl.m_list[idx] );
                            point_creator.setCoordSynthStr( clntMes.m_x_synthesis,
                                                            clntMes.m_y_synthesis, clntMes.m_z_synthesis );
//                            point_creator.setCoordSynthTkn( clntMes.x_synthesis_token,
//                                                            clntMes.y_synthesis_token, clntMes.z_synthesis_token );
                            point_creator_lst.push_back( point_creator );
                        }

                        transfunc_creator.setFilterInfo( mvpl.m_list[0] );

                        std::cout << " time step = "          << mvpl.m_total_number_steps
                                  << " subvolume division = " << mvpl.m_total_number_subvolumes
                                  << std::endl;

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

                    }
                    else
                    {
                        if ( rank == 0 ) std::cerr << "Error: pfifile doesn't exist" << std::endl;
                        bsz = -1;
#ifndef CPU_VER
                        MPI_Bcast( &bsz, 1, MPI_INT, 0, MPI_COMM_WORLD ); // termination message
#endif

                        
#ifndef CPU_VER               // 開けなくても停止しないよう変更  予定 
                        MPI_Finalize();
#endif
                        return 0;
                        strncpy( servMes.m_header, "JPTP /1.0 999 OK\r\n", 18 );
                        // ADD by FEAST 2015.12.24
                        //servMes.m_server_status = 0;
                        // ADD END 2015.12.24
                        servMes.m_number_particle = 0;
                        servMes.m_number_glyph = 0 ;
                        servMes.m_flag_send_bins = 1;
                        servMes.m_message_size = servMes.byteSize();
                        pts.sendMessage( servMes );
                       
                        continue;
                    }

                    //transfunc_creator.setProtocol( clntMes );
                    //int nvariable = mvpl.m_total_number_ingredients;
                    int nvariable;
                    VariableRange range = Calculate_minmax( param, mvpl);
                    if( !clntMes.m_import_flag ) 
                    {
                        std::cout << "defalt parameter " << std::endl;
                        nvariable = mvpl.m_total_number_ingredients;
                        transfunc_creator.setInitialProtocol( nvariable, range );
                    }
                    else
                    {
                        std::cout << "user define parameter " << std::endl;
                        nvariable = clntMes.m_transfer_function.size();
                        transfunc_creator.setProtocol(clntMes);
                        servMes.m_glyph_color_min  =  clntMes.m_transfer_function[0].m_color_variable_min;
                        servMes.m_glyph_color_max  =  clntMes.m_transfer_function[0].m_color_variable_max; 
                        servMes.m_glyph_size_min   =  clntMes.m_transfer_function[0].m_color_variable_min;
                        servMes.m_glyph_size_max   =  clntMes.m_transfer_function[0].m_color_variable_max; 
                    }
#if 1
                    // generate_histogram
                    param.m_sampling_method = 'h';
                    param.m_component_Id = clntMes.m_rendering_id;
                    clntMes.m_enable_crop_region = 0;
                    param.m_crop.setEnable( clntMes.m_enable_crop_region );
                    param.m_crop.set( clntMes.m_crop_region );
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;

                    param.m_transfunc_synthesizer = transfunc_creator.create();
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }
                    
                    // 4 calc histgram
                    clntMes.m_node_type = 'a';  
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
                                point_creator_lst[nf].progressValues();
                        }
                    }

                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;

                    if ( param.m_crop.isEnabled() )
                    {
                        jd.initialize( mvpl.m_total_start_steps, mvpl.m_total_start_steps, mvpl.m_total_number_subvolumes,
                                mvpl.m_total_min_subvolume_coord,
                                mvpl.m_total_max_subvolume_coord,
                                param.m_latency_threshold, param.m_job_id_pack_size,
                                param.m_crop.getMinCoord(),
                                param.m_crop.getMaxCoord() );
                        servMes.m_number_volume_divide = jd.getCountVolumes();
                    }
                    else
                    {
                        jd.initialize( mvpl.m_total_start_steps, mvpl.m_total_start_steps, mvpl.m_total_number_subvolumes,
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
//                    pts.sendMessage( servMes );

                    // 関数の領域確保、初期化を行う : by @hira 2016/12/01
                    servMes.initializeTransferFunction(nvariable, DEFAULT_NBINS);

                    //int tf_count = clntMes.m_transfer_function.size();
                    int tf_count = nvariable;
                    c_bins_size = 0;
                    o_bins_size = 0;
                    for ( int tf = 0; tf < tf_count; tf++ )
                    {
                        c_bins_size += servMes.m_color_nbins[tf];
                        o_bins_size += servMes.m_opacity_nbins[tf];
                    }

                    tmp_c_bins = new vismodule::UInt64[c_bins_size];
                    tmp_o_bins = new vismodule::UInt64[o_bins_size];

                    //add by shimomura 2023/06/14
                    int cnt = 2* servMes.m_transfer_function_count ;
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

                        if (mpi_size == 1) 
                        {
                            int xvl, fidx;
                            fidx = mvpl.getFileIndex( vl, &xvl );
                            MultiVolumeProperty& mvp = mvpl.m_list[fidx];

                            size_t found_pfi = mvp.m_file_path.find(".pfi");
                            size_t found_vtm = mvp.m_file_path.find(".vtm");
                            
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
                            else if ( found_vtm != std::string::npos )
                            {
                                param.m_input_data = mvp.m_file_path;
                            }
#endif
                            else
                            {
                                std::cout << "このファイルは現在対応していません" << std::endl;
                            }

                            vismodule::PointObject* tmp_obj = NULL;
                            param.m_subvolume_id = xvl;
                            int timeStep = 1;
                            try
                            {
                                point_creator_lst[fidx].setCoordSynthStr( clntMes.m_x_synthesis,
                                        clntMes.m_y_synthesis, clntMes.m_z_synthesis );
                                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                                {
                                    tmp_obj = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                                }
                                else if ( mvp.m_file_type == 3 )
                                {
                                    tmp_obj = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st, xvl);
                                }
                                else     // filetype: kvsml
                                {
                                    tmp_obj = point_creator_lst[fidx].run( param, *clntMes.m_camera, timeStep, st );
                                }

                                size_t nmemb = tmp_obj->nvertices() * 3;
                                // modify by @hira at 2016/12/01  
                                int c_count = 0;
                                for ( int tf = 0; tf < transfunc_creator.transfunc().size(); tf++ )
                                {
                                    int c_nbins = tmp_obj->getNbins();
                                    //changed by shimomura 2023/07/24
                                    tmp_max[2*tf+1] = vismodule::Math::Max(tmp_max[2*tf+1],param.m_transfunc_synthesizer-> m_c_max[tf]);
                                    tmp_min[2*tf+1] = vismodule::Math::Min(tmp_min[2*tf+1],param.m_transfunc_synthesizer-> m_c_min[tf]);
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
                                    tmp_max[2*tf] = vismodule::Math::Max(tmp_max[2*tf],param.m_transfunc_synthesizer-> m_c_max[tf]);
                                    tmp_min[2*tf] = vismodule::Math::Min(tmp_min[2*tf],param.m_transfunc_synthesizer-> m_c_min[tf]);
                                    for ( int res = 0; res < o_nbins; res++ )
                                    {
                                        tmp_o_bins[o_count] += tmp_obj->getOHistogram()[ o_count ] ;
                                        o_count++;
                                    }
                                }

                            }
                            catch ( const std::runtime_error& e )
                            {
#ifdef _DEBUG          // debug by @hira
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

                    std::stringstream tt;
                    tt << "t1";  
                    servMes.m_glyph_color_min  = vr.min( tt.str() + "_var_c" );
                    servMes.m_glyph_color_max  = vr.max( tt.str() + "_var_c" ); 
                    servMes.m_glyph_size_min   = vr.min( tt.str() + "_var_c" );
                    servMes.m_glyph_size_max   = vr.max( tt.str() + "_var_c" ); 

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
#endif
                    strncpy( servMes.m_header, "JPTP /1.0 000 OK\r\n", 18 );
                    servMes.m_number_particle = 0;
                    servMes.m_number_glyph = 0 ;
                    servMes.m_number_volume_divide = mvpl.m_total_number_subvolumes;
                    servMes.m_time_step = mvpl.m_total_start_steps;
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
                    transfunc_creator.setTransferFunction(&servMes, vr); 

#if 1
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

                    for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
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
                } // end of change PFI
                //else
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_particle )
                {
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

                        transfunc_creator.setProtocol( clntMes );
                        transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                        param.m_transfunc_synthesizer = transfunc_creator.create();

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
                        if ( param.m_gt5d == true || param.m_gt5d_full == true )
                        {
                            int timeStep = servMes.m_time_step;

                            if ( servMes.m_time_step > 1 )
                            {
                                for ( int nf = 0; nf < point_creator_lst.size(); nf++ )
                                    point_creator_lst[nf].progressValues();
                            }
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

                        // 関数の領域確保、初期化を行う : by @hira 2016/12/01
                        servMes.initializeTransferFunction(clntMes.m_transfer_function.size(), DEFAULT_NBINS);

                        c_bins_size = 0;
                        o_bins_size = 0;
                        for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
                        {
                            c_bins_size += servMes.m_color_nbins[tf];
                            o_bins_size += servMes.m_opacity_nbins[tf];
                        }

                        tmp_c_bins = new vismodule::UInt64[c_bins_size];
                        tmp_o_bins = new vismodule::UInt64[o_bins_size];

                        //add by shimomura 2023/06/14
                        int cnt = 2* servMes.m_transfer_function_count ;
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

                            size_t found_pfi = mvp.m_file_path.find(".pfi");
                            size_t found_vtm = mvp.m_file_path.find(".vtm");
                            
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
                            else if ( found_vtm != std::string::npos )
                            {
                                param.m_input_data = mvp.m_file_path;
                            }
#endif
                            else
                            {
                                std::cout << "このファイルは現在対応していません" << std::endl;
                            }

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
                                else if ( mvp.m_file_type == 3 )
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
                            pts.sendMessage( servMes );
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
                } // end of initParam == 1 generate_particle 
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::generate_glyph )
                {
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
                            //glyph_creator_lst[fidx].setFilterInfo(mvpl.m_list[fidx]);

                            vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;
                            std::stringstream suffix;
                            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                            vismodule::File ifpx( mvpl.m_list[fidx].m_file_path );
                            param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                                 + ifpx.baseName() + suffix.str() + ".kvsml";
                            param.m_subvolume_id = xvl;
                            int timeStep = 1;
                            servMes.m_flag_send_bins = 2;
                            try
                            {
                                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                                {
                                    *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 
                                    // run()で得られるKVSMLObjectglyphとtmp_objは異なるメモリ領域を指しているため,ポインタコピーではなくオペレータを呼び出す必要がある
                                }
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
                } // end of initParam = 3 // generateglyph
                else if ( clntMes.m_initialize_parameter ==  jpv::InitializeParameter::plot_over_line )
                {
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

                            vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
                            std::stringstream suffix;
                            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                            vismodule::File ifpx( mvpl.m_list[fidx].m_file_path );
                            param.m_input_data = ifpx.pathName() + ifpx.Separator()
                                                 + ifpx.baseName() + suffix.str() + ".kvsml";
                            param.m_subvolume_id = xvl ;
                            int timeStep = 1;
                            servMes.m_flag_send_bins = 2;
                            try
                            {
                                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                                {
                                    // run()で得られるKVSMLObjectglyphとtmp_objは異なるメモリ領域を指しているため,ポインタコピーではなくオペレータを呼び出す必要がある
                                }
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

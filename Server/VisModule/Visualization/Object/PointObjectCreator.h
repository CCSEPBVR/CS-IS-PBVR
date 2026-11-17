#ifndef VIS_MODULE_POINT_OBJECT_CREATOR_H_INCLDE
#define VIS_MODULE_POINT_OBJECT_CREATOR_H_INCLDE

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/CS_PointObjectGenerator>
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

//using FuncParser::Variable;
//using FuncParser::Variables;
//using FuncParser::Function;
//using FuncParser::FunctionParser;


class PointObjectCreator
{
private:

    vismodule::UnstructuredVolumeObject* m_volume;

    vismodule::CS_PointObjectGenerator m_generator;

    int m_mpi_rank;

    const MultiVolumeProperty* m_mvp;

    std::string m_xcSynthStr;
    std::string m_ycSynthStr;
    std::string m_zcSynthStr;

public:

    PointObjectCreator();
//    PointObjectCreator()
//        : m_volume(NULL), m_mpi_rank(0), m_mvp(NULL) {}

    ~PointObjectCreator(){};
//    ~PointObjectCreator()
//    {
//    }

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
        std::size_t i3 = 0;
        for ( std::size_t i = 0; i < nnodes; i++, i3 += 3 )
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

    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int timeStep,  const int st = 1 );
//    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int timeStep,  const int st = 1 )
//    {
//        m_generator.setFinlterInfo( m_mvp );
//        m_generator.setCoordSynthTS( st );
//
//        struct stat s;
//        if ( stat( param.m_input_data.c_str(), &s ) )
//        {
//            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
//            exit( 1 );
//        }
//        m_generator.createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step );
//
//        vismodule::PointObject* po = m_generator.getPointObject();
//        return po;
//    }

    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int timeStep, const int st, const int vl);
//    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int timeStep, const int st, const int vl)
//    {
//        m_generator.setFinlterInfo( m_mvp );
//        m_generator.setCoordSynthTS( st );
//        m_generator.createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step, st, vl );
//        vismodule::PointObject* po = m_generator.getPointObject();
//        return po;
//    }

    void setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss );
//    void setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss )
//    {
//        m_xcSynthStr = xss;
//        m_ycSynthStr = yss;
//        m_zcSynthStr = zss;
//
//        vismodule::CoordSynthesizerStrings css( 0, xss, yss, zss );
//        m_generator.setCoordSynthStrs( css );
//    }

    //2023 shimomura
    //void setCoordSynthTkn( const EquationToken & xst, const EquationToken & yst, const EquationToken & zst )
    void setCoordSynthTkn( const jpv::ParticleTransferClientMessage::EquationToken & xst,
                           const jpv::ParticleTransferClientMessage::EquationToken & yst,
                           const jpv::ParticleTransferClientMessage::EquationToken & zst );

//    void setCoordSynthTkn( const jpv::ParticleTransferClientMessage::EquationToken & xst,
//                           const jpv::ParticleTransferClientMessage::EquationToken & yst,
//                           const jpv::ParticleTransferClientMessage::EquationToken & zst )
//    {
////        m_xcSynthStr = xss;
////        m_ycSynthStr = yss;
////        m_zcSynthStr = zss;
//
//        vismodule::EquationToken xst_tmp;
//        vismodule::EquationToken yst_tmp;
//        vismodule::EquationToken zst_tmp;
//
//        for(int i=0; i<128; i++ )
//        {
//            xst_tmp.exp_token[i] = xst.exp_token[i];
//            xst_tmp.var_name[i]  = xst.var_name[i] ;
//            xst_tmp.val_array[i] = xst.value_array[i];
//            yst_tmp.exp_token[i] = yst.exp_token[i];
//            yst_tmp.var_name[i]  = yst.var_name[i] ;
//            yst_tmp.val_array[i] = yst.value_array[i];
//            zst_tmp.exp_token[i] = zst.exp_token[i];
//            zst_tmp.var_name[i]  = zst.var_name[i] ;
//            zst_tmp.val_array[i] = zst.value_array[i];
//        }
//
//        vismodule::CoordSynthesizerTokens cst(xst_tmp, yst_tmp, zst_tmp );
//        m_generator.setCoordSynthTkns( cst );
//    }

protected:
#if 0
    bool apply_coordinate_synthesizer( vismodule::PointObject* po, const int timeStep )
    {
        if ( ! po ) return false;
        if ( m_xcSynthStr.empty() && m_ycSynthStr.empty() && m_zcSynthStr.empty() )
            return true;
        std::size_t i, j, nv = po->nvertices();
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
#endif
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
        for ( std::size_t k = 0; k < elem.z(); k++ )
        {
            for ( std::size_t j = 0; j < elem.y(); j++ )
            {
                for ( std::size_t i = 0; i < elem.x(); i++ )
                {
                    m_index[0] = i + j * line_size + k * slice_size;
                    m_index[1] = m_index[0] + 1;
                    m_index[2] = m_index[1] + line_size;
                    m_index[3] = m_index[0] + line_size;
                    m_index[4] = m_index[0] + slice_size;
                    m_index[5] = m_index[1] + slice_size;
                    m_index[6] = m_index[2] + slice_size;
                    m_index[7] = m_index[3] + slice_size;

                    for ( std::size_t p = 0; p < 8; p++ )
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

#endif

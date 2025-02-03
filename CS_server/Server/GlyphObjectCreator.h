/****************************************************************************/
/**
 *  @file GlyphObjectCreator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GlyphObjectCreator.h 634 2025-01-31 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/

#ifndef KVS__GLYPH_OBJECT_CREATOR_H_INCLUDE
#define KVS__GLYPH_OBJECT_CREATOR_H_INCLUDE

#include <vector>
#include <string>
#include "UnstructuredVolumeObject.h"
#include "FilterInformation.h"
#include "ExtendedTransferFunction.h"
#include "timer.h"
#include "PointObjectGenerator.h"
#include "PointObject.h"
#include "GlyphObjectGenerator.h"
#include "KVSMLObjectGlyph.h"

class GlyphObjectCreator
{
private:

    pbvr::UnstructuredVolumeObject* m_volume;

    //pbvr::PointObjectGenerator m_generator;
    pbvr::GlyphObjectGenerator m_generator;

    int m_mpi_rank;

    const FilterInformationFile* m_fi;

    std::string m_xcSynthStr;
    std::string m_ycSynthStr;
    std::string m_zcSynthStr;

public:

    GlyphObjectCreator()
        : m_volume(NULL), m_mpi_rank(0), m_fi(NULL) {}

    ~GlyphObjectCreator()
    {
    }

    void setFilterInfo( const FilterInformationFile& fi )
    {
        m_fi = &fi;
    }

public:

    kvs::KVSMLObjectGlyph* run( const Argument& param, const kvs::Camera& camera, const int timeStep,  const int st = 1 )
    {
        m_generator.setFinlterInfo( m_fi );
        m_generator.setCoordSynthTS( st );

        struct stat s;
        if ( stat( param.m_input_data.c_str(), &s ) )
        {
            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
            exit( 1 );
        }
        m_generator.createFromFile( param, camera);

        kvs::KVSMLObjectGlyph* po = m_generator.getKVSMLObjectGlyph();
        return po;
    }

    kvs::KVSMLObjectGlyph* run( const Argument& param, const kvs::Camera& camera, const int timeStep, const int st, const int vl)
    {
        m_generator.setFinlterInfo( m_fi );
        m_generator.setCoordSynthTS( st );
        m_generator.createFromFile( param, camera, st, vl );
        kvs::KVSMLObjectGlyph* po = m_generator.getKVSMLObjectGlyph();
        return po;
    }

    void setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss )
    {
        m_xcSynthStr = xss;
        m_ycSynthStr = yss;
        m_zcSynthStr = zss;

        pbvr::CoordSynthesizerStrings css( 0, xss, yss, zss );
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

        pbvr::EquationToken xst_tmp;
        pbvr::EquationToken yst_tmp;
        pbvr::EquationToken zst_tmp;

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

        pbvr::CoordSynthesizerTokens cst(xst_tmp, yst_tmp, zst_tmp );
        m_generator.setCoordSynthTkns( cst );
    }

protected:

//    bool apply_coordinate_synthesizer( pbvr::PointObject* po, const int timeStep )
//    {
//        if ( ! po ) return false;
//        if ( m_xcSynthStr.empty() && m_ycSynthStr.empty() && m_zcSynthStr.empty() )
//            return true;
//        register size_t i, j, nv = po->nvertices();
//        if ( nv < 1 ) return true;
//        FuncParser::Variables synth_vars;
//        FuncParser::Variable X;
//        X.tag( "X" );
//        synth_vars.push_back( X );
//        FuncParser::Variable x;
//        x.tag( "x" );
//        synth_vars.push_back( x );
//        FuncParser::Variable Y;
//        Y.tag( "Y" );
//        synth_vars.push_back( Y );
//        FuncParser::Variable y;
//        y.tag( "y" );
//        synth_vars.push_back( y );
//        FuncParser::Variable Z;
//        Z.tag( "Z" );
//        synth_vars.push_back( Z );
//        FuncParser::Variable z;
//        z.tag( "z" );
//        synth_vars.push_back( z );
//        FuncParser::Variable T;
//        T.tag( "T" );
//        synth_vars.push_back( T );
//        FuncParser::Variable t;
//        t.tag( "t" );
//        synth_vars.push_back( t );
//
//        T = t = ( float )timeStep;
//        const kvs::ValueArray<kvs::Real32>& org_coords = po->coords();
//        kvs::ValueArray<kvs::Real32> new_coords;
//        new_coords.deepCopy( org_coords );
//
//        if ( ! m_xcSynthStr.empty() )
//        {
//            FunctionParser synth_func_parse( m_xcSynthStr, m_xcSynthStr.size() + 1 );
//            FuncParser::Function synth_func;
//            FunctionParser::Error err = synth_func_parse.express( synth_func, synth_vars );
//            if ( err != FunctionParser::ERR_NONE )
//            {
//                std::stringstream ess;
//                ess << "Function : " << synth_func.str() << " error: "
//                    << FunctionParser::error_type_to_string[err].c_str() << std::endl;
//                throw TransferFunctionSynthesizer::NumericException( ess.str() );
//            }
//            for ( i = 0; i < nv; i++ )
//            {
//                j = i * 3;
//                X = x = org_coords[j];
//                Y = y = org_coords[j + 1];
//                Z = z = org_coords[j + 2];
//                float d = ( float )synth_func.eval();
//                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
//                new_coords[j] = d;
//            }
//        }
//        if ( ! m_ycSynthStr.empty() )
//        {
//            FunctionParser synth_func_parse( m_ycSynthStr, m_ycSynthStr.size() + 1 );
//            FuncParser::Function synth_func;
//            FunctionParser::Error err = synth_func_parse.express( synth_func, synth_vars );
//            if ( err != FunctionParser::ERR_NONE )
//            {
//                std::stringstream ess;
//                ess << "Function : " << synth_func.str() << " error: "
//                    << FunctionParser::error_type_to_string[err].c_str() << std::endl;
//                throw TransferFunctionSynthesizer::NumericException( ess.str() );
//            }
//            for ( i = 0; i < nv; i++ )
//            {
//                j = i * 3;
//                X = x = org_coords[j];
//                Y = y = org_coords[j + 1];
//                Z = z = org_coords[j + 2];
//                float d = ( float )synth_func.eval();
//                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
//                new_coords[j + 1] = d;
//            }
//        }
//        if ( ! m_zcSynthStr.empty() )
//        {
//            FunctionParser synth_func_parse( m_zcSynthStr, m_zcSynthStr.size() + 1 );
//            FuncParser::Function synth_func;
//            FunctionParser::Error err = synth_func_parse.express( synth_func, synth_vars );
//            if ( err != FunctionParser::ERR_NONE )
//            {
//                std::stringstream ess;
//                ess << "Function : " << synth_func.str() << " error: "
//                    << FunctionParser::error_type_to_string[err].c_str() << std::endl;
//                throw TransferFunctionSynthesizer::NumericException( ess.str() );
//            }
//            for ( i = 0; i < nv; i++ )
//            {
//                j = i * 3;
//                X = x = org_coords[j];
//                Y = y = org_coords[j + 1];
//                Z = z = org_coords[j + 2];
//                float d = ( float )synth_func.eval();
//                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
//                new_coords[j + 2] = d;
//            }
//        }
//        po->setCoords( new_coords );
//        return true;
//    }

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
};

#endif

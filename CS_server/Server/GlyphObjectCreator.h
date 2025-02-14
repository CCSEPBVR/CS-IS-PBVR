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
#include "../Common/ParticleTransferProtocol.h"

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

    void run( const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide ,const int timeStep, kvs::KVSMLObjectGlyph* object, const int st = 1 )
//    void run( const Argument& param, const kvs::Camera& camera, jpv::ParticleTransferClientMessage &clntMes , const int number_of_divide ,const int timeStep, kvs::KVSMLObjectGlyph* object, const int st = 1 )
    {
        pbvr::GlyphObjectGenerator generator;

        struct stat s;
        if ( stat( param.m_input_data.c_str(), &s ) )
        {
            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
            exit( 1 );
        }
            generator.createFromFile( param, camera, clntMes, number_of_divide);

        kvs::KVSMLObjectGlyph* po = generator.getKVSMLObjectGlyph();

        object -> setCoords(po->coords());
        object -> setColors(po->colors());
        object -> setDirections(po->directions());
        object -> setSizes(po->sizes());
        object -> setColorMin(po->colorMin());
        object -> setColorMax(po->colorMax());
        object -> setSizeMin(po->sizeMin());
        object -> setSizeMax(po->sizeMax());
        //return po;
    }

    kvs::KVSMLObjectGlyph* run( const Argument& param, const kvs::Camera& camera,const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide, const int timeStep, const int st, const int vl)
    {
        m_generator.setFinlterInfo( m_fi );
        m_generator.setCoordSynthTS( st );
        m_generator.createFromFile( param, camera, clntMes, number_of_divide, st, vl );
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

};

#endif

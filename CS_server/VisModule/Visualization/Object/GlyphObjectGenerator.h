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

#ifndef VIS_MODULE__GLYPH_OBJECT_GENERATOR_H_INCLUDE
#define VIS_MODULE__GLYPH_OBJECT_GENERATOR_H_INCLUDE

#include <vector>
#include <string>

#include <vismodule/Camera>
#include <vismodule/Argument>
#include "UnstructuredVolumeObject.h"
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ExtendedTransferFunction>
#include <vismodule/GlyphGenerator>
#include <vismodule/KVSMLObjectGlyph>

namespace vismodule
{
//class KVSMLObjectGlyph;
//class VolumeObjectBase;
//class UnstructuredVolumeObject;

class GlyphObjectGenerator
{
private:
    //vismodule::KVSMLObjectGlyph* m_object;
    
    vismodule::ValueArray<vismodule::Real32> m_coords;       ///< coordinate array
    vismodule::ValueArray<vismodule::UInt8>  m_colors;       ///< color(r,g,b) array
    vismodule::ValueArray<vismodule::Real32> m_directions;   ///< directions array
    vismodule::ValueArray<vismodule::Real32> m_sizes;        ///< size array


    const MultiVolumeProperty*   m_mvp;

    vismodule::CoordSynthesizerStrings m_coord_synthesizer_strings;
    vismodule::CoordSynthesizerTokens  m_coord_synthesizer_tokens;

public:

    vismodule::KVSMLObjectGlyph m_object;

    //GlyphObjectGenerator() : m_object( NULL ), m_mvp(NULL) {m_object = new vismodule::KVSMLObjectGlyph;}
    //GlyphObjectGenerator() :  m_mvp(NULL) {m_object = new vismodule::KVSMLObjectGlyph;}
    GlyphObjectGenerator() :  m_mvp(NULL) {}
    ~GlyphObjectGenerator()
    {
//        if(m_object) delete m_object;
    }

    void createFromFile(
        const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide);
        //const Argument& param, const vismodule::Camera& camera);

    void createFromFile(
        const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide, const int st, const int vl );

    vismodule::KVSMLObjectGlyph* getKVSMLObjectGlyph()
    {
        return &m_object;
    }

    std::string getErrorMessage( const size_t maxMemory ) const;

    void setFilterInfo( const MultiVolumeProperty *mvp )
    {
        m_mvp = mvp;
    }

    void setCoordSynthStrs( const vismodule::CoordSynthesizerStrings& css )
    {
        m_coord_synthesizer_strings = css;
    }

    void setCoordSynthTkns( const vismodule::CoordSynthesizerTokens& cst )
    {
        m_coord_synthesizer_tokens.m_x_coord_synthesizer_token = cst.m_x_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_y_coord_synthesizer_token = cst.m_y_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_z_coord_synthesizer_token = cst.m_z_coord_synthesizer_token;
    }

    void setCoordSynthTS( const int ts )
    {
        m_coord_synthesizer_strings.m_time_step = ts;
    }

    vismodule::CoordSynthesizerStrings getCoordSynthStrs() const
    {
        return m_coord_synthesizer_strings;
    }

    vismodule::CoordSynthesizerTokens  getCoordSynthTkns() const
    {
        return m_coord_synthesizer_tokens;
    }

    void run( const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide ,const int timeStep, vismodule::KVSMLObjectGlyph* object, const int st = 1 );
//    void run( const Argument& param, const vismodule::Camera& camera, jpv::ParticleTransferClientMessage &clntMes , const int number_of_divide ,const int timeStep, vismodule::KVSMLObjectGlyph* object, const int st = 1 )
//    {
//        //vismodule::GlyphObjectGenerator generator;
//        //generator.setFilterInfo(m_mvp);
//
//        struct stat s;
//        if ( stat( param.m_input_data.c_str(), &s ) )
//        {
//            std::cout << "Error. read failed:" << param.m_input_data << std::endl;
//            exit( 1 );
//        }
//            this->createFromFile( param, camera, clntMes, number_of_divide);
//
//        vismodule::KVSMLObjectGlyph* po = this->getKVSMLObjectGlyph();
//
//        object -> setCoords(po->coords());
//        object -> setColors(po->colors());
//        object -> setDirections(po->directions());
//        object -> setSizes(po->sizes());
//        object -> setColorMin(po->colorMin());
//        object -> setColorMax(po->colorMax());
//        object -> setSizeMin(po->sizeMin());
//        object -> setSizeMax(po->sizeMax());
//        //return po;
//    }

#ifdef EXTEND_FILE_FORMAT
    void run( const Argument& param, const vismodule::Camera& camera,const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide, const int timeStep, vismodule::KVSMLObjectGlyph* object, const int st, const int vl);
//    {
////        vismodule::GlyphObjectGenerator generator;
//        this ->setFilterInfo( m_mvp );
//        this ->createFromFile( param, camera, clntMes, number_of_divide, st, vl );
//        vismodule::KVSMLObjectGlyph* po = this->getKVSMLObjectGlyph();
//        
//        object -> setCoords(po->coords());
//        object -> setColors(po->colors());
//        object -> setDirections(po->directions());
//        object -> setSizes(po->sizes());
//        object -> setColorMin(po->colorMin());
//        object -> setColorMax(po->colorMax());
//        object -> setSizeMin(po->sizeMin());
//        object -> setSizeMax(po->sizeMax());
//    }
#endif

    vismodule::KVSMLObjectGlyph* run( const Argument& param, const vismodule::Camera& camera,const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide, const int timeStep, const int st, const int vl);
//    vismodule::KVSMLObjectGlyph* run( const Argument& param, const vismodule::Camera& camera,const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide, const int timeStep, const int st, const int vl)
//    {
//        this->setFilterInfo( m_mvp );
//        this->setCoordSynthTS( st );
//        this->createFromFile( param, camera, clntMes, number_of_divide, st, vl );
//        vismodule::KVSMLObjectGlyph* po = this->getKVSMLObjectGlyph();
//        return po;
//    }


public:

    const vismodule::ValueArray<vismodule::Real32>& coords( void ) const;

    const vismodule::ValueArray<vismodule::UInt8>& colors( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& directions( void ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes( void ) const;

public:

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setColors( const vismodule::ValueArray<vismodule::UInt8>& colors );

    void setDirections( const vismodule::ValueArray<vismodule::Real32>& deirections );
    
    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void clear();
private:
    //vismodule::KVSMLObjectGlyph* sampling( vismodule::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes);
    void sampling( vismodule::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide);

};

}

#endif    // PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE

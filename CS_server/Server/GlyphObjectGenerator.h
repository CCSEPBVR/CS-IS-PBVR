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

#ifndef KVS__GLYPH_OBJECT_GENERATOR_H_INCLUDE
#define KVS__GLYPH_OBJECT_GENERATOR_H_INCLUDE

#include <vector>
#include <string>

#include <kvs/Camera>
#include "Argument.h"
#include "UnstructuredVolumeObject.h"
#include "FilterInformation.h"
#include "ExtendedTransferFunction.h"
#include "timer.h"
#include "GlyphGenerator.h"
#include "KVSMLObjectGlyph.h"

namespace pbvr
{
//class KVSMLObjectGlyph;
//class VolumeObjectBase;
//class UnstructuredVolumeObject;

class GlyphObjectGenerator
{
private:
    //kvs::KVSMLObjectGlyph* m_object;
    
    kvs::ValueArray<kvs::Real32> m_coords;       ///< coordinate array
    kvs::ValueArray<kvs::UInt8>  m_colors;       ///< color(r,g,b) array
    kvs::ValueArray<kvs::Real32> m_directions;   ///< directions array
    kvs::ValueArray<kvs::Real32> m_sizes;        ///< size array


    const FilterInformationFile*   m_fi;

    pbvr::CoordSynthesizerStrings m_coord_synthesizer_strings;
    pbvr::CoordSynthesizerTokens  m_coord_synthesizer_tokens;

public:

    kvs::KVSMLObjectGlyph m_object;

    //GlyphObjectGenerator() : m_object( NULL ), m_fi(NULL) {m_object = new kvs::KVSMLObjectGlyph;}
    //GlyphObjectGenerator() :  m_fi(NULL) {m_object = new kvs::KVSMLObjectGlyph;}
    GlyphObjectGenerator() :  m_fi(NULL) {}
    ~GlyphObjectGenerator()
    {
//        if(m_object) delete m_object;
    }

    void createFromFile(
        const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes);
        //const Argument& param, const kvs::Camera& camera);

    void createFromFile(
        const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int st, const int vl );

    kvs::KVSMLObjectGlyph* getKVSMLObjectGlyph()
    {
        return &m_object;
    }

    std::string getErrorMessage( const size_t maxMemory ) const;

    void setFinlterInfo( const FilterInformationFile *fi )
    {
        m_fi = fi;
    }

    void setCoordSynthStrs( const pbvr::CoordSynthesizerStrings& css )
    {
        m_coord_synthesizer_strings = css;
    }

    void setCoordSynthTkns( const pbvr::CoordSynthesizerTokens& cst )
    {
        m_coord_synthesizer_tokens.m_x_coord_synthesizer_token = cst.m_x_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_y_coord_synthesizer_token = cst.m_y_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_z_coord_synthesizer_token = cst.m_z_coord_synthesizer_token;
    }

    void setCoordSynthTS( const int ts )
    {
        m_coord_synthesizer_strings.m_time_step = ts;
    }

    pbvr::CoordSynthesizerStrings getCoordSynthStrs() const
    {
        return m_coord_synthesizer_strings;
    }

    pbvr::CoordSynthesizerTokens  getCoordSynthTkns() const
    {
        return m_coord_synthesizer_tokens;
    }

public:

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::Real32>& directions( void ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;

public:

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setDirections( const kvs::ValueArray<kvs::Real32>& deirections );
    
    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

    void clear();
private:
    //kvs::KVSMLObjectGlyph* sampling( pbvr::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes);
    void sampling( pbvr::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes);

};

}

#endif    // PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE

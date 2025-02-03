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
    //pbvr::KVSMLObjectGlyph* m_object;
    kvs::KVSMLObjectGlyph* m_object;
    const FilterInformationFile*   m_fi;

    pbvr::CoordSynthesizerStrings m_coord_synthesizer_strings;
    pbvr::CoordSynthesizerTokens  m_coord_synthesizer_tokens;

public:

    GlyphObjectGenerator() : m_object( NULL ), m_fi(NULL) {};
    ~GlyphObjectGenerator()
    {
        std::cout << "test_deallocate !" <<std::endl;
        //delete m_object;
    }

    void createFromFile(
        const Argument& param, const kvs::Camera& camera);

    void createFromFile(
        const Argument& param, const kvs::Camera& camera, const int st, const int vl );

//    pbvr::KVSMLObjectGlyph* getKVSMLObjectGlyph()
//    {
//        return m_object;
//    }
//    pbvr::KVSMLObjectGlyph* getKVSMLObjectGlyph()
    kvs::KVSMLObjectGlyph* getKVSMLObjectGlyph()
    {
        return m_object;
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

private:
    //kvs::KVSMLObjectGlyph* sampling( const Argument& param, const kvs::Camera& camera, pbvr::VolumeObjectBase* volume, const size_t subpixel_level, const float sampling_step );
    kvs::KVSMLObjectGlyph* sampling( pbvr::VolumeObjectBase* volume);

};

}

#endif    // PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE

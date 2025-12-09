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

#ifndef VIS_MODULE__GLYPH_SEED_GENERATOR_H_INCLUDE
#define VIS_MODULE__GLYPH_SEED_GENERATOR_H_INCLUDE

#include <memory>
#include <vector>
#include <string>

#include <vismodule/ValueArray>
#include <vismodule/Camera>
#include <vismodule/Argument>
#include "UnstructuredVolumeObject.h"
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ExtendedTransferFunction>
#include <vismodule/GlyphSeed>
#include <vismodule/KVSMLObjectGlyph>
#include <vismodule/GlyphProperty>

namespace vismodule
{
class GlyphSeedGenerator
{
private:
    const MultiVolumeProperty*   m_mvp;

    vismodule::CoordSynthesizerStrings m_coord_synthesizer_strings;
    vismodule::CoordSynthesizerTokens  m_coord_synthesizer_tokens;

public:

    vismodule::KVSMLObjectGlyph m_object;
    GlyphSeedGenerator() {}
    ~GlyphSeedGenerator() {}

    void GenerateGlyphStruct(
        const GlyphProperty& glyph_property,
        const int number_of_divide,
        const domain_parameters_struct& dom,
        Type** values,
        int nvariables,
        const ServerMode& server_mode,
        vismodule::KVSMLObjectGlyph* object
    );

    void GenerateGlyphUnstruct(
        const GlyphProperty& glyph_property,
        const int number_of_divide,
        Type** values,
        int nvariables,
        float* coordinates,
        int ncoords,
        unsigned int* connections,
        int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype,
        const ServerMode& server_mode,
        vismodule::KVSMLObjectGlyph* object
    );

    vismodule::KVSMLObjectGlyph* getKVSMLObjectGlyph()
    {
        return &m_object;
    }
}; // class GlyphSeedGenerator
} // namespace vismodule

#endif    // PBVR__POINT_OBJECT_GENERATOR_H_INCLUDE

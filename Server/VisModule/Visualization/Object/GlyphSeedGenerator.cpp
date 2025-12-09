#include "GlyphSeedGenerator.h"
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellHistogram>
#include <vismodule/GlyphProperty>

using namespace vismodule;

void GlyphSeedGenerator::GenerateGlyphStruct(
    const GlyphProperty& glyph_property,
    const int number_of_divide,
    const domain_parameters_struct& dom,
    Type** values,
    int nvariables,
    const ServerMode& server_mode,
    vismodule::KVSMLObjectGlyph* object
)
{
    bool is_flag = false;

    if ( server_mode == ServerMode::CS ) is_flag = false;
    else is_flag = true;

    GlyphSeed glyph_generator(
        glyph_property,
        number_of_divide,
        dom,
        values,
        nvariables,
        is_flag
    );
    glyph_generator.getGlyphData(&m_object);

    vismodule::KVSMLObjectGlyph* tmp_obj = this->getKVSMLObjectGlyph();

    object->setCoords( tmp_obj->coords() );
    object->setColors( tmp_obj->colors() );
    object->setDirections( tmp_obj->directions() );
    object->setSizes( tmp_obj->sizes() );
    object->setColorMin( tmp_obj->colorMin() );
    object->setColorMax( tmp_obj->colorMax() );
    object->setSizeMin( tmp_obj->sizeMin() );
    object->setSizeMax( tmp_obj->sizeMax() );
}

void GlyphSeedGenerator::GenerateGlyphUnstruct(
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
)
{
    bool is_flag = false;

    if ( server_mode == ServerMode::CS ) is_flag = false;
    else is_flag = true;

    GlyphSeed glyph_generator(
        glyph_property,
        number_of_divide,
        values,
        nvariables,
        coordinates,
        ncoords,
        connections,
        ncells,
        celltype,
        is_flag
    );

    glyph_generator.getGlyphData(&m_object);
    vismodule::KVSMLObjectGlyph* tmp_obj = this->getKVSMLObjectGlyph();

    object->setCoords( tmp_obj->coords() );
    object->setColors( tmp_obj->colors() );
    object->setDirections( tmp_obj->directions() );
    object->setSizes( tmp_obj->sizes() );
    object->setColorMin( tmp_obj->colorMin() );
    object->setColorMax( tmp_obj->colorMax() );
    object->setSizeMin( tmp_obj->sizeMin() );
    object->setSizeMax( tmp_obj->sizeMax() );
}

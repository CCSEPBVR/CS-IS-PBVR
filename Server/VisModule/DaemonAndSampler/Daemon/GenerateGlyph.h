#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/MultiVolumeProperty>
#include <kvs/PolygonGlyphObject>

void SetGlyphParameterCS(
    const ParticleProperty& particle_property,
    GlyphProperty& glyph_property,
    const MultiVolumePropertyList& mvpl
);

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphCS(
    ParticleProperty& particle_property,
    const GlyphProperty& glyph_property,
    MultiVolumePropertyList& mvpl
);

/*
void generate_glyph(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
    #ifndef CPU_VER
    JobCollector& jc,
    #endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    jpv::ServerMode server_mode
);
*/

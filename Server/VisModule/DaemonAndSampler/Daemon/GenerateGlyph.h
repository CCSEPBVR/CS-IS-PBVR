#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/MultiVolumeProperty>
#include <kvs/PolygonGlyphObject>

void SetDefaultGlyphParameterCS( GlyphProperty& glyph_property );

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphCS(
    std::string& file_path,
    const int time_step,
    const GlyphProperty& glyph_property,
    MultiVolumePropertyList& mvpl
);

/*
void SetGlyphParameterIS(
    GlyphProperty& glyph_property
);
*/

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphIS(
    const int time_step,
    const GlyphProperty& glyph_property,
    const MultiVolumePropertyList& mvpl
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

#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/MultiVolumeProperty>
#include <kvs/PolygonGlyphObject>

void SetDefaultGlyphParameter( GlyphProperty& glyph_property );

void SetDefaultGlyphParameterCS( GlyphProperty& glyph_property );

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphCS(
    std::string& file_path,
    const int time_step,
    const GlyphProperty& glyph_property,
    MultiVolumePropertyList& mvpl
);

void SetDefaultGlyphParameterIS( GlyphProperty& glyph_property );

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphIS(
    const int time_step,
    const GlyphProperty& glyph_property,
    const MultiVolumePropertyList& mvpl
);

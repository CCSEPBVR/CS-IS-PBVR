#ifndef __PARAMETER_FILE_WRITER_H__
#define __PARAMETER_FILE_WRITER_H__

#include <vismodule/NameListFile>
#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>

class ParameterFileWriter
{
private:
    NameListFile m_name_list_file;

public:
    void getParticleParameter( const ParticleProperty& particle_property );
    void getGlyphParameter( const GlyphProperty& glyph_property );
    void getPlotOverLineParameter( const PlotOverLineProperty& pol_property );

    void writeParameterFile( const char* fname );
    const NameListFile& getNameListFile() const;
};

#endif

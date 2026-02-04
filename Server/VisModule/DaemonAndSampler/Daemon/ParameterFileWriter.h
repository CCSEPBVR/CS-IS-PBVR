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
    std::string m_particle_parameter_path;
    std::string m_particle_parameter_old_path;
    std::string m_glyph_parameter_path;
    std::string m_pol_parameter_path;

public:
    ParameterFileWriter();
    std::string getParticleParameterPath();
    std::string getGlyphParameterPath();
    std::string getPOLParameterPath();
    void getParticleParameter( const ParticleProperty& particle_property );
    void getGlyphParameter( const GlyphProperty& glyph_property );
    void getPlotOverLineParameter( const PlotOverLineProperty& pol_property );
    void writeParticleParameterFile();
    void writeParticleParameterOldFile();
    void writeGlyphParameterFile();
    void writePlotOverLineParameterFile();
    const NameListFile& getNameListFile() const;
};

#endif

#ifndef __JUPITER_PARTICLE_MONITOR_H__
#define __JUPITER_PARTICLE_MONITOR_H__

#include <string>
#include <vismodule/Type>
#include <vismodule/PointObject>
//#include "PointObject.h"
#include "ParticleFile.h"
#include "GlyphFile.h"
#include "ParticleStatusFile.h"
#include "ParticleHistoryFile.h"
//#include <vismodule/GlyphObject>
#include <vismodule/KVSMLObjectGlyph>
#include <vismodule/KVSMLObjectPlotOverLine>
//#include "KVSMLObjectGlyph.h"

#include "PlotOverLineFile.h"
//#include "KVSMLObjectPlotOverLine.h"

class ParticleMonitor
{
private:
    ParticleFile          m_particle_file; 
    GlyphFile             m_glyph_file; 
    PlotOverLineFile      m_plot_over_line_file; 
    ParticleStatusFile    m_status_file; 
    ParticleHistoryFile   m_history_file; 
    std::string           m_history_file_prefix;
    vismodule::PointObject     m_particle; 
    vismodule::KVSMLObjectGlyph      m_glyph; 
    vismodule::KVSMLObjectPlotOverLine    m_plot_over_line; 
    vismodule::Int32            m_time_step_particle;
    vismodule::Int32            m_time_step_glyph;
    vismodule::Int32            m_time_step_pol;

private:
    inline bool checking_status_file();

public:
    ParticleMonitor();
    ~ParticleMonitor();
    ParticleMonitor( const std::string& particle_file_prefix,
                     const std::string& particle_status_file_name,
                     const std::string& particle_history_file_prefix );
    ParticleMonitor( const std::string& particle_file_prefix,
                     const std::string& glyph_file_prefix,
                     const std::string& plot_over_line_file_prefix,
                     const std::string& particle_status_file_name,
                     const std::string& particle_history_file_prefix );

    void setParticleFilePrefix( const std::string& prefix );
    void setGlyphFilePrefix( const std::string& prefix );
    void setPlotOverLineFilePrefix( const std::string& prefix );
    void setParticleStatusFileName( const std::string& file_name );
    void setParticleHistoryFileName( const std::string& file_name );
    void setParticleHistoryFilePrefix( const std::string& prefix );
    bool setTimeStep_particle( const vismodule::Int32 time_step );
    bool setTimeStep_glyph( const vismodule::Int32 time_step );
    bool setTimeStep_pol( const vismodule::Int32 time_step );
    void check();
    void readParticleHistoryFile();
    void readParticleFile();
    bool findGlyphFile();
    void readGlyphFile();
    bool findPlotOverLineFile();
    void readPlotOverLineFile();
    void getParticle( vismodule::PointObject* object );
    void getGlyph( vismodule::KVSMLObjectGlyph* object );
    void getPlotOverLine( vismodule::KVSMLObjectPlotOverLine* object );
    //void getGlyph( vismodule::PointObject* object );
    vismodule::Int32 getSubpixelLevel();
    ParticleStatusFile& particleStatusFile();
    ParticleHistoryFile& particleHistoryFile();
    GlyphFile& glyphFile();
    PlotOverLineFile& plotOverLineFile();
    bool statusFileChanged();
    bool stepExisted();
    vismodule::Int32 getTimeStep_particle();
    vismodule::Int32 getTimeStep_glyph();
    vismodule::Int32 getTimeStep_pol();
};

inline bool ParticleMonitor::checking_status_file()
{
    return m_status_file.getStatus() != ParticleStatusFile::STEP_CHANGED;
}
#endif

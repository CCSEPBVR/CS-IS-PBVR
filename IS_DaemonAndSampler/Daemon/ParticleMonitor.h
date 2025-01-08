#ifndef __JUPITER_PARTICLE_MONITOR_H__
#define __JUPITER_PARTICLE_MONITOR_H__

#include <string>
#include <kvs/Type>
#include "PointObject.h"
#include "ParticleFile.h"
#include "GlyphFile.h"
#include "ParticleStatusFile.h"
#include "ParticleHistoryFile.h"
//#include <kvs/GlyphObject>
#include "KVSMLObjectGlyph.h"

class ParticleMonitor
{
private:
    ParticleFile          m_particle_file; 
    GlyphFile             m_glyph_file; 
    ParticleStatusFile    m_status_file; 
    ParticleHistoryFile   m_history_file; 
    std::string           m_history_file_prefix;
    pbvr::PointObject     m_particle; 
    kvs::KVSMLObjectGlyph      m_glyph; 
    kvs::Int32            m_time_step;

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
                     const std::string& particle_status_file_name,
                     const std::string& particle_history_file_prefix );

    void setParticleFilePrefix( const std::string& prefix );
    void setGlyphFilePrefix( const std::string& prefix );
    void setParticleStatusFileName( const std::string& file_name );
    void setParticleHistoryFileName( const std::string& file_name );
    void setParticleHistoryFilePrefix( const std::string& prefix );
    bool setTimeStep( const kvs::Int32 time_step );
    void check();
    void readParticleHistoryFile();
    void readParticleFile();
    bool findGlyphFile();
    void readGlyphFile();
    void getParticle( pbvr::PointObject* object );
    void getGlyph( kvs::KVSMLObjectGlyph* object );
    //void getGlyph( pbvr::PointObject* object );
    kvs::Int32 getSubpixelLevel();
    ParticleStatusFile& particleStatusFile();
    ParticleHistoryFile& particleHistoryFile();
    GlyphFile& glyphFile();
    bool statusFileChanged();
    bool stepExisted();
    kvs::Int32 getTimeStep();
};

inline bool ParticleMonitor::checking_status_file()
{
    return m_status_file.getStatus() != ParticleStatusFile::STEP_CHANGED;
}
#endif

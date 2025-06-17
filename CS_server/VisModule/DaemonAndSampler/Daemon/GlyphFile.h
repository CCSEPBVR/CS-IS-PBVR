#ifndef __JUPITER_GLYPH_FILE_H__
#define __JUPITER_GLYPH_FILE_H__

#include <string>
#include <sstream>
#include <algorithm>
#include <vismodule/Directory>
#include <vismodule/File>
#include <vismodule/Type>

#include <vismodule/PointObject>
#include <vismodule/PointImporter>
#include <vismodule/KVSMLObjectGlyph>
//#include "PointObject.h"
//#include "PointImporter.h"
//#include "KVSMLObjectGlyph.h"
#include <vismodule/GlyphObject>

class GlyphFile
{
private:
    vismodule::UInt32 m_subvolume_number ;
    vismodule::UInt32 m_initial_step;
    vismodule::UInt32 m_final_step;
    vismodule::UInt32 m_kvsml_file_number;
    std::string m_file_prefix;

public:
    void setFilePrefix( const std::string& prefix );
    void setParameterFromFile();
    void generatePointObject( const int time_step, vismodule::PointObject* object );
    void generateGlyphObject( const int time_step, vismodule::KVSMLObjectGlyph* object );
    int subVolumeNumber () {return m_subvolume_number;}
    int getStartTimeStep () {return m_initial_step;}
    int getLatestTimeStep () {return m_final_step;}
};

#endif

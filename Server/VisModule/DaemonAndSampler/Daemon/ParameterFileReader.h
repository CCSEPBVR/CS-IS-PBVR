#ifndef __PARAMETER_FILE_READER_H__
#define __PARAMETER_FILE_READER_H__

#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <algorithm>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/Camera>
#include <vismodule/ColorMap>
#include <vismodule/OpacityMap>
#include <vismodule/NameListFile>
#include <vismodule/ParticleProperty>

class ParameterFileReader
{
private:
    NameListFile m_name_list_file;
    
    void set_default_parameter();

public:
    void setParticleParameter( ParticleProperty& param );
    void setGlyphParameter( ParticleProperty& param );
    void setPlotOverLineParameter( ParticleProperty& param );
    void readParticleParameterFile( const char* fname );
    void readGlyphParameterFile( const char* fname );
    void readPlotOverLineParameterFile( const char* fname );
    std::vector<int> getTableInt( std::string table_string );
    std::vector<float> getTableFloat( std::string table_string );
    std::vector<std::string> getTableString( std::string table_string );
    void setNameListFile( const NameListFile& nameListFile );
    const NameListFile& getNameListFile() const;
};

#endif

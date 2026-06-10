#ifndef __PARAMETER_FILE_READER_H__
#define __PARAMETER_FILE_READER_H__

#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <algorithm>
#include <vector>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/Camera>
#include <vismodule/ColorMap>
#include <vismodule/OpacityMap>
#include <vismodule/NameListFile>
#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/PlotOverTimeProperty>

class ParameterFileReader
{
private:
    NameListFile m_name_list_file;
    
    void set_default_parameter();

    std::vector<int> getTableInt( std::string table_string );
    std::vector<float> getTableFloat( std::string table_string );
    std::vector<std::string> getTableString( std::string table_string );
public:
    void readTransferFunctionFile( const char* fname ); // CSのConnect時に指定した.tfファイルを読み込む
    void readParticleParameterFile( const char* fname ); // ISのdefault.tfファイルを読み込む
    void readGlyphParameterFile( const char* fname );
    void readPlotOverLineParameterFile( const char* fname );
    void readPlotOverTimeParameterFile( const char* fname );
    void setTransferFunctionParameter( ParticleProperty& particle_property ); // CSのConnect時に指定した.tfファイルを読み込んだ値を設定する
    void setParticleParameter( ParticleProperty& particle_property ); // ISのdefault.tfファイルを読み込んだ値を設定する
    void setGlyphParameter( GlyphProperty& glyph_property );
    void setPlotOverLineParameter( PlotOverLineProperty& pol_property );
    void setPlotOverTimeParameter( PlotOverTimeProperty& pot_property );
    void setNameListFile( const NameListFile& nameListFile );
    const NameListFile& getNameListFile() const;
};

#endif

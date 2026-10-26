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
#include <vismodule/TransferFunctionJsonWriter>

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
    void readParticleParameterFile( const char* fname ); // ISの旧形式default.tfファイルを読み込む
    bool readTransferFunctionFromJson( const char* fname, ParticleProperty& particle_property ); // ISのdefault.jsonファイルを読み込む
    void readGlyphParameterFile( const char* fname, GlyphProperty& glyph_property );
    void readPlotOverLineParameterFile( const char* fname, PlotOverLineProperty& pol_property );
    void readPlotOverTimeParameterFile( const char* fname, PlotOverTimeProperty& pot_property );
    void setTransferFunctionParameter( ParticleProperty& particle_property ); // CSのConnect時に指定した.tfファイルを読み込んだ値を設定する
    void setParticleParameter( ParticleProperty& particle_property ); // ISの旧形式default.tfファイルを読み込んだ値を設定する
    void setNameListFile( const NameListFile& nameListFile );
    const NameListFile& getNameListFile() const;
};

#endif

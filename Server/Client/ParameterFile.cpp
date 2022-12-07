#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include <map>

#include "ParameterFile.h"

bool ParameterFile::loadIN( const std::string& filename )
{
    std::ifstream fin( filename.c_str(), std::ios::in );
    std::string name;
    std::string value;
    std::string equal;
    const std::string TF_NAME = "TF_";
    if ( !fin.is_open() )
    {
        std::cerr << "Cannot open " << filename << std::endl;
        fin.close();
        return false;
    }

    std::string line;

    m_param_state.insert( std::map<std::string, bool>::value_type( "sub_pixel_level" , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "repeat_level"    , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "crop_region_0"   , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "crop_region_1"   , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "crop_region_2"   , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "crop_region_3"   , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "crop_region_4"   , false ) );
    m_param_state.insert( std::map<std::string, bool>::value_type( "crop_region_5"   , false ) );

    // delete by @hira at 2016/12/01
    //m_param_state.insert( std::map<std::string, bool>::value_type( "t1"   , false ) );
    //m_param_state.insert( std::map<std::string, bool>::value_type( "t2"   , false ) );
    //m_param_state.insert( std::map<std::string, bool>::value_type( "t3"   , false ) );
    //m_param_state.insert( std::map<std::string, bool>::value_type( "t4"   , false ) );
    //m_param_state.insert( std::map<std::string, bool>::value_type( "t5"   , false ) );

    while ( std::getline( fin, line ) )
    {
        size_t pos;
        while ( ( pos = line.find_first_of( " 　\t\r\n" ) ) != std::string::npos )
        {
            line.erase( pos, 1 );
        }

        if ( ( pos = line.find_first_of( "=" ) ) != std::string::npos )
        {
            name = line.substr( 0, pos );
            value = line.substr( pos + 1 );
        }
        else
        {
            name = "";
        }

        std::string key = "";
//        if( !strcmp( name.c_str(),"PBVR_SUB_PIXEL_LEVEL_DETAILED" ) )   key = "sub_pixel_level";
        if ( !strcmp( name.c_str(), "SUB_PIXEL_LEVEL" ) )   key = "sub_pixel_level";
//        else if( !strcmp( name.c_str(),"PBVR_REPEAT_LEVEL_DETAILED" ) ) key = "repeat_level";
        else if ( !strcmp( name.c_str(), "REPEAT_LEVEL" ) ) key = "repeat_level";
        else if ( !strcmp( name.c_str(), "CROP_CXMIN" ) )            key = "crop_region_0";
        else if ( !strcmp( name.c_str(), "CROP_CYMIN" ) )            key = "crop_region_1";
        else if ( !strcmp( name.c_str(), "CROP_CZMIN" ) )            key = "crop_region_2";
        else if ( !strcmp( name.c_str(), "CROP_CXMAX" ) )            key = "crop_region_3";
        else if ( !strcmp( name.c_str(), "CROP_CYMAX" ) )            key = "crop_region_4";
        else if ( !strcmp( name.c_str(), "CROP_CZMAX" ) )            key = "crop_region_5";
        else if ( !strcmp( name.c_str(), "TF_RESOLUTION" ) )         key = "tf_resolution";
#if 0		// delete by @hira at 2016/12/01
        else if ( !strcmp( name.c_str(), "TF_SYNTH" ) )              key = "tf_synthesis";
        else if ( name == "TF_NAME1_VAR_C"   ) key = name;
        else if ( name == "TF_NAME1_MIN_C"   ) key = name;
        else if ( name == "TF_NAME1_MAX_C"   ) key = name;
        else if ( name == "TF_NAME1_VAR_O"   ) key = name;
        else if ( name == "TF_NAME1_MIN_O"   ) key = name;
        else if ( name == "TF_NAME1_MAX_O"   ) key = name;
        else if ( name == "TF_NAME1_TABLE_C" ) key = name;
        else if ( name == "TF_NAME1_TABLE_O" ) key = name;
        else if ( name == "TF_NAME2_VAR_C"   ) key = name;
        else if ( name == "TF_NAME2_MIN_C"   ) key = name;
        else if ( name == "TF_NAME2_MAX_C"   ) key = name;
        else if ( name == "TF_NAME2_VAR_O"   ) key = name;
        else if ( name == "TF_NAME2_MIN_O"   ) key = name;
        else if ( name == "TF_NAME2_MAX_O"   ) key = name;
        else if ( name == "TF_NAME2_TABLE_C" ) key = name;
        else if ( name == "TF_NAME2_TABLE_O" ) key = name;
        else if ( name == "TF_NAME3_VAR_C"   ) key = name;
        else if ( name == "TF_NAME3_MIN_C"   ) key = name;
        else if ( name == "TF_NAME3_MAX_C"   ) key = name;
        else if ( name == "TF_NAME3_VAR_O"   ) key = name;
        else if ( name == "TF_NAME3_MIN_O"   ) key = name;
        else if ( name == "TF_NAME3_MAX_O"   ) key = name;
        else if ( name == "TF_NAME3_TABLE_C" ) key = name;
        else if ( name == "TF_NAME3_TABLE_O" ) key = name;
        else if ( name == "TF_NAME4_VAR_C"   ) key = name;
        else if ( name == "TF_NAME4_MIN_C"   ) key = name;
        else if ( name == "TF_NAME4_MAX_C"   ) key = name;
        else if ( name == "TF_NAME4_VAR_O"   ) key = name;
        else if ( name == "TF_NAME4_MIN_O"   ) key = name;
        else if ( name == "TF_NAME4_MAX_O"   ) key = name;
        else if ( name == "TF_NAME4_TABLE_C" ) key = name;
        else if ( name == "TF_NAME4_TABLE_O" ) key = name;
        else if ( name == "TF_NAME5_VAR_C"   ) key = name;
        else if ( name == "TF_NAME5_MIN_C"   ) key = name;
        else if ( name == "TF_NAME5_MAX_C"   ) key = name;
        else if ( name == "TF_NAME5_VAR_O"   ) key = name;
        else if ( name == "TF_NAME5_MIN_O"   ) key = name;
        else if ( name == "TF_NAME5_MAX_O"   ) key = name;
        else if ( name == "TF_NAME5_TABLE_C" ) key = name;
        else if ( name == "TF_NAME5_TABLE_O" ) key = name;
#endif
        else if ( name.find(TF_NAME) == 0 ) key = name;
        else
        {
            for ( int i = 1; i <= 5; i++ )
            {
                std::string num;
                std::stringstream ss;
                ss << i;
                num = ss.str();

                if ( !strcmp( name.c_str(), std::string( "T" + num + "_R" ).c_str() ) )   key = "t" + num + "_r";
                else if ( !strcmp( name.c_str(), std::string( "T" + num + "_G" ).c_str() ) )   key = "t" + num + "_g";
                else if ( !strcmp( name.c_str(), std::string( "T" + num + "_B" ).c_str() ) )   key = "t" + num + "_b";
                else if ( !strcmp( name.c_str(), std::string( "T" + num + "_A" ).c_str() ) )   key = "t" + num + "_a";
                if ( key != "" )
                {
                    m_param_state["t" + num] = true;
                    break;
                }
            }

            for ( int i = 1; i <= 5; i++ )
            {
                std::string num;
                std::stringstream ss;
                ss << i;
                num = ss.str();
                if ( !strcmp( name.c_str(), std::string( "T" + num + "_VAR" ).c_str() ) ) key = "t" + num + "_var";
                else if ( !strcmp( name.c_str(), std::string( "T" + num + "_MIN" ).c_str() ) ) key = "t" + num + "_min";
                else if ( !strcmp( name.c_str(), std::string( "T" + num + "_MAX" ).c_str() ) ) key = "t" + num + "_max";
            }
        }

        if ( key != "" )
        {
            m_param.insert( std::map<std::string, std::string>::value_type( key, value ) );
            m_param_state[key] = true;
        }
    }

    return true;
}

size_t ParameterFile::getSubpixelLevel()
{
    return std::atoi( m_param["sub_pixel_level"].c_str() );
}

size_t ParameterFile::getRepeatLevel()
{
    return std::atoi( m_param["repeat_level"].c_str() );
}

float ParameterFile::getCropRegion( const int index )
{
    float value;
    if (      index == 0 ) value = std::atof( m_param["crop_region_0"].c_str() );
    else if ( index == 1 ) value = std::atof( m_param["crop_region_1"].c_str() );
    else if ( index == 2 ) value = std::atof( m_param["crop_region_2"].c_str() );
    else if ( index == 3 ) value = std::atof( m_param["crop_region_3"].c_str() );
    else if ( index == 4 ) value = std::atof( m_param["crop_region_4"].c_str() );
    else if ( index == 5 ) value = std::atof( m_param["crop_region_5"].c_str() );
    else                 value = 0.0;

    return value;
}

int ParameterFile::getInt( const std::string& name )
{
    return std::atoi( m_param[name].c_str() );
}

float ParameterFile::getFloat( const std::string& name )
{
    return std::atof( m_param[name].c_str() );
}

std::string ParameterFile::getString( const std::string& name )
{
    if ( m_param.find( name ) == m_param.end() )
        return "";
    else
        return m_param[name];
}

/**
 * TF_NAME[NN]_XXXXの項目数を取得する
 * TF_NAME[NN]_VAR_Cの項目を検索する
 * @return		TF_NAME[NN]_XXXXの項目数：NNの数
 */
int ParameterFile::getTfnameSize()
{
    int count = 0;
    std::map<std::string, std::string>::const_iterator itr;
    for (itr=this->m_param.begin(); itr!=m_param.end(); itr++) {
        std::string item = itr->first;
        std::string value = itr->second;
        if (this->isTfnameNvalc(item)) count++;
    }
    return count;
}

/**
 * TF_NAME[NN]_XXXXの接頭文字列（TF_NAME[NN]）を取得する
 * @param index       インデックス（NNではない）
 * @return		TF_NAME[NN]
 */
std::string ParameterFile::getTfnamePrefix(int index)
{
    int count = 0;
    std::string prefix = "";
    std::map<std::string, std::string>::const_iterator itr;
    for (itr=this->m_param.begin(); itr!=m_param.end(); itr++) {
        std::string item = itr->first;
        std::string value = itr->first;
        if (!this->isTfnameNvalc(item)) continue;
        if (index == count) {
            int pos = item.find("_VAR_C");
            prefix = item.erase(pos);
            return prefix;
        }
        count++;
    }

    return prefix;
}


/**
 * TF_NAME[NN]_XXXXの番号（TF_NAME[NN]のNN）を取得する
 * @param index       インデックス（NNではない）
 * @return		NN
 */
int ParameterFile::getTfnameNumber(int index)
{
    int count = 0;
    std::string str_num = "";
    int num = 0;
    const std::string TF_NAME = "TF_NAME";
    std::map<std::string, std::string>::const_iterator itr;
    for (itr=this->m_param.begin(); itr!=m_param.end(); itr++) {
        std::string item = itr->first;
        std::string value = itr->first;
        if (!this->isTfnameNvalc(item)) continue;
        if (index == count) {
            int pos = item.find("_VAR_C");
            str_num = item.erase(pos);
            str_num = str_num.substr(TF_NAME.length());
            return std::atoi(str_num.c_str());
        }
        count++;
    }

    return -1;
}


/**
 * TF_NAME[NN]_VAR_Cの項目であるかチェックする
 * @return		true=TF_NAME[NN]_VAR_C
 */
bool ParameterFile::isTfnameNvalc(const std::string& item)
{
    if (item.find("TF_NAME") == std::string::npos) return false;
    if (item.find("TF_NAME") != 0) return false;
    if (item.find("_VAR_C") == std::string::npos) return false;

    return true;
}

/**
 * 読込パラメータが存在しているかチェックする
 * @param name		パラメータ項目名
 * @return			true=存在している
 */
bool ParameterFile::hasParam( const std::string& name )
{
    if (this->m_param.find(name) == this->m_param.end()) return false;

    return true;
}


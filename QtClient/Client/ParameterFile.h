#ifndef  PBVR__PARAMETER_FILE_H_INCLUDE
#define PBVR__PARAMETER_FILE_H_INCLUDE

#include <string>
#ifdef CS_MODE
#define MAX_TF_NUMBER		99
#endif

class ParameterFile
{

private:
    size_t m_subpixel_level;
    size_t m_repeat_level;
    float m_crop_region[6];
    std::map<std::string, bool> m_param_state;
    std::map<std::string, std::string> m_param;

public:
    bool getState( const std::string& key )
    {
        return m_param_state[key];
    };

    float getCropRegion( const int index );
    size_t getSubpixelLevel();
    size_t getRepeatLevel();

    //Osaki QTISPBVR update(2020/11/16) std::stringの&を削除
    int         getInt( const std::string& name );
    float       getFloat( const std::string& name );
    std::string getString( const std::string& name );

public:
    bool   loadIN( const std::string& filename );

    //Osaki QTISPBVR update(2020/11/16) OpenGL版のISでは使用しているためifdefを無効にしました。
//#ifdef CS_MODE
    int getTfnameSize();
    std::string getTfnamePrefix(int index);
    int getTfnameNumber(int index);
//#endif

    bool        hasParam( const std::string& name );

private:
    bool isTfnameNvalc(const std::string& item);
};

#endif // PBVR__PARAMETER_FILE_H_INCLUDE

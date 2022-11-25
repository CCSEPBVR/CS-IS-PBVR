#ifndef  PBVR__PARAMETER_FILE_H_INCLUDE
#define PBVR__PARAMETER_FILE_H_INCLUDE

#include <string>
#include <vector>

// add by @hira at 2016/12/01
// #define PBVR_FILTER_PARAMFILENAME    "FILTER_PARAMETER_FILENAME"
#define MAX_TF_NUMBER		99

class ParameterFile
{

private:
    std::map<std::string, bool> m_parameter_state;
    std::map<std::string, std::string> m_parameter;
    std::vector<std::string> m_name_list;

public:
    bool getState( const std::string& key )
    {
        //return m_parameter_state[key];
        if (m_parameter_state.find(key) == m_parameter_state.end()) {
            return false;
        }
        return m_parameter_state.at(key);
    };

    int         getInt( const std::string& name );
    float       getFloat( const std::string& name );
    std::string getString( const std::string& name );
    int getSize();
    int getTfnameSize();
    std::string getTfnamePrefix(int index);
    int getTfnameNumber(int index);

public:
    bool   LoadIN( const std::string& filename );

private:
    bool isTfnameNvalc(const std::string& item);
};

#endif // PBVR__PARAMETER_FILE_H_INCLUDE


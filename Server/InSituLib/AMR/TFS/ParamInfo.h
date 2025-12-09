#ifndef  __PARAM_INFO_H__
#define __PARAM_INFO_H__

#include <string>
#include <vector>
#include "Serializer.h"

class ParamInfo
{

private:
    std::map<std::string, bool> param_state;
    std::map<std::string, std::string> param;
    std::vector<std::string> name_list;

public:
    bool getState( std::string key )
    {
        return ( param_state[key] );
    };

    int                getInt( std::string name );
    float              getFloat( std::string name );
    std::string        getString( std::string name );
    std::vector<int>   getTableInt( std::string name );
    std::vector<float> getTableFloat( std::string name );
    void        write( const std::string name );

public:
    bool   LoadIN( const std::string& filename );
    size_t byteSize() const;
    size_t pack( char* buf ) const;
    size_t unpack( const char* buf );
};

#endif // __PARAM_INFO_H__


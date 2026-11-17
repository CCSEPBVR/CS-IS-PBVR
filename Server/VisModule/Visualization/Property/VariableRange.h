#ifndef VISMODULE__VARIABLE_RANGE_H_INCLUDE
#define VISMODULE__VARIABLE_RANGE_H_INCLUDE

#include <map>
#include <string>

#ifdef WIN32
#undef min
#undef max
#endif

class VariableRange
{

protected:
    typedef std::map<std::string, float> range_map_t;
    range_map_t m_min;
    range_map_t m_max;

public:
    VariableRange();
    virtual ~VariableRange();

public:
    void setValue( const std::string& tfname, const float val );

    float min( const std::string& tfname ) const;
    float max( const std::string& tfname ) const;

    void clear();
    void merge( const VariableRange& vr );

    std::size_t byteSize() const;
    std::size_t pack( char* buf ) const;
    std::size_t unpack( const char* buf );

    void show();
};

#endif // VISMODULE__VARIABLE_RANGE_H_INCLUDE


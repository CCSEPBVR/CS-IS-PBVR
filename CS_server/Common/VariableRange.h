#ifndef __VARIABLE_RANGE_H__
#define __VARIABLE_RANGE_H__

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

    size_t byteSize() const;
    size_t pack( char* buf ) const;
    size_t unpack( const char* buf );
};

#endif // __VARIABLE_RANGE_H__


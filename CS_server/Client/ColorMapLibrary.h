#ifndef PBVR__KVS__VISCLIENT__COLOR_MAP_LIBRARY_H_INCLUDE
#define PBVR__KVS__VISCLIENT__COLOR_MAP_LIBRARY_H_INCLUDE

#include <map>
#include <vector>
#include <kvs/ColorMap>

namespace kvs
{
namespace visclient
{

class ColorMapLibrary : public std::vector<std::string>
{

protected:
    size_t m_resolution;
    std::map<std::string, kvs::ColorMap> m_map;

public:
    ColorMapLibrary( const size_t resolution );
    virtual ~ColorMapLibrary();

public:
    void add( const std::string& name, const kvs::ColorMap& color_map );

    const std::string getNameByIndex( const size_t n ) const;
    const kvs::ColorMap& getColorMapByIndex( const size_t n ) const;
    const size_t getIndexByName( const std::string& name ) const;
    const kvs::ColorMap& getColorMapByName( const std::string& name ) const;
};

}
}

#endif // PBVR__KVS__VISCLIENT__COLOR_MAP_LIBRARY_H_INCLUDE


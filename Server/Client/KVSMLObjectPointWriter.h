#ifndef PBVR__KVS_VISCLIENT__KVSML_OBJECT_POINT_WRITER_H_INCLUDE
#define PBVR__KVS_VISCLIENT__KVSML_OBJECT_POINT_WRITER_H_INCLUDE

#include <string>
#include "kvs/PointObject"

namespace kvs
{
namespace visclient
{

class KVSMLObjectPointWriter
{
private:

    const kvs::PointObject* m_object;
    std::string m_basename;

public:

    KVSMLObjectPointWriter( const kvs::PointObject& object, const std::string& basename );

private:

    void write_main_tag();
    void write_coords();
    void write_colors();
    void write_normals();
};

}; // namespace visclient
}; // namespace kvs

#endif //  PBVR__KVS_VISCLIENT__KVSML_OBJECT_POINT_WRITER_H_INCLUDE

#ifndef PBVR__KVSML_OBJECT_POINT_WRITER_H_INCLUDE
#define PBVR__KVSML_OBJECT_POINT_WRITER_H_INCLUDE

#include "PointObject.h"

class KVSMLObjectPointWriter
{
private:

    const pbvr::PointObject* m_object;
    std::string m_basename;

public:

    KVSMLObjectPointWriter( const pbvr::PointObject& object, const std::string& basename );

private:

    void write_main_tag();
    void write_coords();
    void write_colors();
    void write_normals();
};

#endif //  PBVR__KVSML_OBJECT_POINT_WRITER_H_INCLUDE

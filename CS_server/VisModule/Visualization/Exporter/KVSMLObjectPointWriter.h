#ifndef VIS_MODULE__KVSML_OBJECT_POINT_WRITER_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_POINT_WRITER_H_INCLUDE

#include <vismodule/PointObject>

class KVSMLObjectPointWriter
{
private:

    const vismodule::PointObject* m_object;
    std::string m_basename;

public:

    KVSMLObjectPointWriter( const vismodule::PointObject& object, const std::string& basename );

private:

    void write_main_tag();
    void write_coords();
    void write_colors();
    void write_normals();
};

#endif //  VIS_MODULE__KVSML_OBJECT_POINT_WRITER_H_INCLUDE

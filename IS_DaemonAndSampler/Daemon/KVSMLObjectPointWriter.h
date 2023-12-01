#ifndef KVSML_OBJECT_POINT_WRITER_H_INCLUDE
#define KVSML_OBJECT_POINT_WRITER_H_INCLUDE

#include "PointObject.h"

class KVSMLObjectPointWriter
{
private:

    pbvr::PointObject* m_object;
    std::string m_basename;

public:

    KVSMLObjectPointWriter( pbvr::PointObject* object, std::string basename );

private:

    void write_main_tag( void );
    void write_coords( void );
    void write_colors( void );
    void write_normals( void );
};

#endif //  KVSML_OBJECT_POINT_WRITER_H_INCLUDE

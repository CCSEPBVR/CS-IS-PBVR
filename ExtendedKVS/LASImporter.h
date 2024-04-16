#ifndef LASIMPORTER_H
#define LASIMPORTER_H
#include <kvs/PointObject>
#include <liblas/liblas.hpp>

class LASImporter : public kvs::PointObject
{
public:
    LASImporter( std::string filename );
    kvs::UInt8 int2byte( uint16_t value );
};

#endif // LASIMPORTER_H

#ifndef DATA_INITIALIZER_H
#define DATA_INITIALIZER_H

#include <kvs/ValueArray>
#include <kvs/Type>

class DataInitializer
{
public:
    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::Real32> directions;
    kvs::ValueArray<kvs::Real32> sizes;
    kvs::ValueArray<kvs::UInt8> colors;

    DataInitializer();

    void initialize();
};

#endif // DATA_INITIALIZER_H

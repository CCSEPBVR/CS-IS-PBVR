/*****************************************************************************/
/**
 *  @file   Tuple.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include "Object.h"

namespace kvs
{

namespace python
{

class Tuple : public kvs::python::Object
{
public:
    Tuple( const std::size_t size = 0 );
    Tuple( const kvs::python::Object& object );
    Tuple(
        const kvs::python::Object& object0,
        const kvs::python::Object& object1 );
    Tuple(
        const kvs::python::Object& object0,
        const kvs::python::Object& object1,
        const kvs::python::Object& object2 );
    Tuple(
        const kvs::python::Object& object0,
        const kvs::python::Object& object1,
        const kvs::python::Object& object2,
        const kvs::python::Object& object3 );

    void set( const std::size_t index, const kvs::python::Object& object );
    std::size_t size() const;
    kvs::python::Object operator [] ( const std::size_t index ) const;
};

} // end of namespace python

} // end of namespace kvs

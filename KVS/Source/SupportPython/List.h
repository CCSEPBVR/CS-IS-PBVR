/*****************************************************************************/
/**
 *  @file   List.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include "Object.h"


namespace kvs
{

namespace python
{

class List : public kvs::python::Object
{
public:
    static bool Check( const kvs::python::Object& object );

public:
    List( const std::size_t size = 0 );
    List( const kvs::python::Object& object );

    bool set( const std::size_t index, const kvs::python::Object& object );
    bool insert( const std::size_t index, const kvs::python::Object& object );
    bool append( const kvs::python::Object& object );
    std::size_t size() const;
    kvs::python::Object operator [] ( const std::size_t index ) const;
};

} // end of namespace python

} // end of namespace kvs

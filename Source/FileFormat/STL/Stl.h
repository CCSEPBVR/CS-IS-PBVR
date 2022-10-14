/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__STL_H_INCLUDE
#define CVT__STL_H_INCLUDE

#include "Core/FileFormat/FileFormatBase.h"
#include "kvs/PolygonObject"
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <string>

namespace cvt
{

/**
 * STL data class using a VTK reader and writer.
 */
class Stl : public kvs::FileFormatBase
{
public:
    /**
     * A base class type.
     */
    typedef kvs::FileFormatBase BaseClass;

public:
    /**
     * Check if a file extention is for STL.
     *
     * \param filename A file name.
     */
    static bool CheckExtension( const std::string& filename );

public:
    /**
     * Construct a STL data.
     *
     * \param filename A file name.
     */
    Stl( const std::string& filename ): kvs::FileFormatBase() { setFilename( filename ); }
    /**
     * Construct a STL data.
     *
     * \param filename A file name.
     */
    Stl( std::string&& filename ): kvs::FileFormatBase() { setFilename( filename ); }
    virtual ~Stl() {}

public:
    bool read( const std::string& filename );
    bool write( const std::string& filename );

public:
    /**
     * Convert and set to a KVS object.
     *
     * \param [out] object a KVS object.
     */
    void get( kvs::PolygonObject* object );
    /**
     * Convert and set to a KVS object.
     *
     * \param [out] object a KVS object.
     */
    void get( kvs::PolygonObject* object ) const;
    /**
     * Reserve a writing object.
     *
     * \param [in] object A writing object.
     */
    void set( const kvs::PolygonObject* object ) { set( *object ); }
    /**
     * Reserve a writing object.
     *
     * \param [in] object A writing object.
     */
    void set( const kvs::PolygonObject& object );
    /**
     * Reserve a writing object.
     *
     * \param [in] object A writing object.
     */
    void set( kvs::PolygonObject&& object ) { set( object ); };

private:
    vtkSmartPointer<vtkPolyData> polydata;
};
} // namespace cvt

#endif // CVT__STL_H_INCLUDE
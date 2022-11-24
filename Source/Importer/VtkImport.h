/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT_VTK_IMPORT_H_INCLUDE
#define CVT_VTK_IMPORT_H_INCLUDE

#include "kvs/PolygonObject"
#include "kvs/StructuredVolumeObject"
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>

#include "CvtMode.h"

namespace cvt
{
namespace detail
{

/**
 * Import a KVS polygon object from a VTK PolyData.
 *
 * \param [inout] polygon_object A KVS polygon object.
 * \param [in] data A VTK PolyData.
 */
void ImportPolygonObject( kvs::PolygonObject* polygon_object, vtkSmartPointer<vtkPolyData> data );
/**
 * Import a KVS rectilinear object from a VTK RectilinearGrid.
 *
 * \param [inout] rectilinear_object A KVS rectilinear object.
 * \param [in] data A VTK RectilinearGrid.
 */
void ImportRectilinearObject( kvs::StructuredVolumeObject* rectilinear_object,
                              vtkSmartPointer<vtkRectilinearGrid> data );
/**
 * Import a KVS uniform structured volume object from a VTK StructuredGrid.
 *
 * This function ignores point coordinates, only refers to dimensions.
 *
 * \param [inout] uniform_object A KVS uniform structured volume object.
 * \param [in] data A VTK StructuredGrid.
 */
void ImportUniformStructuredVolumeObject( kvs::StructuredVolumeObject* uniform_object,
                                          vtkSmartPointer<vtkStructuredGrid> data );
/**
 * Import a KVS irregular structured volume object from a VTK StructuredGrid.
 *
 * \param [inout] irregular_object A KVS irregular structured volume object.
 * \param [in] data A VTK StructuredGrid.
 */
void ImportIrregularStructuredVolumeObject( kvs::StructuredVolumeObject* irregular_object,
                                            vtkSmartPointer<vtkStructuredGrid> data );
} // namespace detail
} // namespace cvt

namespace cvt
{
/**
 * Import from a VTK data to a KVS object.
 *
 * \param [inout] object A destination.
 * \param [in] data A source.
 * \param [in] mode An import mode.
 */
template <typename OutputType, typename InputType>
inline void Import( OutputType object, InputType data, int mode = 0 );

template <>
inline void Import<kvs::PolygonObject*, vtkSmartPointer<vtkPolyData>>(
    kvs::PolygonObject* polygon_object, vtkSmartPointer<vtkPolyData> data, int ignored )
{
    if ( !polygon_object )
    {
        throw std::runtime_error( "A KVS object was a null pointer." );
        return;
    }
    if ( !data )
    {
        throw std::runtime_error( "A VTK data was a null pointer." );
        return;
    }
    cvt::detail::ImportPolygonObject( polygon_object, data );

    polygon_object->setColorType( kvs::PolygonObject::PolygonColor );
    polygon_object->setColor( kvs::RGBColor( 255, 255, 255 ) );
    polygon_object->setOpacity( 255 );
}

template <>
inline void Import<kvs::StructuredVolumeObject*, vtkSmartPointer<vtkRectilinearGrid>>(
    kvs::StructuredVolumeObject* rectilinear_object, vtkSmartPointer<vtkRectilinearGrid> data,
    int ignored )
{
    if ( !rectilinear_object )
    {
        throw std::runtime_error( "A KVS object was a null pointer." );
        return;
    }
    if ( !data )
    {
        throw std::runtime_error( "A VTK data was a null pointer." );
        return;
    }
    cvt::detail::ImportRectilinearObject( rectilinear_object, data );
}

template <>
inline void Import<kvs::StructuredVolumeObject*, vtkSmartPointer<vtkStructuredGrid>>(
    kvs::StructuredVolumeObject* object, vtkSmartPointer<vtkStructuredGrid> data, int mode )
{
    if ( !object )
    {
        throw std::runtime_error( "A KVS object was a null pointer." );
        return;
    }
    if ( !data )
    {
        throw std::runtime_error( "A VTK data was a null pointer." );
        return;
    }

    if ( mode == cvt::UNIFORM_GRID_MODE )
    {
        cvt::detail::ImportUniformStructuredVolumeObject( object, data );
    }
    else
    {
        cvt::detail::ImportIrregularStructuredVolumeObject( object, data );
    }
}
} // namespace cvt

#endif // CVT_VTK_IMPORT_H_INCLUDE

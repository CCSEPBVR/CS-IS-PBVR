/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT__FILE_FORMAT_DETAIL_H_INCLUDE
#define CVT__FILE_FORMAT_DETAIL_H_INCLUDE
#include <stdexcept>

#include "kvs/PolygonObject"
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace cvt
{
namespace detail
{

/**
 * Convert to a KVS polygon object from a VTK PolyData.
 *
 * \param [inout] polygon_object A KVS polygon object.
 * \param [in] data A VTK PolyData.
 */
void ConvertToPolygonObject( kvs::PolygonObject* polygon_object,
                             vtkSmartPointer<vtkPolyData> data );
/**
 * Convert from a KVS polygon object to a VTK PolyData.
 *
 * \param [inout] data A VTK PolyData.
 * \param [in] polygon_object A KVS polygon object.
 */
void ConvertFromPolygonObject( vtkSmartPointer<vtkPolyData>& data,
                               const kvs::PolygonObject* polygon_object );
} // namespace detail
} // namespace cvt

namespace cvt
{
namespace detail
{
/**
 * Convert to an object from the other object.
 *
 * \param [inout] dst A destination.
 * \param [in] src A source.
 */
template <typename OutputType, typename InputType>
inline void Convert( OutputType dst, InputType src );

template <>
inline void Convert<kvs::PolygonObject*, vtkSmartPointer<vtkPolyData>>(
    kvs::PolygonObject* polygon_object, vtkSmartPointer<vtkPolyData> data )
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
    cvt::detail::ConvertToPolygonObject( polygon_object, data );

    polygon_object->setColorType( kvs::PolygonObject::PolygonColor );
    polygon_object->setColor( kvs::RGBColor( 255, 255, 255 ) );
    polygon_object->setOpacity( 255 );
}

template <>
inline void Convert<std::reference_wrapper<vtkSmartPointer<vtkPolyData>>,
                    const kvs::PolygonObject*>(
    std::reference_wrapper<vtkSmartPointer<vtkPolyData>> data,
    const kvs::PolygonObject* polygon_object )
{
    if ( !polygon_object )
    {
        throw std::runtime_error( "A KVS object was a null pointer." );
        return;
    }
    cvt::detail::ConvertFromPolygonObject( data, polygon_object );
}
} // namespace detail
} // namespace cvt
#endif // CVT__FILE_FORMAT_DETAIL_H_INCLUDE

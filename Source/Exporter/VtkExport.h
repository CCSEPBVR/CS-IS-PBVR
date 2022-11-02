/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 */
#ifndef CVT_VTK_EXPORT_H_INCLUDE
#define CVT_VTK_EXPORT_H_INCLUDE
#include <stdexcept>

#include "kvs/PolygonObject"
#include "kvs/StructuredVolumeObject"
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSmartPointer.h>

namespace cvt
{
namespace detail
{

/**
 * Export from a KVS polygon object to a VTK PolyData.
 *
 * \param [inout] data A VTK PolyData.
 * \param [in] polygon_object A KVS polygon object.
 */
void ExportPolygonObject( vtkSmartPointer<vtkPolyData>& data,
                          const kvs::PolygonObject* polygon_object );
/**
 * Export from a KVS rectilinear object to a VTK RectilinearGrid.
 *
 * \param [inout] data A VTK RectilinearGrid.
 * \param [in] rectilinear_object A KVS rectilinear object.
 */
void ExportRectilinearObject( vtkSmartPointer<vtkRectilinearGrid>& data,
                              const kvs::StructuredVolumeObject* rectilinear_object );

} // namespace detail
} // namespace cvt

namespace cvt
{
namespace detail
{
/**
 * An error message.
 */
constexpr const char* const KVS_OBJECT_NULL_POINTER_MESSAGE = "A KVS object was a null pointer.";
} // namespace detail
} // namespace cvt

namespace cvt
{
/**
 * Export a VTK data to a KVS object.
 *
 * \param [inout] dst A destination.
 * \param [in] src A source.
 */
template <typename OutputType, typename InputType>
inline void Export( OutputType dst, InputType src );

template <>
inline void Export<std::reference_wrapper<vtkSmartPointer<vtkPolyData>>, const kvs::PolygonObject*>(
    std::reference_wrapper<vtkSmartPointer<vtkPolyData>> data,
    const kvs::PolygonObject* polygon_object )
{
    if ( !polygon_object )
    {
        throw std::runtime_error( cvt::detail::KVS_OBJECT_NULL_POINTER_MESSAGE );
        return;
    }
    cvt::detail::ExportPolygonObject( data, polygon_object );
}

template <>
inline void Export<std::reference_wrapper<vtkSmartPointer<vtkRectilinearGrid>>,
                   const kvs::StructuredVolumeObject*>(
    std::reference_wrapper<vtkSmartPointer<vtkRectilinearGrid>> data,
    const kvs::StructuredVolumeObject* rectilinear_object )
{
    if ( !rectilinear_object )
    {
        throw std::runtime_error( cvt::detail::KVS_OBJECT_NULL_POINTER_MESSAGE );
        return;
    }
    cvt::detail::ExportRectilinearObject( data, rectilinear_object );
}
} // namespace cvt

#endif // CVT_VTK_EXPORT_H_INCLUDE
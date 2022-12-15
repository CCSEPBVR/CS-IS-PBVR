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
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredGrid.h>

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
void ExportRectilinearGridObject( vtkSmartPointer<vtkRectilinearGrid>& data,
                                  const kvs::StructuredVolumeObject* rectilinear_object );
/**
 * Export from a KVS uniform object to a VTK ImageData.
 *
 * \param [inout] data A VTK ImageData.
 * \param [in] uniform_object A KVS uniform object.
 */
void ExportUniformGridObject( vtkSmartPointer<vtkImageData>& data,
                              const kvs::StructuredVolumeObject* uniform_object );
/**
 * Export from a KVS irregular object to a VTK StructuredGrid.
 *
 * \param [inout] data A VTK StructuredGrid.
 * \param [in] irregular_object A KVS irregular object.
 */
void ExportIrregularGridObject( vtkSmartPointer<vtkStructuredGrid>& data,
                                const kvs::StructuredVolumeObject* irregular_object );
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
    cvt::detail::ExportRectilinearGridObject( data, rectilinear_object );
}

template <>
inline void Export<std::reference_wrapper<vtkSmartPointer<vtkStructuredGrid>>,
                   const kvs::StructuredVolumeObject*>(
    std::reference_wrapper<vtkSmartPointer<vtkStructuredGrid>> data,
    const kvs::StructuredVolumeObject* object )
{
    if ( !object )
    {
        throw std::runtime_error( cvt::detail::KVS_OBJECT_NULL_POINTER_MESSAGE );
        return;
    }
    if ( object->gridType() == kvs::StructuredVolumeObject::GridType::Uniform )
    {
        throw std::runtime_error( cvt::detail::KVS_OBJECT_NULL_POINTER_MESSAGE );
        return;
    }
    else
    {
        cvt::detail::ExportIrregularGridObject( data, object );
    }
}

template <>
inline void Export<std::reference_wrapper<vtkSmartPointer<vtkImageData>>,
                   const kvs::StructuredVolumeObject*>(
    std::reference_wrapper<vtkSmartPointer<vtkImageData>> data,
    const kvs::StructuredVolumeObject* object )
{
    if ( !object )
    {
        throw std::runtime_error( cvt::detail::KVS_OBJECT_NULL_POINTER_MESSAGE );
        return;
    }
    if ( object->gridType() == kvs::StructuredVolumeObject::GridType::Uniform )
    {
        cvt::detail::ExportUniformGridObject( data, object );
    }
    else
    {
        throw std::runtime_error( cvt::detail::KVS_OBJECT_NULL_POINTER_MESSAGE );
        return;
    }
}
} // namespace cvt

#endif // CVT_VTK_EXPORT_H_INCLUDE
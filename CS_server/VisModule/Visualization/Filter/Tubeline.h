/*****************************************************************************/
/**
 *  @file   Tubeline.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tubeline.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__TUBELINE_H_INCLUDE
#define VIS_MODULE__TUBELINE_H_INCLUDE

#include <vismodule/LineObject>
#include <vismodule/PolygonObject>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/FilterBase>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Create tubeline from line object.
 */
/*===========================================================================*/
class Tubeline : public vismodule::FilterBase, public vismodule::PolygonObject
{
    // Class name.
    visModuleClassName( vismodule::Tubeline );

    // Module information.
    visModuleCategory( Filter );
    visModuleBaseClass( vismodule::FilterBase );
    visModuleSuperClass( vismodule::PolygonObject );

protected:

    size_t m_ndivisions; ///< number of divisions of circle

public:

    Tubeline( void );

    Tubeline(
        const vismodule::LineObject* object,
        const size_t ndivisions = 6 );

    virtual ~Tubeline( void );

public:

    void setNDivisions( const size_t ndivisions );

public:

    SuperClass* exec( const vismodule::ObjectBase* object );

protected:

    void filtering_strip( const vismodule::LineObject* line );

    void filtering_uniline( const vismodule::LineObject* line );

    void filtering_polyline( const vismodule::LineObject* line );

    void filtering_segment( const vismodule::LineObject* line );

protected:

    void calculate_tubes(
        std::vector<vismodule::Real32>* vertices,
        std::vector<vismodule::UInt8>* colors,
        std::vector<vismodule::UInt32>* connections,
        std::vector<vismodule::Real32>* normals,
        const vismodule::ValueArray<vismodule::Real32> line_vertices,
        const vismodule::ValueArray<vismodule::Real32> line_sizes,
        const vismodule::ValueArray<vismodule::UInt8> line_colors,
        const size_t nvertices,
        const vismodule::PolygonObject::ColorType color_type );

    void calculate_circles(
        std::vector<vismodule::Vector3f>* start_circle,
        std::vector<vismodule::Vector3f>* end_circle,
        const vismodule::Vector3f& start_postion,
        const vismodule::Vector3f& end_position,
        const float radius,
        const float pre_radius,
        const float post_radius );

    void set_vertices(
        std::vector<vismodule::Real32>* vertices,
        const std::vector<vismodule::Vector3f>& start_circle,
        const std::vector<vismodule::Vector3f>& end_circle );

    void set_colors(
        std::vector<vismodule::UInt8>* colors,
        const vismodule::RGBColor& start_color,
        const vismodule::RGBColor& end_color,
        const vismodule::PolygonObject::ColorType color_type );

    void set_connections_and_normals(
        std::vector<vismodule::UInt32>* connections,
        std::vector<vismodule::Real32>* normals,
        const std::vector<vismodule::Vector3f>& start_circle,
        const std::vector<vismodule::Vector3f>& end_circle,
        const size_t vertex_number );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TUBELINE_H_INCLUDE

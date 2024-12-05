/*****************************************************************************/
/**
 *  @file   AxisObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AxisObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__AXIS_OBJECT_H_INCLUDE
#define VIS_MODULE__AXIS_OBJECT_H_INCLUDE

#include <string>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>
#include <vismodule/LineObject>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Axis object class.
 */
/*===========================================================================*/
class AxisObject : public vismodule::LineObject
{
    // Class name.
    visModuleClassName( vismodule::AxisObject );

    // Module information.
    visModuleCategory( Object );
    visModuleSuperClass( vismodule::LineObject );

protected:

    vismodule::Vector3ui m_nsublines; ///< number of sublines for the x axis
    vismodule::Vector3f m_min_value; ///<
    vismodule::Vector3f m_max_value; ///<
    std::string m_x_tag; ///< tag for the x axis
    std::string m_y_tag; ///< tag for the y axis
    std::string m_z_tag; ///< tag for the z axis
    vismodule::RGBColor m_line_color; ///< line color
    vismodule::RGBColor m_tag_color; ///< tag color
    vismodule::Real32 m_line_width; ///< line width
    vismodule::Real32 m_subline_width; ///< subline width

public:

    AxisObject( void );

    AxisObject(
        const vismodule::ObjectBase& object,
        const vismodule::Vector3ui& nsublines = vismodule::Vector3ui( 5, 5, 5 ),
        const std::string x_tag = std::string("X"),
        const std::string y_tag = std::string("Y"),
        const std::string z_tag = std::string("Z"),
        const vismodule::RGBColor& line_color = vismodule::RGBColor( 255, 255, 255 ),
        const vismodule::RGBColor& tag_color = vismodule::RGBColor( 255, 255, 255 ),
        const vismodule::Real32 line_width = 2.0f,
        const vismodule::Real32 subline_width = 1.0f );

    virtual ~AxisObject( void );

public:

//    static AxisObject* Cast( vismodule::ObjectBase* object );

public:

    void create( const vismodule::ObjectBase& object );

    void clear( void );

public:

    void setMinValue( const vismodule::Vector3f& value );

    void setMaxValue( const vismodule::Vector3f& value );

    void setNSublines( const vismodule::Vector3ui& nsublines );

    void setXTag( const std::string& tag );

    void setYTag( const std::string& tag );

    void setZTag( const std::string& tag );

    void setLineColor( const vismodule::RGBColor& color );

    void setTagColor( const vismodule::RGBColor& color );

    void setLineWidth( const vismodule::Real32 width );

    void setSublineWidth( const vismodule::Real32 width );

public:

    const vismodule::Vector3f& minValue( void ) const;

    const vismodule::Vector3f& maxValue( void ) const;

    const vismodule::Vector3ui& nsublines( void ) const;

    const std::string& xTag( void ) const;

    const std::string& yTag( void ) const;

    const std::string& zTag( void ) const;

    const vismodule::RGBColor& lineColor( void ) const;

    const vismodule::RGBColor& tagColor( void ) const;

    const vismodule::Real32 lineWidth( void ) const;

    const vismodule::Real32 sublineWidth( void ) const;

private:

    //void AxisObject::create_principal_lines(
    void create_principal_lines(
        std::vector<vismodule::Real32>* coords,
        std::vector<vismodule::UInt32>* connections,
        std::vector<vismodule::Real32>* sizes );

    //void AxisObject::create_sublines(
    void create_sublines(
        std::vector<vismodule::Real32>* coords,
        std::vector<vismodule::UInt32>* connections,
        std::vector<vismodule::Real32>* sizes );
};

} // end of namespace vismodule

#endif // VIS_MODULE__AXIS_OBJECT_H_INCLUDE

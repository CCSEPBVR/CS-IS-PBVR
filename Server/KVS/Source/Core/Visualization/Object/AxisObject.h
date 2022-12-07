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
#ifndef KVS__AXIS_OBJECT_H_INCLUDE
#define KVS__AXIS_OBJECT_H_INCLUDE

#include <string>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/LineObject>
#include <kvs/ClassName>
#include <kvs/Module>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Axis object class.
 */
/*===========================================================================*/
class AxisObject : public kvs::LineObject
{
    // Class name.
    kvsClassName( kvs::AxisObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleSuperClass( kvs::LineObject );

protected:

    kvs::Vector3ui m_nsublines; ///< number of sublines for the x axis
    kvs::Vector3f m_min_value; ///<
    kvs::Vector3f m_max_value; ///<
    std::string m_x_tag; ///< tag for the x axis
    std::string m_y_tag; ///< tag for the y axis
    std::string m_z_tag; ///< tag for the z axis
    kvs::RGBColor m_line_color; ///< line color
    kvs::RGBColor m_tag_color; ///< tag color
    kvs::Real32 m_line_width; ///< line width
    kvs::Real32 m_subline_width; ///< subline width

public:

    AxisObject( void );

    AxisObject(
        const kvs::ObjectBase* object,
        const kvs::Vector3ui& nsublines = kvs::Vector3ui( 5, 5, 5 ),
        const std::string x_tag = std::string("X"),
        const std::string y_tag = std::string("Y"),
        const std::string z_tag = std::string("Z"),
        const kvs::RGBColor& line_color = kvs::RGBColor( 255, 255, 255 ),
        const kvs::RGBColor& tag_color = kvs::RGBColor( 255, 255, 255 ),
        const kvs::Real32 line_width = 2.0f,
        const kvs::Real32 subline_width = 1.0f );

    virtual ~AxisObject( void );

public:

//    static AxisObject* Cast( kvs::ObjectBase* object );

public:

    void create( const kvs::ObjectBase* object );

    void clear( void );

public:

    void setMinValue( const kvs::Vector3f& value );

    void setMaxValue( const kvs::Vector3f& value );

    void setNSublines( const kvs::Vector3ui& nsublines );

    void setXTag( const std::string& tag );

    void setYTag( const std::string& tag );

    void setZTag( const std::string& tag );

    void setLineColor( const kvs::RGBColor& color );

    void setTagColor( const kvs::RGBColor& color );

    void setLineWidth( const kvs::Real32 width );

    void setSublineWidth( const kvs::Real32 width );

public:

    const kvs::Vector3f& minValue( void ) const;

    const kvs::Vector3f& maxValue( void ) const;

    const kvs::Vector3ui& nsublines( void ) const;

    const std::string& xTag( void ) const;

    const std::string& yTag( void ) const;

    const std::string& zTag( void ) const;

    const kvs::RGBColor& lineColor( void ) const;

    const kvs::RGBColor& tagColor( void ) const;

    const kvs::Real32 lineWidth( void ) const;

    const kvs::Real32 sublineWidth( void ) const;

private:

    //void AxisObject::create_principal_lines(
    void create_principal_lines(
        std::vector<kvs::Real32>* coords,
        std::vector<kvs::UInt32>* connections,
        std::vector<kvs::Real32>* sizes );

    //void AxisObject::create_sublines(
    void create_sublines(
        std::vector<kvs::Real32>* coords,
        std::vector<kvs::UInt32>* connections,
        std::vector<kvs::Real32>* sizes );
};

} // end of namespace kvs

#endif // KVS__AXIS_OBJECT_H_INCLUDE

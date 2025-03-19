/****************************************************************************/
/**
 *  @file Light.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Light.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__LIGHT_H_INCLUDE
#define VIS_MODULE__LIGHT_H_INCLUDE

#include <vismodule/XformControl>
#include <vismodule/ClassName>
#include <vismodule/Vector3>
#include <vismodule/Camera>


namespace vismodule
{

class RGBAColor;

/*==========================================================================*/
/**
 *  Light class
 */
/*==========================================================================*/
class Light : public vismodule::XformControl
{
    visModuleClassName( vismodule::Light );

protected:

    unsigned int  m_id;            ///< light ID
    vismodule::Vector3f m_init_position; ///< initial light position
    vismodule::Vector3f m_position;      ///< light position
    vismodule::Vector3f m_diffuse;       ///< diffuse color
    vismodule::Vector3f m_ambient;       ///< ambient color
    vismodule::Vector3f m_specular;      ///< specular color

public:

    Light( bool collision = true );

    virtual ~Light( void );

public:

    void initialize( void );

public:

    void setID( const unsigned int id );

    void setPosition( const float x, const float y, const float z );

    void setPosition( const vismodule::Vector3f& position );

    void setColor( const float r, const float g, const float b );

    void setColor( const vismodule::RGBAColor& color );

    void setDiffuse( const float r, const float g, const float b );

    void setDiffuse( const vismodule::RGBAColor& color );

    void setAmbient( const float r, const float g, const float b );

    void setAmbient( const vismodule::RGBAColor& color );

    void setSpecular( const float r, const float g, const float b );

    void setSpecular( const vismodule::RGBAColor& color );

public:

    const vismodule::Vector3f& position( void ) const;

    const vismodule::Vector3f& diffuse( void ) const;

    const vismodule::Vector3f& ambient( void ) const;

    const vismodule::Vector3f& specular( void ) const;

public:

    virtual void update( const vismodule::Camera* camera );

    void on( void ) const;

    void off( void ) const;

    const bool isEnabled( void ) const;

public:

    void resetXform( void );

    void rotate( const vismodule::Matrix33f& rotation );

    void translate( const vismodule::Vector3f& translation );

    void scale( const vismodule::Vector3f& scaling );

private:

    void update_position( void );

public:

    static void setModelLocalViewer( bool flag );

    static void setModelTwoSide( bool flag );

    static void setModelAmbient( float ambient[4] );
};

} // end of namespace vismodule

#endif // VIS_MODULE__LIGHT_H_INCLUDE

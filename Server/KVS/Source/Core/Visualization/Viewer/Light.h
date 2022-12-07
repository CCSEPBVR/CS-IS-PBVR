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
#ifndef KVS__LIGHT_H_INCLUDE
#define KVS__LIGHT_H_INCLUDE

#include <kvs/XformControl>
#include <kvs/ClassName>
#include <kvs/Vector3>
#include <kvs/Camera>


namespace kvs
{

class RGBAColor;

/*==========================================================================*/
/**
 *  Light class
 */
/*==========================================================================*/
class Light : public kvs::XformControl
{
    kvsClassName( kvs::Light );

protected:

    unsigned int  m_id;            ///< light ID
    kvs::Vector3f m_init_position; ///< initial light position
    kvs::Vector3f m_position;      ///< light position
    kvs::Vector3f m_diffuse;       ///< diffuse color
    kvs::Vector3f m_ambient;       ///< ambient color
    kvs::Vector3f m_specular;      ///< specular color

public:

    Light( bool collision = true );

    virtual ~Light( void );

public:

    void initialize( void );

public:

    void setID( const unsigned int id );

    void setPosition( const float x, const float y, const float z );

    void setPosition( const kvs::Vector3f& position );

    void setColor( const float r, const float g, const float b );

    void setColor( const kvs::RGBAColor& color );

    void setDiffuse( const float r, const float g, const float b );

    void setDiffuse( const kvs::RGBAColor& color );

    void setAmbient( const float r, const float g, const float b );

    void setAmbient( const kvs::RGBAColor& color );

    void setSpecular( const float r, const float g, const float b );

    void setSpecular( const kvs::RGBAColor& color );

public:

    const kvs::Vector3f& position( void ) const;

    const kvs::Vector3f& diffuse( void ) const;

    const kvs::Vector3f& ambient( void ) const;

    const kvs::Vector3f& specular( void ) const;

public:

    virtual void update( const kvs::Camera* camera );

    void on( void ) const;

    void off( void ) const;

    const bool isEnabled( void ) const;

public:

    void resetXform( void );

    void rotate( const kvs::Matrix33f& rotation );

    void translate( const kvs::Vector3f& translation );

    void scale( const kvs::Vector3f& scaling );

private:

    void update_position( void );

public:

    static void setModelLocalViewer( bool flag );

    static void setModelTwoSide( bool flag );

    static void setModelAmbient( float ambient[4] );
};

} // end of namespace kvs

#endif // KVS__LIGHT_H_INCLUDE

/****************************************************************************/
/**
 *  @file XformControl.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XformControl.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__XFORM_CONTROL_H_INCLUDE
#define VIS_MODULE__XFORM_CONTROL_H_INCLUDE

#include <vismodule/Xform>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Xform control class.
 */
/*==========================================================================*/
class XformControl : public vismodule::Xform
{
    visModuleClassName( vismodule::XformControl );

public:

    typedef vismodule::Xform parent_class;

protected:

    vismodule::Xform m_initial_xform; ///< initial transform vector
    bool       m_can_collision; ///< enable collision detection or not

public:

    XformControl( bool collision = true );

    XformControl(
        const vismodule::Vector3f&  translation,
        const vismodule::Vector3f&  scale,
        const vismodule::Matrix33f& rotation,
        bool                  collision = true );

    virtual ~XformControl( void );

public:

    void enableCollision( void );

    void disableCollision( void );

    bool canCollision( void );

    void setInitialXform(
        const vismodule::Vector3f&  translation = vismodule::Vector3f(0,0,0),
        const vismodule::Vector3f&  scaling     = vismodule::Vector3f(1,1,1),
        const vismodule::Matrix33f& rotation    = vismodule::Matrix33f(1,0,0,0,1,0,0,0,1) );

    void saveXform( void );

    void resetXform( void );

    void multiplyXform( const vismodule::Xform& xform );

    void setXform( const vismodule::Xform& xform );

    void applyXform( void ) const;

    const vismodule::Xform xform( void ) const;

public:

    void rotate( const vismodule::Matrix33f& rotation );

    void translate( const vismodule::Vector3f& translation );

    void scale( const vismodule::Vector3f& scaling );
};

} // end of namespace vismodule

#endif // VIS_MODULE__XFORM_CONTROL_H_INCLUDE

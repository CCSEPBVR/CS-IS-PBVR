/****************************************************************************/
/**
 *  @file XformControl.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: XformControl.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "XformControl.h"
#include <vismodule/OpenGL>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Constructor.
 *  @param collision [in] conllision flag
 */
/*==========================================================================*/
XformControl::XformControl( bool collision ) :
    vismodule::Xform(),
    m_can_collision( collision )
{
    m_initial_xform.initialize();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param translation [in] translation vector
 *  @param scale [in] scaling parameters
 *  @param rotation [in] rotation matrix
 *  @param collision [in] collision flag
 */
/*==========================================================================*/
XformControl::XformControl(
    const vismodule::Vector3f&  translation,
    const vismodule::Vector3f&  scale,
    const vismodule::Matrix33f& rotation,
    bool                  collision ):
    vismodule::Xform( translation, scale, rotation ),
    m_can_collision( collision )
{
    this->saveXform();
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
XformControl::~XformControl( void )
{
}

/*==========================================================================*/
/**
 *  Enable collision detection.
 */
/*==========================================================================*/
void XformControl::enableCollision( void )
{
    m_can_collision = true;
}

/*==========================================================================*/
/**
 *  Disable collision detection.
 */
/*==========================================================================*/
void XformControl::disableCollision( void )
{
    m_can_collision = false;
}

/*==========================================================================*/
/**
 *  Test whether the collision is detected.
 */
/*==========================================================================*/
bool XformControl::canCollision( void )
{
    return( m_can_collision );
}

/*==========================================================================*/
/**
 *  Set initial xform.
 *  @param translation [in] translation vector
 *  @param scale [in] scaling parameters
 *  @param rotation [in] rotation matrix
 */
/*==========================================================================*/
void XformControl::setInitialXform(
    const vismodule::Vector3f&  translation,
    const vismodule::Vector3f&  scale,
    const vismodule::Matrix33f& rotation )
{
    parent_class::set( translation, scale, rotation );
    this->saveXform();
}

/*==========================================================================*/
/**
 *  Save xform.
 */
/*==========================================================================*/
void XformControl::saveXform( void )
{
    m_initial_xform.set( *this );
}

/*==========================================================================*/
/**
 *  Reset xform.
 */
/*==========================================================================*/
void XformControl::resetXform( void )
{
    parent_class::set( m_initial_xform );
}

/*==========================================================================*/
/**
 *  Multiply xform.
 *  @param xform [in] xform matrix
 */
/*==========================================================================*/
void XformControl::multiplyXform( const vismodule::Xform& xform )
{
    parent_class::set( xform * (*this) );
}

/*==========================================================================*/
/**
 *  Set xform.
 *  @param xform [in] xform matrix.
 */
/*==========================================================================*/
void XformControl::setXform( const vismodule::Xform& xform )
{
    parent_class::set( xform );
}

/*==========================================================================*/
/**
 *  Apply xform matrix.
 */
/*==========================================================================*/
void XformControl::applyXform( void ) const
{
#ifndef NO_CLIENT
    float xform[16];
    parent_class::get( &xform );
    glMultMatrixf( xform );
#endif
}
/*==========================================================================*/
/**
 *  Get xform.
 *  @return xform matrix
 */
/*==========================================================================*/
const vismodule::Xform XformControl::xform( void ) const
{
    return( parent_class::get() );
}

/*==========================================================================*/
/**
 *  Rotate.
 *  @param rotation [in] current rotation matrix.
 */
/*==========================================================================*/
void XformControl::rotate( const vismodule::Matrix33f& rotation )
{
    vismodule::Vector3f position( (*this)[0][3], (*this)[1][3], (*this)[2][3] );

    this->translate( -position );
    parent_class::updateRotation( rotation );
    this->translate( position );
}

/*==========================================================================*/
/**
 *  Translate.
 *  @param translation [in] current translation vector.
 */
/*==========================================================================*/
void XformControl::translate( const vismodule::Vector3f& translation )
{
    parent_class::updateTranslation( translation );
}

/*==========================================================================*/
/**
 *  Scaling.
 *  @param scale [in] current scaling value.
 */
/*==========================================================================*/
void XformControl::scale( const vismodule::Vector3f& scaling )
{
    vismodule::Vector3f position( (*this)[0][3], (*this)[1][3], (*this)[2][3] );

    this->translate( -position );
    parent_class::updateScaling( scaling );
    this->translate( position );
}

} // end of namespace vismodule

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
#ifndef KVS__XFORM_CONTROL_H_INCLUDE
#define KVS__XFORM_CONTROL_H_INCLUDE

#include <kvs/Xform>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Xform control class.
 */
/*==========================================================================*/
class XformControl : public kvs::Xform
{
    kvsClassName( kvs::XformControl );

public:

    typedef kvs::Xform parent_class;

protected:

    kvs::Xform m_initial_xform; ///< initial transform vector
    bool       m_can_collision; ///< enable collision detection or not

public:

    XformControl( bool collision = true );

    XformControl(
        const kvs::Vector3f&  translation,
        const kvs::Vector3f&  scale,
        const kvs::Matrix33f& rotation,
        bool                  collision = true );

    virtual ~XformControl( void );

public:

    void enableCollision( void );

    void disableCollision( void );

    bool canCollision( void );

    void setInitialXform(
        const kvs::Vector3f&  translation = kvs::Vector3f(0,0,0),
        const kvs::Vector3f&  scaling     = kvs::Vector3f(1,1,1),
        const kvs::Matrix33f& rotation    = kvs::Matrix33f(1,0,0,0,1,0,0,0,1) );

    void saveXform( void );

    void resetXform( void );

    void multiplyXform( const kvs::Xform& xform );

    void setXform( const kvs::Xform& xform );

    void applyXform( void ) const;

    const kvs::Xform xform( void ) const;

public:

    void rotate( const kvs::Matrix33f& rotation );

    void translate( const kvs::Vector3f& translation );

    void scale( const kvs::Vector3f& scaling );
};

} // end of namespace kvs

#endif // KVS__XFORM_CONTROL_H_INCLUDE

/****************************************************************************/
/**
 *  @file Xform.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Xform.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__XFORM_H_INCLUDE
#define KVS__XFORM_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Vector3>
#include <kvs/Matrix33>
#include <kvs/Matrix44>


namespace kvs
{

/*==========================================================================*/
/**
 *  Xform matrix class.
 *
 *  This xform matrix \f${\bf X}\f$ is a affine transformation matrix as follows:
 *
 *  \f[
 *  {\bf X} = \left[ \begin{array}{cc}
 *                   {\bf SR}    & {\bf T} \\
 *                   {\bf 0}^{t} & 1
 *                   \end{array}
 *            \right]
 *  \f]
 *
 * where, \f${\bf S}\f$ is a scaling matrix which is composed of three scaling
 * parameters \f${\bf s}=[s_{x},s_{y},s_{z}]^{t}\f$, \f${\bf T}\f$ is a translation
 * vector, and \f${\bf R}\f$ is a rotation matrix.
 *
 *  \f[
 *
 *  \begin{array}{ccl}
 *
 *  {\bf S} & = & diag({\bf s}) = \left[ \begin{array}{ccc}
 *                                       s_{x} & 0     & 0     \\
 *                                       0     & s_{y} & 0     \\
 *                                       0     & 0     & s_{z}
 *                                       \end{array}
 *                                \right]
 *
 *  \\
 *
 *  {\bf T} & = & [t_{x},t_{y},t_{z}]^{t}
 *
 *  \\
 *
 *  {\bf R} & = & \left[ \begin{array}{ccc}
 *                       r_{00} & r_{01} & r_{02} \\
 *                       r_{10} & r_{11} & r_{12} \\
 *                       r_{20} & r_{21} & r_{22}
 *                       \end{array}
 *                \right]
 *
 *  \end{array}
 *
 *  \f]
 */
/*==========================================================================*/
class Xform : public kvs::Matrix44f
{
    kvsClassName( kvs::Xform );

protected:

    kvs::Matrix33f m_rotation; ///< rotation matrix
    kvs::Vector3f  m_scaling;  ///< scaling vector

public:

    Xform( void );

    Xform(
        const kvs::Vector3f&  translation,
        const kvs::Vector3f&  scaling,
        const kvs::Matrix33f& rotation );

    Xform( const Xform& xform );

    Xform( const kvs::Matrix44f& m );

    virtual ~Xform( void );

public:

    Xform& operator = ( const kvs::Matrix44f& m );

    Xform& operator = ( const Xform& xform );

public:

    void initialize( void );

    void clear( void );

    void set(
        const kvs::Vector3f&  translation,
        const kvs::Vector3f&  scaling,
        const kvs::Matrix33f& rotation );

    void set( const Xform& xform );

public:

    void updateRotation( const kvs::Matrix33f& rotation );

    void updateTranslation( const kvs::Vector3f& translation );

    void updateScaling( const kvs::Vector3f& scaling );

    void updateScaling( float scaling );

    const kvs::Vector3f translation( void ) const;

    const kvs::Matrix33f& rotation( void ) const;

    const kvs::Matrix33f scaledRotation( void ) const;

    const kvs::Vector3f& scaling( void ) const;

    Xform get( void ) const;

    void get( float (*array)[16] ) const;
};

} // end of namespace kvs

#endif // KVS__XFORM_H_INCLUDE

/****************************************************************************/
/**
 *  @file Xform.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Xform.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Xform.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Default constructor.
 */
/*==========================================================================*/
Xform::Xform( void )
{
    this->initialize();
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param translation [in] translation vector
 *  @param scaling [in] scaling vector
 *  @param rotation [in] rotation matrix
 */
/*==========================================================================*/
Xform::Xform(
    const kvs::Vector3f&  translation,
    const kvs::Vector3f&  scaling,
    const kvs::Matrix33f& rotation )
{
    this->set( translation, scaling, rotation );
}

/*==========================================================================*/
/**
 *  Copy constructor.
 *  @param xform [in] xform matrix
 */
/*==========================================================================*/
Xform::Xform( const Xform& xform )
{
    *this = xform;
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param m [in] xform matrix
 */
/*==========================================================================*/
Xform::Xform( const kvs::Matrix44f& m )
{
    *this = m;
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
Xform::~Xform( void )
{
    this->clear();
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param m [in] xform matrix
 *  @return xform
 */
/*==========================================================================*/
Xform& Xform::operator = ( const kvs::Matrix44f& m )
{
    (*this)[0] = m[0];
    (*this)[1] = m[1];
    (*this)[2] = m[2];
    (*this)[3] = m[3];

    return( *this );
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param xform [in] xform
 *  @return xform
 */
/*==========================================================================*/
Xform& Xform::operator = ( const Xform& xform )
{
    *this = kvs::Matrix44f( xform );

    m_rotation = xform.m_rotation;
    m_scaling  = xform.m_scaling;

    return( *this );
}

/*==========================================================================*/
/**
 *  Initialize the xform.
 */
/*==========================================================================*/
void Xform::initialize( void )
{
    kvs::Matrix44f::identity();

    m_rotation.identity();
    m_scaling.set( 1.0, 1.0, 1.0 );
}

/*==========================================================================*/
/**
 *  Clear the xform matrix.
 */
/*==========================================================================*/
void Xform::clear( void )
{
    kvs::Matrix44f::zero();

    m_rotation.zero();
    m_scaling.zero();
}

/*==========================================================================*/
/**
 *  Set the transformation parameters.
 *  @param translation [in] translation vector
 *  @param scaling [in] scaling vector
 *  @param rotation [in] rotation matrix
 *
 *  Calculate the xform matrix \f${\bf X}\f$ by using the specified
 *  transformation parameters, which are the translation vector \f${\bf T}\f$,
 *  the scaling values \f${\bf s}=[s_{x},s_{y},s_{z}]^{t}\f$ and the rotation
 *  matrix \f${\bf R}\f$ as follows:
 *
 *  \f[
 *  {\bf X} = \left[ \begin{array}{cc}
 *                   diag({\bf s}) {\bf R}    & {\bf T} \\
 *                   {\bf 0}^{t}              & 1
 *                   \end{array}
 *            \right]
 *          = \left[ \begin{array}{cccc}
 *                   s_{x} r_{00} & s_{x} r_{01} & s_{x} r_{02} & t_{x} \\
 *                   s_{x} r_{10} & s_{x} r_{11} & s_{y} r_{12} & t_{y} \\
 *                   s_{x} r_{20} & s_{x} r_{21} & s_{z} r_{22} & t_{z} \\
 *                   0            & 0            & 0            & 1     \\
 *                   \end{array}
 *            \right]
 *  \f]
 */
/*==========================================================================*/
void Xform::set(
    const kvs::Vector3f&  translation,
    const kvs::Vector3f&  scaling,
    const kvs::Matrix33f& rotation )
{
    this->initialize();
    this->updateRotation( rotation );
    this->updateScaling( scaling );
    this->updateTranslation( translation );
}

/*==========================================================================*/
/**
 *  Set the xform.
 *  @param xform [in] xform matrix
 */
/*==========================================================================*/
void Xform::set( const Xform& xform )
{
    *this = xform;
}

/*==========================================================================*/
/**
 *  Update the rotation parameter.
 *  @param rotation [in] rotation matrix
 */
/*==========================================================================*/
void Xform::updateRotation( const kvs::Matrix33f& rotation )
{
    const kvs::Matrix44f rotation44(
        rotation[0][0], rotation[0][1], rotation[0][2], 0,
        rotation[1][0], rotation[1][1], rotation[1][2], 0,
        rotation[2][0], rotation[2][1], rotation[2][2], 0,
        0,              0,              0,              1 );

    *this = rotation44 * kvs::Matrix44f( *this );

    m_rotation = rotation * m_rotation;
}

/*==========================================================================*/
/**
 *  Update the translation parameter.
 *  @param translation [in] translation vector
 */
/*==========================================================================*/
void Xform::updateTranslation( const kvs::Vector3f& translation )
{
    (*this)[0][3] += translation.x();
    (*this)[1][3] += translation.y();
    (*this)[2][3] += translation.z();
}

/*==========================================================================*/
/**
 *  Update the scaling parameters.
 *  @param scaling [in] scaling vector
 */
/*==========================================================================*/
void Xform::updateScaling( const kvs::Vector3f& scaling )
{
    (*this)[0][0] *= scaling.x(); (*this)[0][1] *= scaling.x();
    (*this)[0][2] *= scaling.x(); (*this)[0][3] *= scaling.x();

    (*this)[1][0] *= scaling.y(); (*this)[1][1] *= scaling.y();
    (*this)[1][2] *= scaling.y(); (*this)[1][3] *= scaling.y();

    (*this)[2][0] *= scaling.z(); (*this)[2][1] *= scaling.z();
    (*this)[2][2] *= scaling.z(); (*this)[2][3] *= scaling.z();

    m_scaling.x() *= scaling.x();
    m_scaling.y() *= scaling.y();
    m_scaling.z() *= scaling.z();
}

/*==========================================================================*/
/**
 *  Update the scaling parameters.
 *  @param scaling [in] scaling vector
 */
/*==========================================================================*/
void Xform::updateScaling( float scaling )
{
    (*this)[0][0] *= scaling; (*this)[0][1] *= scaling;
    (*this)[0][2] *= scaling; (*this)[0][3] *= scaling;

    (*this)[1][0] *= scaling; (*this)[1][1] *= scaling;
    (*this)[1][2] *= scaling; (*this)[1][3] *= scaling;

    (*this)[2][0] *= scaling; (*this)[2][1] *= scaling;
    (*this)[2][2] *= scaling; (*this)[2][3] *= scaling;

    m_scaling *= scaling;
}

/*==========================================================================*/
/**
 *  Get the translation vector.
 *  @return translation vector
 */
/*==========================================================================*/
const kvs::Vector3f Xform::translation( void ) const
{
    return( kvs::Vector3f( (*this)[0][3], (*this)[1][3], (*this)[2][3] ) );
}

/*==========================================================================*/
/**
 *  Get the rotation matrix.
 *  @return rotation matrix
 */
/*==========================================================================*/
const kvs::Matrix33f& Xform::rotation( void ) const
{
    return( m_rotation );
}

/*==========================================================================*/
/**
 *  Get the scaled rotation matrix.
 *  @return scaled rotation matrix
 */
/*==========================================================================*/
const kvs::Matrix33f Xform::scaledRotation( void ) const
{
    return( kvs::Matrix33f( (*this)[0][0], (*this)[1][0], (*this)[2][0],
                            (*this)[0][1], (*this)[1][1], (*this)[2][1],
                            (*this)[0][2], (*this)[1][2], (*this)[2][2] ) );
}

/*==========================================================================*/
/**
 *  Get the scale vector.
 *  @return rotation matrix
 */
/*==========================================================================*/
const kvs::Vector3f& Xform::scaling( void ) const
{
    return( m_scaling );
}

/*==========================================================================*/
/**
 *  Get the xform.
 *  @return xform
 */
/*==========================================================================*/
Xform Xform::get( void ) const
{
    return( *this );
}

/*==========================================================================*/
/**
 *  Get the xform as array type.
 *  @param  array [i/o] xform array
 */
/*==========================================================================*/
void Xform::get( float (*array)[16] ) const
{
    (*array)[0]  = (*this)[0][0];
    (*array)[1]  = (*this)[1][0];
    (*array)[2]  = (*this)[2][0];
    (*array)[3]  = (*this)[3][0];

    (*array)[4]  = (*this)[0][1];
    (*array)[5]  = (*this)[1][1];
    (*array)[6]  = (*this)[2][1];
    (*array)[7]  = (*this)[3][1];

    (*array)[8]  = (*this)[0][2];
    (*array)[9]  = (*this)[1][2];
    (*array)[10] = (*this)[2][2];
    (*array)[11] = (*this)[3][2];

    (*array)[12] = (*this)[0][3];
    (*array)[13] = (*this)[1][3];
    (*array)[14] = (*this)[2][3];
    (*array)[15] = (*this)[3][3];
}

} // end of namepsace kvs

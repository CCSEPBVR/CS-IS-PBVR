/*****************************************************************************/
/**
 *  @file   QRDecomposer.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QRDecomposer.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__QR_DECOMPOSER_H_INCLUDE
#define VIS_MODULE__QR_DECOMPOSER_H_INCLUDE

#include <vismodule/Matrix33>
#include <vismodule/Matrix44>
#include <vismodule/Matrix>
#include <vismodule/Vector>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  QR decomposition class.
 */
/*===========================================================================*/
template <typename T>
class QRDecomposer
{
    visModuleClassName_without_virtual( vismodule::QRDecomposer );

protected:

    vismodule::Matrix<T> m_m;  ///< original matrix
    vismodule::Matrix<T> m_qt; ///< Transpose matrix of Q matrix
    vismodule::Matrix<T> m_r;  ///< R matrix (upper triangle matrix)

public:

    QRDecomposer( void );

    QRDecomposer( const vismodule::Matrix33<T>& m );

    QRDecomposer( const vismodule::Matrix44<T>& m );

    QRDecomposer( const vismodule::Matrix<T>& m );

public:

    const vismodule::Matrix<T> Q( void ) const;

    const vismodule::Matrix<T>& R( void ) const;

    const vismodule::Matrix<T>& Qt( void ) const;

public:

    QRDecomposer& operator = ( const QRDecomposer& q );

public:

    void setMatrix( const vismodule::Matrix33<T>& m );

    void setMatrix( const vismodule::Matrix44<T>& m );

    void setMatrix( const vismodule::Matrix<T>& m );

    void decompose( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__QR_DECOMPOSER_H_INCLUDE

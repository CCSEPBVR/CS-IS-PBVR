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
#ifndef KVS__QR_DECOMPOSER_H_INCLUDE
#define KVS__QR_DECOMPOSER_H_INCLUDE

#include <kvs/Matrix33>
#include <kvs/Matrix44>
#include <kvs/Matrix>
#include <kvs/Vector>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  QR decomposition class.
 */
/*===========================================================================*/
template <typename T>
class QRDecomposer
{
    kvsClassName_without_virtual( kvs::QRDecomposer );

protected:

    kvs::Matrix<T> m_m;  ///< original matrix
    kvs::Matrix<T> m_qt; ///< Transpose matrix of Q matrix
    kvs::Matrix<T> m_r;  ///< R matrix (upper triangle matrix)

public:

    QRDecomposer( void );

    QRDecomposer( const kvs::Matrix33<T>& m );

    QRDecomposer( const kvs::Matrix44<T>& m );

    QRDecomposer( const kvs::Matrix<T>& m );

public:

    const kvs::Matrix<T> Q( void ) const;

    const kvs::Matrix<T>& R( void ) const;

    const kvs::Matrix<T>& Qt( void ) const;

public:

    QRDecomposer& operator = ( const QRDecomposer& q );

public:

    void setMatrix( const kvs::Matrix33<T>& m );

    void setMatrix( const kvs::Matrix44<T>& m );

    void setMatrix( const kvs::Matrix<T>& m );

    void decompose( void );
};

} // end of namespace kvs

#endif // KVS__QR_DECOMPOSER_H_INCLUDE

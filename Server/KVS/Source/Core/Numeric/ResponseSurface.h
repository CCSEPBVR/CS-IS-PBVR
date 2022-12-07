/*****************************************************************************/
/**
 *  @file   ResponseSurface.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ResponseSurface.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__RESPONSE_SURFACE_H_INCLUDE
#define KVS__RESPONSE_SURFACE_H_INCLUDE

#include <string>
#include <vector>
#include <algorithm>
#include <kvs/BitArray>
#include <kvs/Assert>
#include <kvs/Vector2>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Vector>
#include <kvs/Matrix>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Response surface method class.
 */
/*===========================================================================*/
template <typename T>
class ResponseSurface : public kvs::Vector<T>
{
    kvsClassName( kvs::ResponseSurface );

protected:

    size_t m_npoints; ///< number of data points
    size_t m_nvariables; ///< number of independent variables (ex. x,y -> 2 )
    size_t m_nterms; ///< number of terms (ex. x,y -> 6 = {1, x, y, x^2, y^2, xy})
    kvs::BitArray m_mask; ///< mask table for the active coefficients
    kvs::Matrix<T> m_coefficient_matrix; ///< coeffiecinet matrix (matrix of independent variable)
    kvs::Vector<T> m_responses; ///< response values (vector of dependent variable)
    kvs::Vector<T> m_t_values; ///< t-values
    T m_r_square; ///< determination coefficient (R^2)
    T m_adjusted_r_square; ///< adjusted determination coefficient

public:

    ResponseSurface( void );

    ResponseSurface( const kvs::Matrix<T>& variables, const kvs::Vector<T>& responses );

    virtual ~ResponseSurface( void );

public:

    const kvs::Vector<T>& solve( const kvs::Matrix<T>& variables, const kvs::Vector<T>& responses );

    const kvs::Vector<T>& improve( const T threshold = T( 2.45 ) );

public:

    const size_t npoints( void ) const;

    const size_t nvariables( void ) const;

    const T Rsquare( void ) const;

    const T adjustedRsquare( void ) const;

    const kvs::Vector<T>& Tvalues( void ) const;

private:

    void solve_regression_coefficients( void );

    void create_coefficient_matrix( const kvs::Matrix<T>& variables );

    void update_coefficient_matrix( void );
};

} // end of namespace kvs

#endif // KVS__RESPONSE_SURFACE_H_INCLUDE

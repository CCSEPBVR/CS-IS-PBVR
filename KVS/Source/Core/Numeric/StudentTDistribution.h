/*****************************************************************************/
/**
 *  @file   StudentTDistribution.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/Type>
#include <kvs/ValueArray>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Student-T distribution class.
 */
/*===========================================================================*/
class StudentTDistribution
{
private:
    std::size_t m_dof; ///< degree of freedom
    kvs::Real64 m_coef; ///< coefficient

public:
    StudentTDistribution( const std::size_t dof );

    std::size_t dof() const{ return m_dof; }
    kvs::Real64 pdf( const kvs::Real64 t ) const;
    kvs::Real64 cdf( const kvs::Real64 t ) const;

    template <typename T>
    kvs::ValueArray<T> pdf( const kvs::ValueArray<T>& x ) const
    {
        const std::size_t n = x.size();
        kvs::ValueArray<T> values( n );
        for ( std::size_t i = 0; i < n; i++ ) { values[i] = this->pdf( x[i] ); }
        return values;
    }

    template <typename T>
    kvs::ValueArray<T> cdf( const kvs::ValueArray<T>& x ) const
    {
        const std::size_t n = x.size();
        kvs::ValueArray<T> values( n );
        for ( std::size_t i = 0; i < n; i++ ) { values[i] = this->cdf( x[i] ); }
        return values;
    }
};

} // end of namespace kvs

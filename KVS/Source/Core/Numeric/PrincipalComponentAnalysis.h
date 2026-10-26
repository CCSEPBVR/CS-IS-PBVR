/*****************************************************************************/
/**
 *  @file   PrincipalComponentAnalysis.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/Vector>
#include <kvs/Matrix>
#include <kvs/ValueTable>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Principal Component Analysis (PCA) class.
 */
/*===========================================================================*/
template <typename T>
class PrincipalComponentAnalysis
{
private:
    std::size_t m_ncomponents = 0; /// number of components (if 0, dimension of the input data matrix)
    kvs::Matrix<T> m_components{}; /// eigenvector matrix derived from the covariance matrix of the input data matrix
    kvs::Vector<T> m_explained_variance{}; /// eigen value vector from the covariance matrix of the input data matrix
    kvs::Vector<T> m_explained_variance_ratio{}; /// ratio of the eigen values

public:
    PrincipalComponentAnalysis( const std::size_t ncomps = 0 ): m_ncomponents( ncomps ) {}
    PrincipalComponentAnalysis( const kvs::ValueTable<T>& data, const std::size_t ncomponents = 0 );

    void setNumberOfComponents( const std::size_t ncomponents ) { m_ncomponents = ncomponents; }
    std::size_t numberOfComponents() const { return m_ncomponents; }
    const kvs::Matrix<T>& components() const { return m_components; }
    const kvs::Vector<T>& explainedVariance() const { return m_explained_variance; }
    const kvs::Vector<T>& explainedVarianceRatio() const { return m_explained_variance_ratio; }

    void fit( const kvs::ValueTable<T>& data );
    kvs::ValueTable<T> transform( const kvs::ValueTable<T>& data );
};

} // end of namespace kvs

/*****************************************************************************/
/**
 *  @file   CurvedParallelCoordinatesRenderer.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <kvs/Module>
#include <kvs/ParallelCoordinatesRenderer>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Curved parallel coordinates renderer class.
 */
/*===========================================================================*/
class CurvedParallelCoordinatesRenderer : public kvs::ParallelCoordinatesRenderer
{
    kvsModule( kvs::CurvedParallelCoordinatesRenderer, Renderer );
    kvsModuleBaseClass( kvs::ParallelCoordinatesRenderer );

private:
    std::size_t m_nintervals = 10; ///< number of intervals

public:
    CurvedParallelCoordinatesRenderer()= default;

    void setNumberOfIntervals( const std::size_t nintervals ) { m_nintervals = nintervals; }
    std::size_t numberOfIntervals() const { return m_nintervals; }

    void exec( kvs::ObjectBase* object, kvs::Camera* camera, kvs::Light* light );
};

} // end of namespace kvs

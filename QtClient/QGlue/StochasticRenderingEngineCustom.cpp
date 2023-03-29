/*****************************************************************************/
/**
 *  @file   StochasticRenderingEngineCustom.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#include "StochasticRenderingEngineCustom.h"

#include "TimeMeasureUtility.h"

#include "objnameutil.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new StochasticRenderingEngineCustom class.
 */
/*===========================================================================*/
StochasticRenderingEngineCustom::StochasticRenderingEngineCustom():
       m_random_texture_size_previous(-1)
{

}

/*===========================================================================*/
/**
 *  @brief  Create random texture if m_random_texture_size is changed.
 */
/*===========================================================================*/
void StochasticRenderingEngineCustom::createRandomTexture()
{
#ifdef SKIP_REDUNDANT_CREATE_RANDOM_TEXTURE
    if(m_random_texture_size_previous == randomTextureSize()){
        return;
    }
#endif

    StochasticRenderingEngine::createRandomTexture();
    m_random_texture_size_previous = randomTextureSize();
}


} // end of namespace kvs

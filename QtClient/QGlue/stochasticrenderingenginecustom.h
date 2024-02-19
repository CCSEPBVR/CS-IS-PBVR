/*****************************************************************************/
/**
 *  @file   stochasticrenderingenginecustom.h
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
#pragma once
#include <kvs/StochasticRenderingEngine>

/*===========================================================================*/
/**
 *  @brief  Engine class for stochastic renderer.
 */
/*===========================================================================*/
class StochasticRenderingEngineCustom : public kvs::StochasticRenderingEngine {
private:
  size_t m_random_texture_size_previous;

public:
  StochasticRenderingEngineCustom();
  void createRandomTexture();
};

/****************************************************************************/
/**
 *  @file Global.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Global.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__GLUT__GLOBAL_H_INCLUDE
#define KVS__GLUT__GLOBAL_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/glut/GlobalBase>
#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/IDManager>
#include <kvs/VisualizationPipeline>


namespace kvs
{

namespace glut
{

/*==========================================================================*/
/**
 *  Global class.
 */
/*==========================================================================*/
class Global : public kvs::glut::GlobalBase
{
    kvsClassName( kvs::glut::Global );

public:

    Global( int count, char** values );

    virtual ~Global( void );

public:

    void insert( const kvs::VisualizationPipeline& pipeline );
};

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__GLOBAL_H_INCLUDE

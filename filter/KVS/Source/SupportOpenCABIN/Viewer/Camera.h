/*****************************************************************************/
/**
 *  @file   Camera.h
 *  @author Naohisa Sakamoto
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
#ifndef KVS__OPENCABIN__CAMERA_H_INCLUDE
#define KVS__OPENCABIN__CAMERA_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Camera>


namespace kvs
{

namespace opencabin
{

/*===========================================================================*/
/**
 *  @brief  Camera class for OpenCABIN.
 */
/*===========================================================================*/
class Camera : public kvs::Camera
{
    kvsClassName( kvs::opencabin::Camera );

public:

    virtual void update( void );
};

} // end of namespace opencabin

} // end of namespace kvs

#endif // KVS__OPENCABIN__CAMERA_H_INCLUDE

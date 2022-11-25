/*****************************************************************************/
/**
 *  @file   VideoObject.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VideoObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__OPENCV__VIDEO_OBJECT_H_INCLUDE
#define KVS__OPENCV__VIDEO_OBJECT_H_INCLUDE

#include <kvs/ObjectBase>
#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/opencv/CaptureDevice>


namespace kvs
{

namespace opencv
{

/*===========================================================================*/
/**
 *  @brief  Video object.
 */
/*===========================================================================*/
class VideoObject : public kvs::ObjectBase
{
    // Class name.
    kvsClassName( kvs::opencv::VideoObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::ObjectBase );

public:

    enum PixelType
    {
        Gray8   =  8, ///< 8 bit gray pixel
        Color24 = 24  ///< 24 bit RGB color pixel (8x8x8 bits)
    };

public:

    int                        m_device_id; ///< capture device ID
    kvs::opencv::CaptureDevice m_device;    ///< video capture device
    PixelType                  m_type;      ///< pixel type
    size_t                     m_width;     ///< capture widht
    size_t                     m_height;    ///< capture height
    size_t                     m_nchannels; ///< number of channels

public:

    VideoObject( void );

    VideoObject( const int device_id );

    virtual ~VideoObject( void );

public:

    const ObjectType objectType( void ) const;

    const int deviceID( void ) const;

    const kvs::opencv::CaptureDevice& device( void ) const;

    const PixelType type( void ) const;

    const size_t width( void ) const;

    const size_t height( void ) const;

    const size_t nchannels( void ) const;

public:

    const bool initialize( const size_t device_id );
};

} // end of namespace opencv

} // end of namespace kvs

#endif // KVS__OPENCV__VIDEO_OBJECT_H_INCLUDE

/*****************************************************************************/
/**
 *  @file   DisplayFormat.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DisplayFormat.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__DISPLAY_FORMAT_H_INCLUDE
#define KVS__DISPLAY_FORMAT_H_INCLUDE

#include <kvs/OpenGL>
#include <kvs/ClassName>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Display format class.
 */
/*===========================================================================*/
class DisplayFormat
{
    kvsClassName( kvs::DisplayFormat );

protected:

    bool m_double_buffer; ///< double buffering (true:double buffering, false:single buffering, default:true)
    bool m_color_buffer; ///< color buffer mode (true:RGBA mode, false:color index mode, default:true)
    bool m_depth_buffer; ///< depth buffer (true:enable, false:disable, default:true)
    bool m_accumulation_buffer; ///< accumulation buffer (true:enable, false:disable, default:false)
    bool m_stencil_buffer; ///< stencil buffer (true:enable, false:disable, default:false)
    bool m_stereo_buffer; ///< stereo buffer (true:enable, false:disable, default:false)
    bool m_multisample_buffer; ///< multisample buffer (true:enable, false:disable, default:false)
    bool m_alpha_channel; ///< alpha channel (true:enable, false:disable, default:false)

public:

    DisplayFormat( void );

    DisplayFormat( const DisplayFormat& format );

    virtual ~DisplayFormat( void );

public:

    const bool doubleBuffer( void ) const;

    const bool colorBuffer( void ) const;

    const bool depthBuffer( void ) const;

    const bool accumulationBuffer( void ) const;

    const bool stencilBuffer( void ) const;

    const bool stereoBuffer( void ) const;

    const bool multisampleBuffer( void ) const;

    const bool alphaChannel( void ) const;

public:

    void setDoubleBuffer( const bool enable );

    void setColorBuffer( const bool enable );

    void setDepthBuffer( const bool enable );

    void setAccumulationBuffer( const bool enable );

    void setStencilBuffer( const bool enable );

    void setStereoBuffer( const bool enable );

    void setMultisampleBuffer( const bool enable );

    void setAlphaChannel( const bool enable );

public:

    DisplayFormat& operator = ( const DisplayFormat& format );
};

} // end of namespace kvs

#endif // KVS__DISPLAY_FORMAT_H_INCLUDE

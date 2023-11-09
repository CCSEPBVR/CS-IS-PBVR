/*****************************************************************************/
/**
 *  @file   Widget.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Widget.h 620 2010-09-30 08:03:52Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVSVIEW__WIDGET_H_INCLUDE
#define KVSVIEW__WIDGET_H_INCLUDE

#include <string>
#include <kvs/glut/Label>
#include <kvs/glut/LegendBar>
#include <kvs/glut/OrientationAxis>


namespace kvsview
{

namespace Widget
{

/*===========================================================================*/
/**
 *  @brief  Label class to show the frame rate.
 */
/*===========================================================================*/
class FPSLabel : public kvs::glut::Label
{
    std::string m_renderer_name; ///< renderer name

public:

    FPSLabel( kvs::ScreenBase* screen, const std::string renderer_name );

    void screenUpdated( void );
};

/*===========================================================================*/
/**
 *  @brief  Legend bar class.
 */
/*===========================================================================*/
class LegendBar : public kvs::glut::LegendBar
{
public:

    LegendBar( kvs::ScreenBase* screen );

    void screenResized( void );
};

/*===========================================================================*/
/**
 *  @brief  Orientation axis class.
 */
/*===========================================================================*/
class OrientationAxis : public kvs::glut::OrientationAxis
{
public:

    OrientationAxis( kvs::ScreenBase* screen );

    void screenResized( void );
};

} // end of namespace Widget

} // end of namespace kvsview

#endif // KVSVIEW__WIDGET_H_INCLUDE

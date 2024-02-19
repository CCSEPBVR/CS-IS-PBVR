/*****************************************************************************/
/**
 *  @file   TransferFunctionEditorColorFreeformCurve.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditorColorFreeformCurve.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__LGEND_DIVISION_COLOR_H_INCLUDE
#define PBVR__KVS__VISCLIENT__LGEND_DIVISION_COLOR_H_INCLUDE

#include <string>
#include <kvs/ClassName>
#include <kvs/OpenGL>
#include <kvs/RGBColor>
#include <kvs/RGBAColor>
#include <kvs/Vector2>
#include <kvs/TransferFunction>
#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/MouseReleaseEventListener>
#include <kvs/glut/Screen>
#include <kvs/glut/PushButton>
#include <kvs/glut/Text>
#include <kvs/glut/ColorPalette>

namespace kvs
{

namespace visclient
{
class LegendpanelUI;

/*===========================================================================*/
/**
 *  @brief  Legend Frame Color class.
 */
/*===========================================================================*/
class LegendDivisionColor : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::LegendDivisionColor );

public:

    typedef kvs::glut::Screen SuperClass;

protected:

    bool m_visible;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorPalette*      m_color_palette;       ///< color palette
    kvs::glut::PushButton*        m_save_button;         ///< save button
    kvs::glut::PushButton*        m_cancel_button;       ///< cancel button

    kvs::visclient::LegendpanelUI* m_parent;

public:

    LegendDivisionColor( kvs::visclient::LegendpanelUI* parent = 0, kvs::ScreenBase* parent_screen = 0 );

    virtual ~LegendDivisionColor();

public:

    int show();

    virtual void hide();

public:

    kvs::ScreenBase* screen();

    const kvs::glut::ColorPalette* colorPalette() const;

public:

    const kvs::RGBColor color() const;

public:

    virtual void save();

    virtual void cancel();
};


} // end of namespace visclient

} // end of namespace kvs

#endif // PBVR__KVS__VISCLIENT__LGEND_DIVISION_COLOR_H_INCLUDE

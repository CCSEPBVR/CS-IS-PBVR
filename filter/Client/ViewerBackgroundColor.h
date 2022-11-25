/*****************************************************************************/
/**
 *  @file   ViewerBackgroundColor.h
 *  @author Fujitus
 */
/*****************************************************************************/
#ifndef PBVR__KVS__VISCLIENT__VIEWER_BACKGROUNDCOLOR_H_INCLUDE
#define PBVR__KVS__VISCLIENT__VIEWER_BACKGROUNDCOLOR_H_INCLUDE

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
class ViewerpanelUI;;


class ViewerBackgroundColor : public kvs::glut::Screen
{
    kvsClassName( kvs::visclient::ViewerBackgroundColor );

public:

    typedef kvs::glut::Screen SuperClass;

protected:

    bool m_visible;

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorPalette*      m_color_palette;       ///< color palette
    kvs::glut::PushButton*        m_save_button;         ///< save button
    kvs::glut::PushButton*        m_cancel_button;       ///< cancel button

    kvs::visclient::ViewerpanelUI* m_parent;

public:

    ViewerBackgroundColor( kvs::visclient::ViewerpanelUI* parent = 0, kvs::ScreenBase* parent_screen = 0 );

    virtual ~ViewerBackgroundColor();

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

#endif // PBVR__KVS__VISCLIENT__VIEWER_BACKGROUNDCOLOR_H_INCLUDE


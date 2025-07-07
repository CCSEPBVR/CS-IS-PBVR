/*****************************************************************************/
/**
 *  @file   RemoteDesktop.h
 *  @author Keisuke Tajiri
 */
/*****************************************************************************/
#pragma once
#include <SupportOpenXR/OpenXR.h>
#include <kvs/OpenGL>
#include <kvs/Texture2D>
#include <kvs/FrameBufferObject>
#include <kvs/Xform>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>

namespace kvs
{

namespace openxr
{

/*===========================================================================*/
/**
 *  @brief  RemoteDesktop class.
 */
/*===========================================================================*/
class RemoteDesktop
{
private:
    bool m_visible = false;
    HDC m_hdc = nullptr;
    HBITMAP m_hbitmap = nullptr;
    kvs::Vec2i m_screen_pos = kvs::Vec2i::Zero();
    kvs::Vec2i m_screen_size = kvs::Vec2i::Zero();
    kvs::UInt8* m_rdp_image;
    kvs::Texture2D m_rdp_texture;
    kvs::Xform m_rdp_xform = kvs::Xform();
    float m_rdp_height = 0.6f;

    kvs::FrameBufferObject m_vk_fbo;
    kvs::Texture2D m_vk_texture;
    kvs::Vec2i m_vk_size = kvs::Vec2i::Zero();
    kvs::Vec2i m_vk_mouse_pos = kvs::Vec2i::Zero();
    kvs::Vec2i m_vk_button_size = kvs::Vec2i::Zero();
    kvs::Vec2i m_vk_space_size = kvs::Vec2i::Zero();
    bool m_vk_pressed = false;
    bool m_vk_pressing = false;
    kvs::Xform m_vk_xform = kvs::Xform();
    bool m_shift_mode = false;
    bool m_symbol_mode = false;

public:
    RemoteDesktop();
    virtual ~RemoteDesktop();

public:
    void initialize();
    void setVisible( bool visible ) { m_visible = visible; }
    bool visible() const { return m_visible; }
    void setRDPXform( const kvs::Xform& xform ) { m_rdp_xform = xform; }
    void update( const kvs::Xform& controller_xform, 
                 bool pressed, 
                 bool pressing, 
                 bool released, 
                 float& distance );
    void render();

private:
    void clickedMouseLeft();
    void pressMouseLeft();
    void releaseMouseLeft();
    void pressedKey( kvs::Int32 key, bool shift );
    void grabDesktopImage();
    bool raycast( const kvs::Vec3& ray_origin, 
                  const kvs::Vec3& ray_dir,
                  const kvs::Vec3& plane_origin,
                  const kvs::Vec3& plane_front_dir,
                  const kvs::Vec3& plane_up_dir,
                  const kvs::Vec3& plane_right_dir,
                  const kvs::Vec2& plane_size,
                  kvs::Vec2& out_uv,
                  float& out_distance);
};

} // end of namespace openxr

} // end of namespace kvs

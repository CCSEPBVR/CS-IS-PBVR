#pragma once
#include <kvs/Vector3>
#include <kvs/Vector2>

namespace kvs { namespace openxr { namespace controller_visual {

// All visual tuning is kept here. Positions/lengths are metres in the FBX
// axes (before the model correction), except the pointer which uses the
// calibrated controller axes.
struct PoseCorrection
{
    // Provisional pitch based on the observed 30-45 degree mismatch.
    // In OpenXR grip axes, +X rotation raises forward (-Z) towards +Y;
    // use a negative angle to lower it. Rotation order is Z * Y * X.
    // Set all components to zero to restore the uncorrected grip orientation.
    kvs::Vec3 rotation_degrees = kvs::Vec3( -35.0f, 0.0f, 0.0f );
};

struct ModelCorrection
{
    kvs::Vec3 scale = kvs::Vec3::Ones();
    kvs::Vec3 rotation_degrees = kvs::Vec3( 0.0f, 180.0f, 0.0f );
    kvs::Vec3 translation = kvs::Vec3::Zero();
};

struct Settings
{
    // Left, Right: shared by model/labels, pointer and fly-through only.
    // Raw controller status and coordinate points remain uncorrected.
    PoseCorrection pose[2];
    // Hardware Art v1.8: UnitScaleFactor=1 (cm), Y up; mesh nodes have unit scale.
    // The front trigger lies on +Z in the FBX; turn it towards OpenXR -Z.
    float source_unit_to_metres = 0.01f;
    ModelCorrection model[2]; // Left, Right: independent adjustment if needed.
    const char* model_path[2] = {
        "Meta Quest Touch Pro/models/questpro_controllers_left.fbx",
        "Meta Quest Touch Pro/models/questpro_controllers_right.fbx"
    };
    // Used only for materials whose Stingray BaseColor is not exposed by Assimp.
    const char* base_color_texture[2] = {
        "controller_l_lo_BaseColor.png", "controller_r_lo_BaseColor.png"
    };

    const char* font_family = "Yu Gothic";
    int font_pixels = 36;
    int texture_width = 1024;
    int texture_height = 224;
    int text_margin_pixels = 20;
    kvs::Vec2 label_size = kvs::Vec2( 0.27f, 0.059f );
    // Fixed XZ plane parallel to the button face, with its normal along +Y.
    // Text faces upwards and follows the controller, never a billboard.
    kvs::Vec3 label_rotation_degrees = kvs::Vec3( -90.0f, 180.0f, 0.0f );
    float leader_width_pixels = 2.0f;
    float grip_press_threshold = 0.5f;

    kvs::Vec3 pointer_origin = kvs::Vec3( 0.0f, 0.0f, -0.035f );
    float pointer_length = 0.12f;
    float pointer_width_pixels = 2.0f;
};

enum class LabelVisibility { Always, GripPressed, GripReleased };

struct Label
{
    const char* text;
    kvs::Vec3 centre;
    kvs::Vec3 leader_start;
    kvs::Vec3 button;
    LabelVisibility visibility = LabelVisibility::GripPressed;
};

inline const Settings settings;
// Button anchors were read from b_button_b/b_button_oculus/b_thumbstick/
// b_trigger_front in the actual v1.8 FBX, converted from cm to metres.
// Label centres and leader starts share a Y height; rows run along Z so that
// all three rows are visible from above. +X appears on the viewer's left after
// the default model correction, so the left controller's outer label uses +X.
inline const Label right_labels[] = {
    { u8"Bボタン（長押し）\nPlot Over Lineモード ON/OFF",
      { 0.19f, 0.015f, 0.17f }, { 0.055f, 0.015f, 0.17f }, { 0.013317f, 0.006368f, 0.003020f } },
    { u8"Bボタン（短押し）\nPlot Over Lineの場所指定",
      { 0.19f, 0.015f, 0.095f }, { 0.055f, 0.015f, 0.095f }, { 0.013317f, 0.006368f, 0.003020f } },
    { u8"サムスティック 上・下\nフライスルー",
      { -0.19f, 0.015f, 0.17f }, { -0.055f, 0.015f, 0.17f }, { -0.006998f, -0.002463f, 0.007780f } },
    { u8"サムスティック 右・左\n視点左右回転",
      { -0.19f, 0.015f, 0.095f }, { -0.055f, 0.015f, 0.095f }, { -0.006998f, -0.002463f, 0.007780f } },
    { u8"Meta/Oculusボタン\nVirtual Desktopモード ON/OFF",
      { -0.19f, 0.015f, 0.02f }, { -0.055f, 0.015f, 0.02f }, { -0.013280f, 0.001855f, -0.011046f }, LabelVisibility::Always },
    { u8"左右トリガーを同時押し\nオブジェクトの拡大・縮小",
      { 0.19f, 0.015f, 0.02f }, { 0.055f, 0.015f, 0.02f }, { 0.011774f, -0.003923f, 0.028344f } },
    // New row keeps the verified existing label positions unchanged.
    // Grip anchor is approximate and can be tuned after on-device inspection.
    { u8"右グリップボタン\n押している間、操作説明を表示",
      { 0.19f, 0.015f, -0.055f }, { 0.055f, 0.015f, -0.055f }, { 0.025f, -0.03f, -0.025f }, LabelVisibility::GripReleased }
};

inline const Label left_labels[] = {
    { u8"左右トリガーを同時押し\nオブジェクトの拡大・縮小",
      { 0.19f, 0.015f, 0.02f }, { 0.055f, 0.015f, 0.02f }, { -0.011770f, -0.003920f, 0.028340f } },
    // X anchor is approximate; the label uses the vacant outer row.
    { u8"Xボタン（長押し）\n視点リセット",
      { 0.19f, 0.015f, 0.095f }, { 0.055f, 0.015f, 0.095f }, { -0.013f, 0.006f, 0.018f } }
};

} } }

#ifndef PBVRGUI_H
#define PBVRGUI_H

#include "ExtendedKVS/QtScreen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <QMainWindow>

#include "Widgets/Preference.h"
#include "Widgets/VolumeTransform.h"
#include "Widgets/Connect.h"

namespace Ui {
class PBVRGUI;
}

class PBVRGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit PBVRGUI(kvs::qt::Application& app, QWidget *parent = nullptr);
    void initialize();
    ~PBVRGUI();

private:
    Ui::PBVRGUI *ui;
    QtScreen* m_screen;
    kvs::StochasticRenderingCompositor* compositor;
    kvs::ColorMapBar* m_color_map_bar;
    kvs::OrientationAxis* m_orientation_axis;

    Preference m_preference;
    VolumeTransform m_volumeTransform;
    Connect m_connect;

private:
    void onPreference()      { m_preference.show(); }
    void onVolumeTransform() { m_volumeTransform.show(); }
    void onConnect() { m_connect.show(); }
};

#endif // PBVRGUI_H

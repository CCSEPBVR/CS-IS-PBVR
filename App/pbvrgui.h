#ifndef PBVRGUI_H
#define PBVRGUI_H

#include <kvs/qt/Screen>
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <QMainWindow>

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
    kvs::qt::Screen* m_screen;
    kvs::StochasticRenderingCompositor* compositor;
    kvs::ColorMapBar* m_color_map_bar;
    kvs::OrientationAxis* m_orientation_axis;
};

#endif // PBVRGUI_H

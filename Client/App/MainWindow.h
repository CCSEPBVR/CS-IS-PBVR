#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>

#include <kvs/qt/Application>
#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>

// ウィジェット群(A~Z)
// ABCDEFGHIJKLMNOPQRSTUVWXYZ
#include "Communication.h"

#ifdef OPENXR_SCREEN
#include "VRHandControllerListener.h"
#endif

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( kvs::qt::Application& app, QWidget *parent = nullptr );
    ~MainWindow();

private:
    // メンバ変数群
    Ui::MainWindow *ui;

    // KVS関連
    kvs::qt::jaea::Screen* m_screen                             = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor            = nullptr;
    kvs::ColorMapBar* m_color_map_bar                           = nullptr;
    kvs::OrientationAxis* m_orientation_axis                    = nullptr;
    kvs::Label* m_fps_label                                     = nullptr;
    kvs::Label* m_time_step_label                               = nullptr;

    // 通信関連
    QWebSocket* m_web_binary_socket                             = nullptr;
    QWebSocket* m_web_text_socket                               = nullptr;

    // ウィジェット群(A~Z)
    // ABCDEFGHIJKLMNOPQRSTUVWXYZ
    Communication* m_communication = nullptr;

    // VR関連
#ifdef OPENXR_SCREEN
    VRHandControllerListener* m_vr_listener;
#endif

    // メソッド群
    void initialize();
    void communicationInitialize();
};
#endif // MAINWINDOW_H

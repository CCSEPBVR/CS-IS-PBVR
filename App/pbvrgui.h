#ifndef PBVRGUI_H
#define PBVRGUI_H

#include <kvs/qt/Screen>
#include <kvs/StochasticRenderingCompositor>
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
};

#endif // PBVRGUI_H

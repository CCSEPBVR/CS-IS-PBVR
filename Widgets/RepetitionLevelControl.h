#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>

#include "ExtendedKVS/Screen.h"
#include <kvs/StochasticRenderingCompositor>

namespace Ui {
class RepetitionLevelControl;
}

class RepetitionLevelControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit RepetitionLevelControl(QWidget *parent = nullptr);
    ~RepetitionLevelControl();

    void setScreen( kvs::qt::jaea::Screen* screen ) { m_screen = screen; }
    void setCompositor( kvs::StochasticRenderingCompositor* compositor ) { m_compositor = compositor; }

private:
    Ui::RepetitionLevelControl *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::StochasticRenderingCompositor* m_compositor;

private slots:
    void onApplyButtonClicked();
};

#endif // REPETITIONLEVELCONTROL_H

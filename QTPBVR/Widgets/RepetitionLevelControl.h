#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>

#include "Widgets/ShadingController.h"
#include "ExtendedKVS/Screen.h"
#include <kvs/StochasticRenderingCompositor>

namespace Ui {
class RepetitionLevelControl;
}

class RepetitionLevelControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit RepetitionLevelControl(QWidget *parent = nullptr, PBVRGUI *pbvr_gui = nullptr, ShadingController* shading_controller = nullptr);
    ~RepetitionLevelControl();

private:
    Ui::RepetitionLevelControl *ui;
    PBVRGUI *m_pbvr_gui;
    ShadingController* m_shading_controller;

private slots:
    void onApplyButtonClicked();
};

#endif // REPETITIONLEVELCONTROL_H

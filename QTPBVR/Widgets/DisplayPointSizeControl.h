#ifndef DISPLAYPOINTSIZECONTROL_H
#define DISPLAYPOINTSIZECONTROL_H

#include <QDockWidget>

#include "Widgets/Preference.h"
#include "ExtendedKVS/Screen.h"
#include <kvs/StochasticRenderingCompositor>

namespace Ui {
class DisplayPointSizeControl;
}

class DisplayPointSizeControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit DisplayPointSizeControl(QWidget *parent = nullptr, PBVRGUI *pbvr_gui = nullptr, Preference* preference = nullptr);
    ~DisplayPointSizeControl();

private:
    Ui::DisplayPointSizeControl *ui;
    PBVRGUI *m_pbvr_gui;
    Preference* m_preference;

private slots:
    void onApplyButtonClicked();
};

#endif // DISPLAYPOINTSIZECONTROL_H

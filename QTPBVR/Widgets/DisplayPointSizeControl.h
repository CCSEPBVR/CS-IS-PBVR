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
    explicit DisplayPointSizeControl(QWidget *parent = nullptr);
    ~DisplayPointSizeControl();
    void setPreference( Preference* preference ){ m_preference = preference; }
    void setScreen( kvs::qt::jaea::Screen* screen ) { m_screen = screen; }

private:
    Ui::DisplayPointSizeControl *ui;
    Preference* m_preference;
    kvs::qt::jaea::Screen* m_screen;

private slots:
    void onApplyButtonClicked();
};

#endif // DISPLAYPOINTSIZECONTROL_H

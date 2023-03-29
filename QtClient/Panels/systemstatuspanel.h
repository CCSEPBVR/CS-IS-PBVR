#ifndef SYSTEMSTATUSPANEL_H
#define SYSTEMSTATUSPANEL_H


#include <QDockWidget>
#include <QTimer>

#include <QGlue/extCommand.h>



namespace Ui {
class SystemstatusPanel;
}

class SystemstatusPanel : public QDockWidget
{

public:

    explicit SystemstatusPanel(QWidget *parent = 0);

    static void updateSystemStatus(int nverts);
    void updateSystemStatus();

    ~SystemstatusPanel();

public:
    Qt::DockWidgetArea default_area=Qt::RightDockWidgetArea;
    static int m_no_repsampling;
    static int interval_msec;

private:
    int nparticles;
    static SystemstatusPanel* instance;
    Ui::SystemstatusPanel *ui;
    QTimer updateTimer;

private:
    void updateMemoryInfo();
    void setCommandInstance(ExtCommand* cmd);

private://Event handlers

    void customEvent(QEvent * event);
    void onUpdateTimer();
    void on_updateRequest(int nverts);
    void onSamplingIntervalChanged(int arg1);
    void onDisableSamplingToggled(int arg1);
    void onSetParamClicked();







};

#endif // SYSTEMSTATUSPANEL_H

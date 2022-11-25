#ifndef FilterinfoPanel_H
#define FilterinfoPanel_H

#include <QDockWidget>

#include <QGlue/extCommand.h>
#include <Client/ReceivedMessage.h>
namespace Ui {
class FilterinfoPanel;
}

class FilterinfoPanel : public QDockWidget
{

public:

    explicit FilterinfoPanel(QWidget *parent);
    static void updateFilterInfo();
    static void updateFilterInfo(jpv::ParticleTransferServerMessage reply);
    static void updateFilterInfo(kvs::visclient::ReceivedMessage* reply);
    Qt::DockWidgetArea default_area=Qt::RightDockWidgetArea;

    ~FilterinfoPanel();

private:
    void customEvent(QEvent * event);
    static FilterinfoPanel* instance;
    Ui::FilterinfoPanel *ui;
};

#endif // FilterinfoPanel_H

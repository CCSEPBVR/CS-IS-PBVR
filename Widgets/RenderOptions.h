#ifndef RENDEROPTIONS_H
#define RENDEROPTIONS_H

#include <QDockWidget>

#include "ParticleTransferProtocol.h"
namespace Ui {
class RenderOptions;
}

class RenderOptions : public QDockWidget
{
    Q_OBJECT

public:
    explicit RenderOptions(QWidget *parent = nullptr);
    ~RenderOptions();
    void setClientMessage( jpv::ParticleTransferClientMessage* client_message ){ m_client_message = client_message; }


private:
    Ui::RenderOptions *ui;
    jpv::ParticleTransferClientMessage* m_client_message;

private slots:
    void onApplyButtonClicked();
};

#endif // RENDEROPTIONS_H

#ifndef COORDINATES_H
#define COORDINATES_H

#include <QDialog>
#include "ParticleTransferProtocol.h"
#include "Widgets/MergePanel2.h"
namespace Ui {
class Coordinates;
}

class Coordinates : public QDialog
{
    Q_OBJECT

public:
    explicit Coordinates(QWidget *parent = nullptr, MergePanel2 *merge = nullptr);
    ~Coordinates();
    void setClientMessage( jpv::ParticleTransferClientMessage* client_message ){ m_client_message = client_message; }

private:
    Ui::Coordinates *ui;
    MergePanel2* m_merge;
    jpv::ParticleTransferClientMessage* m_client_message;

private slots:
    void onApplyButtonClicked();
};

#endif // COORDINATES_H

#ifndef COORDINATES_H
#define COORDINATES_H

#include <QDialog>
#include "ParticleTransferProtocol.h"
namespace Ui {
class Coordinates;
}

class Coordinates : public QDialog
{
    Q_OBJECT

public:
    explicit Coordinates(QWidget *parent = nullptr);
    ~Coordinates();
    void setClientMessage( jpv::ParticleTransferClientMessage* client_message ){ m_client_message = client_message; }

private:
    Ui::Coordinates *ui;
    jpv::ParticleTransferClientMessage* m_client_message;

private slots:
//    jpv::ParticleTransferClientMessage::EquationToken convertToken( std::string );
    void onApplyButtonClicked();
};

#endif // COORDINATES_H

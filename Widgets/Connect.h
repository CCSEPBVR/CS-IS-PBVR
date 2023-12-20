#ifndef CONNECT_H
#define CONNECT_H

#include <QDialog>

#include "ParticleTransferClient.h"
#include "ParticleTransferProtocol.h"
#include "ExpressionTokenizer.h"
#include "ExpressionConverter.h"

#include "Widgets/FilterInfomation.h"

#include <kvs/PointObject>
#include "ExtendedKVS/Screen.h"
namespace Ui {
class Connect;
}

class Connect : public QDialog
{
    Q_OBJECT

public:
    explicit Connect(QWidget *parent = nullptr);
    ~Connect();
    void setScreen( kvs::qt::jaea::Screen* screen ){ m_screen = screen; }
    void setCamera( kvs::Camera* camera ){ m_camera = camera; }
    void setFilterInfomation( FilterInfomation* filter_infomation ){ m_filter_infomation = filter_infomation; }

private:
    Ui::Connect *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::Camera* m_camera;
    FilterInfomation* m_filter_infomation;

    jpv::ParticleTransferClientMessage::EquationToken convertToken( std::string );
    void connect1();
    kvs::PointObject* connect2();

    void onConnectButtonClicked();
};

#endif // CONNECT_H

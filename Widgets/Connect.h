#ifndef CONNECT_H
#define CONNECT_H

#include <QDialog>

#include "ParticleTransferClient.h"
#include "ParticleTransferProtocol.h"
#include "ExpressionTokenizer.h"
#include "ExpressionConverter.h"

#include "Widgets/DataProperties.h"

#include <kvs/PointObject>
#include "ExtendedKVS/Screen.h"

class Merge;

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
    void setMerge( Merge* merge ){  m_merge = merge; }
    void setFilterInfomation( DataProperties* filter_infomation ){ m_filter_infomation = filter_infomation; }
    jpv::ParticleTransferClientMessage* getClientMessage(){ return &m_client_message; }
    kvs::PointObject* connect2( int timeStep );

private:
    Ui::Connect *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::Camera* m_camera;
    Merge* m_merge;
    DataProperties* m_filter_infomation;

    jpv::ParticleTransferClientMessage m_client_message;

    jpv::ParticleTransferClientMessage::EquationToken convertToken( std::string );
    void connect1();

    void onConnectButtonClicked();
};

#endif // CONNECT_H

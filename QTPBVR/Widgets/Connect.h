#ifndef CONNECT_H
#define CONNECT_H

#include <QDialog>

#include "ParticleTransferClient.h"
#include "ParticleTransferProtocol.h"
#include "ExpressionTokenizer.h"
#include "ExpressionConverter.h"
#include "Widgets/ExtendedTransferFunctionMessage.h"
#include "Widgets/DataProperties.h"
#include "Widgets/TransferFunctionEditor.h"

#include <kvs/PointObject>
#include "ExtendedKVS/Screen.h"
#include "ReceivedMessage.h"

class MergePanel2;

namespace Ui {
class Connect;
}

class Connect : public QDialog
{
    Q_OBJECT

public:
    explicit Connect(QWidget *parent = nullptr, MergePanel2* merge = nullptr, DataProperties* filter_infomation = nullptr, TransferFunctionEditor* transfer_function_editor = nullptr);
    ~Connect();
    void setScreen( kvs::qt::jaea::Screen* screen ){ m_screen = screen; }
    void setCamera( kvs::Camera* camera ){ m_camera = camera; }
    jpv::ParticleTransferClientMessage* getClientMessage(){ return &m_client_message; }
    jpv::ParticleTransferServerMessage* getServerMessage(){ return &m_server_message; }
    kvs::visclient::ReceivedMessage* getReceivedMessage(){ return &m_received_message; }
    kvs::PointObject* generateParticles( int timeStep );
    void deletedServerObject();

private:
    Ui::Connect *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::Camera* m_camera;
    MergePanel2* m_merge;
    DataProperties* m_filter_infomation;
    TransferFunctionEditor* m_transfer_function_editor;
    ExtendedTransferFunctionMessage m_extended_transfer_function_message;

    jpv::ParticleTransferClientMessage m_client_message;
    jpv::ParticleTransferServerMessage m_server_message;
    kvs::visclient::ReceivedMessage m_received_message;

    void connectServerCS();
    void connectServerIS();

    void onVolumeDataBrowseButtonClicked();
    void onTransferFunctionFileBrowseButtonClicked();
    void onConnectButtonClicked();
};

#endif // CONNECT_H

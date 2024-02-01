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
    void setTransferFunctionEditor( TransferFunctionEditor* transfer_function_editor ){ m_transfer_function_editor = transfer_function_editor; }
    jpv::ParticleTransferClientMessage* getClientMessage(){ return &m_client_message; }
    kvs::PointObject* connect2( int timeStep );

private:
    Ui::Connect *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::Camera* m_camera;
    Merge* m_merge;
    DataProperties* m_filter_infomation;
    TransferFunctionEditor* m_transfer_function_editor;
    ExtendedTransferFunctionMessage m_extended_transfer_function_message;

    jpv::ParticleTransferClientMessage m_client_message;

    jpv::ParticleTransferClientMessage::EquationToken convertToken( std::string );
    void connect1();

    void onConnectButtonClicked();
};

#endif // CONNECT_H

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
#include "Widgets/GlyphEditor.h"

#include <kvs/PointObject>
#include "ExtendedKVS/Screen.h"
#include "ReceivedMessage.h"

class PBVRGUI;
class MergePanel;
class RenderOptions;

namespace Ui {
class Connect;
}

class Connect : public QDialog
{
    Q_OBJECT

public:
    explicit Connect(QWidget *parent = nullptr, PBVRGUI *pbvr_gui = nullptr, MergePanel* merge = nullptr, DataProperties* filter_infomation = nullptr, RenderOptions* render_options = nullptr, TransferFunctionEditor* transfer_function_editor = nullptr, GlyphEditor* glyph_editor = nullptr );
    ~Connect();
    jpv::ParticleTransferClientMessage* getClientMessage(){ return &m_client_message; }
    jpv::ParticleTransferServerMessage* getServerMessage(){ return &m_server_message; }
    kvs::visclient::ReceivedMessage* getReceivedMessage(){ return &m_received_message; }
    kvs::PointObject* generateParticles( int timeStep );
    kvs::PolygonObject* generateGlyphPolygons( int timeStep );
    void sendTransferFunction();
    void deletedServerObject();

private:
    Ui::Connect *ui;
    PBVRGUI *m_pbvr_gui;
    MergePanel* m_merge;
    DataProperties* m_filter_infomation;
    RenderOptions* m_render_options;
    TransferFunctionEditor* m_transfer_function_editor;
    GlyphEditor* m_glyph_editor;
    ExtendedTransferFunctionMessage m_extended_transfer_function_message;
    bool connecting = false; //通信状態の判定パラメータ

    jpv::ParticleTransferClientMessage m_client_message;
    jpv::ParticleTransferServerMessage m_server_message;
    kvs::visclient::ReceivedMessage m_received_message;

    void connectServer();

    void onVolumeDataBrowseButtonClicked();
    void onTransferFunctionFileBrowseButtonClicked();
    void onConnectButtonClicked();
};

#endif // CONNECT_H

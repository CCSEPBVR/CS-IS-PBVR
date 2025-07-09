#ifndef CONNECT_H
#define CONNECT_H

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

#include "Screen.h"
#include <kvs/Camera>
#include <kvs/PointObject>
#include <kvs/PolygonObject>
#include <kvs/PolygonGlyphObject>

#include "ParticleTransferProtocol.h"
#include "ParticleTransferClient.h"
#include "ReceivedMessage.h"
#include "ObjectItem.h"
#include "TransferFunctionItem.h"
#include "GlyphItem.h"
#include "PlotOverLineItem.h"
#include "Utils.h"

namespace Ui {
class Connect;
}

class Connect : public QDialog
{
    Q_OBJECT
public:
    enum SERVERMODE
    {
        None            = 0,
        ClientServer    = 1,
        InSitu          = 2
    };

public:
    explicit Connect( kvs::qt::jaea::Screen*,
                      QWidget *parent = nullptr );
    ~Connect();
    kvs::PointObject* generateParticles( int timeStep );
    kvs::PolygonObject* generateGlyphPolygons( int timeStep );

private:
    Ui::Connect *ui;
    kvs::qt::jaea::Screen* m_screen = nullptr;

    SERVERMODE m_server_mode;
    jpv::ParticleTransferClientMessage m_client_message;
    jpv::ParticleTransferServerMessage m_server_message;
    kvs::visclient::ReceivedMessage m_received_message;
    bool m_connecting = false;
    QString m_transfer_function_file_path;

    GlyphItem::GlyphType m_glyph_type;
    float m_glyph_scale_factor;

    bool m_is_plot_over_line;

    bool m_has_insitu_object = false;

    void initialize();
    void connectServer();
    void sendGlyphFlagFalse();
    void sendRecvPlotOverLine( int timeStep );

public:
    void deleteServerObject();

signals:
    void importTransferFunctionFromFile( const QString transferFunctionFile );
    // void importTransferFunctionFromServer( const jpv::ParticleTransferServerMessage& serverMessage, kvs::visclient::ReceivedMessage receivedMessage );
    void importTransferFunctionFromServer( const jpv::ParticleTransferServerMessage& serverMessage, kvs::visclient::ReceivedMessage receivedMessage, const bool isImportFlag );
    void applyTransferFunction();
    void updateNumberOfVector( const int );
    // void insituObjectActive( kvs::Vec3f, kvs::Vec3f );
    void insituObjectActive( kvs::Vec3f, kvs::Vec3f, int, float, float );

    void serverPointObjectCS( QString, ServerPointObjectPropertiesCS );
    void serverGlyphObjectCS( QString, ServerGlyphObjectPropertiesCS );
    void serverPointObjectIS( QString, ServerPointObjectPropertiesIS );
    void serverGlyphObjectIS( QString, ServerGlyphObjectPropertiesIS );
    void updateTransferFunctionRangeAndView( const jpv::ParticleTransferServerMessage* serverMessage, kvs::visclient::ReceivedMessage receivedMessage );
    void clearTransferFunction();
    void updateInSituTimeStep( int minTimeStep, int maxTimeStep );
    void setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );

public slots:
    void failedTransferFunctionImport();
    void successTransferFunctionImport( const QString transferFunctionFile );
    void updateTransferFunctionClientMessage( QString colorSynthesizer, QString opacitySynthesizer, QStandardItemModel* model );
    void updateGlyphParameterClientMessage( QStandardItemModel* model );
    void updatePlotOverLineParameterClientMessage( QStandardItemModel* model );
    void updateRenderParameterClientMessage( QStandardItemModel* model );
    void updateCoordinateParameterClientMessage( QStandardItemModel* model );

private slots:
    void onConnect();
    void onVolumeDataBrowse();
    void onTransferFunctionBrowse();
};

#endif // CONNECT_H

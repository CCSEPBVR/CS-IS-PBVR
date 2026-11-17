#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>
#include <QJsonObject>

#include <kvs/ParticleBasedRenderer>
#include <kvs/PointObject>
#include <kvs/StochasticRenderingCompositor>
#include "Screen.h" // <kvs/qt/Screen>
#include "VizMode.h"
#include "WebSocketPair.h"

namespace Ui
{
class RepetitionLevelControl;
}

class RepetitionLevelControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit RepetitionLevelControl(
        kvs::qt::jaea::Screen* screen,
        kvs::StochasticRenderingCompositor* compositor,
        WebSocketPair* websockets,
        Viz::Mode* vizMode,
        QWidget *parent = nullptr );
    ~RepetitionLevelControl();

signals:
    void shading( kvs::RendererBase* rendererBase );

public slots:
    void onOperatorStateUpdate( const bool operatorState );
    void onUpdateCurrentRepetitionLevel();
    void onReceiveRepetitionLevelParameter( const QJsonObject& payload );
    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::RepetitionLevelControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor = nullptr;
    WebSocketPair* m_web_sockets = nullptr;
    Viz::Mode* m_viz_mode = nullptr;
    bool m_is_operator = false;

private slots:
    void onApply();
};

#endif // REPETITIONLEVELCONTROL_H

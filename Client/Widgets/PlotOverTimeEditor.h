#ifndef PLOTOVERTIMEEDITOR_H
#define PLOTOVERTIMEEDITOR_H

#include <QDockWidget>
#include <QLabel>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

#include "Screen.h"
#include <kvs/PointObject>
#include <kvs/LineObject>
#include <kvs/StochasticPointRenderer>
#include <kvs/StochasticLineRenderer>

#include "WebSocketPair.h"

#include "../../Shared/JsonKeys.h"

// NOTE:glew.hより先にgl.hが読み込まれるのを防ぐため、qcustomplot.hを最後にインクルードする
#include "qcustomplot.h"

namespace Ui
{
class PlotOverTimeEditor;
}

class PlotOverTimeEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverTimeEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, QWidget *parent = nullptr );
    ~PlotOverTimeEditor();
    void reset();

signals:
    void updateStatusBarMessage( const QString& message );
    void sharePointFocus( const kvs::Vec3 resultMinObjectCoords, const kvs::Vec3 resultMaxObjectCoords );
    void plotOverTimeParameterUpdate();

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し

    void onUpdateNumberOfVector( const int numberOfVector );

    void onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void onUpdateTranslation();

    // NOTE:テキストソケット用
    void onReceivePlotOverTimeParameter( const QJsonObject& payload );

    void onDrawVRPlotOverTime( kvs::Real32 coordArray[ 2 * 3 ] );
    void onToggleShowHideVRPlotOverTime();

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::PlotOverTimeEditor *ui;

    QCustomPlot* m_q_custom_plot    = nullptr;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    WebSocketPair* m_web_sockets    = nullptr;

    bool m_is_operator              = true;

    QStringList m_variable_list;

    kvs::Vec3 m_point_initial_translation;

    kvs::PointObject* m_point_object        = nullptr;

    double m_x_coord = 0.0;
    double m_y_coord = 0.0;
    double m_z_coord = 0.0;

    QVector<double> m_time_buffer;
    QVector<QVector<double>> m_value_buffers;

    bool m_plot_inited       = false;
    bool m_auto_fit          = true; // Controls both X following and Y fitting.
    bool m_user_interacting  = false;

    QMetaObject::Connection m_combo_connection;

    QMap<int, QVector<double>> m_samples;
    QMap<int, QString> m_sample_states;
    QJsonObject m_applied_observation;
    QJsonArray m_applied_point;
    QString m_pending_apply;
    QString m_calculation_state = "idle";
    QLabel* m_calculation_status = nullptr;
    void clearPlot();
    void updateCalculationStatus();

    void redrawPlotFromBuffers( int index, bool resetView = false );
    void calculateInitialTranslation();

public slots:
     void onDisplayGraphComboBoxChanged( int index );
    void onResetPlotView();
    void onPlotOverTimeCheckBox();
    void onApply();
};

#endif // PLOTOVERTIMEEDITOR_H

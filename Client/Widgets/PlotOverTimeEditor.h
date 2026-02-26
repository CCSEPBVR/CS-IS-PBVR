#ifndef PLOTOVERTIMEEDITOR_H
#define PLOTOVERTIMEEDITOR_H

#include <QDockWidget>

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
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

    kvs::Vec3 m_point_initial_translation;

    kvs::PointObject* m_point_object        = nullptr;

    double m_x_coord;
    double m_y_coord;
    double m_z_coord;

    QVector<double> m_time_buffer;
    QVector<QVector<double>> m_value_buffers;

    bool m_plot_inited       = false;
    bool m_follow_right_edge = true;
    bool m_user_interacting  = false;
    bool m_need_rebuild      = true;

    QMetaObject::Connection m_combo_connection;

    struct PlotOverTimeUiSnapshot
    {
        bool enable = false;
        int resolution = 0;
        int target = 0;
    };

    PlotOverTimeUiSnapshot m_last_snap_shot;
    bool m_has_last_snap_shot = false;

    PlotOverTimeUiSnapshot captureUiSnapshot() const;
    static bool fuzzyEqual( double a, double b );
    static bool sameVec3( const double a[3], const double b[3] );

    void redrawPlotFromBuffers( int index, double tLatest, bool forceRebuild );
    void calculateInitialTranslation();

public slots:
     void onDisplayGraphComboBoxChanged( int index );
    void onResetPlotView();
    void onPlotOverTimeCheckBox();
    void onApply();
};

#endif // PLOTOVERTIMEEDITOR_H

#ifndef PLOTOVERLINEEDITOR_H
#define PLOTOVERLINEEDITOR_H

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
class PlotOverLineEditor;
}

class PlotOverLineEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLineEditor( kvs::qt::jaea::Screen* screen, WebSocketPair* websockets, QWidget *parent = nullptr );
    ~PlotOverLineEditor();
    void reset();

signals:
    void updateStatusBarMessage( const QString& message );
    void sharePointFocus( const kvs::Vec3 resultMinObjectCoords, const kvs::Vec3 resultMaxObjectCoords );
    void plotOverLineParameterUpdate();

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し

    void onUpdateNumberOfVector( const int numberOfVector );
    void onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void onUpdateTranslation();

    // NOTE:テキストソケット用
    void onReceivePlotOverLineParameter( const QJsonObject& payload );
    // void onReceivePlotOverLineParameter( const QJsonObject& payload );

    void onDrawVRPlotOverLine( kvs::Real32 coordArray[ 2 * 3 ] );
    void onToggleShowHideVRPlotOverLine();

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    // NOTE:kvs::LineObject用定数
    static constexpr size_t k_N = 2; // 点の数
    static constexpr size_t k_L = 1; // 線の数
    // 座標
    static constexpr kvs::Real32 k_initial_coords[k_N * 3] =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
        };
    // 色
    static constexpr kvs::UInt8 k_initial_colors[k_N * 3] =
        {
            0, 255, 0,
            0, 255, 0,
        };
    // 接続情報
    static constexpr kvs::UInt32 k_initial_connections[k_L * 2] =
        {
            0, 1,
        };

    Ui::PlotOverLineEditor *ui;

    QCustomPlot* m_q_custom_plot    = nullptr;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    WebSocketPair* m_web_sockets    = nullptr;

    bool m_is_operator              = true;

    QStringList m_variable_list;
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

    kvs::Vec3 m_start_point_initial_translation;
    kvs::Vec3 m_end_point_initial_translation;

    kvs::PointObject* m_start_point_object      = nullptr; // NOTE:始点(左手)
    kvs::PointObject* m_end_point_object        = nullptr; // NOTE:終点(右手)
    kvs::LineObject* m_plot_over_line_object    = nullptr;
    QPair<int,int> m_plot_over_line_object_id   = { -1, -1 };

    struct PlotOverLineUiSnapshot
    {
        bool enable = false;
        int resolution = 0;
        int target = 0;
        double start[3] = { 0.0, 0.0, 0.0 };
        double end[3]   = { 0.0, 0.0, 0.0 };
    };

    PlotOverLineUiSnapshot m_last_snap_shot;
    bool m_has_last_snap_shot = false;

    PlotOverLineUiSnapshot captureUiSnapshot() const;
    static bool fuzzyEqual( double a, double b );
    static bool sameVec3( const double a[3], const double b[3] );

    void calculateInitialTranslation();
    void updateTranslationSpinBox( kvs::PointObject* point,
                                  QDoubleSpinBox* xTranslationSpinBox, QDoubleSpinBox* yTranslationSpinBox, QDoubleSpinBox* zTranslationSpinBox,
                                  QDoubleSpinBox* xCoordsSpinBox     , QDoubleSpinBox* yCoordsSpinBox     , QDoubleSpinBox* zCoordsSpinBox,
                                  const kvs::Vec3& initialTranslation );

    void updateCoordsSpinBox( kvs::PointObject* point,
                             QDoubleSpinBox* xTranslationSpinBox, QDoubleSpinBox* yTranslationSpinBox, QDoubleSpinBox* zTranslationSpinBox,
                             QDoubleSpinBox* xCoordsSpinBox     , QDoubleSpinBox* yCoordsSpinBox     , QDoubleSpinBox* zCoordsSpinBox,
                             const kvs::Vec3& initialTranslation );

    void setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );

public slots:
    void onResetPlotView();

    void onPlotOverLineGroupBoxCheckBox();

    void onStartTranslationValueChanged();
    void onStartCoordsValueChanged();

    void onEndTranslationValueChanged();
    void onEndCoordsValueChanged();

    void onCreateLine();
    void onApply();
};

#endif // PLOTOVERLINEEDITOR_H

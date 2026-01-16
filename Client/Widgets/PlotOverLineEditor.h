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

public slots:
    void onUpdateServerState( bool serverState );     // true:接続中
    void onOperatorStateUpdate( bool operatorState ); // true:権限あり
    void onReset();
    void onUpdateNumberOfVector( const int numberOfVector );
    void onUpdateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void onUpdateTranslation();
    void onSetPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );
    void onReceivePlotOverLineParameter( const QJsonObject& dataArray );
    void onReceiveRequestDataAtPlotOverLineParameter( const QJsonObject& dataArray );
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

private:
    // kvs::LineObject用定数
    static constexpr size_t N = 2; // 点の数
    static constexpr size_t L = 1; // 線の数
    static constexpr kvs::Real32 InitialCoords[N * 3] = // 座標
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
        };
    static constexpr kvs::UInt8 InitialColors[N * 3] = // 色
        {
            0, 255, 0,
            0, 255, 0,
        };
    static constexpr kvs::UInt32 InitialConnections[L * 2] = // 接続情報
        {
            0, 1,
        };

    Ui::PlotOverLineEditor *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    WebSocketPair* m_web_sockets    = nullptr;
    bool m_is_operator;

    QStringList m_vector_list;
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

    QCustomPlot* m_q_custom_plot = nullptr;

    kvs::Vec3 m_start_point_initial_translation;
    kvs::Vec3 m_end_point_initial_translation;

    kvs::PointObject* m_start_point_object      = nullptr;  // 始点 左手
    kvs::PointObject* m_end_point_object        = nullptr;  // 終点 右手
    kvs::LineObject* m_plot_over_line_object    = nullptr;
    QPair<int,int> m_plot_over_line_object_id = { -1, -1 };

    void calculateInitialTranslation();
    void updateTranslationSpinBox( kvs::PointObject* point,
                                  QDoubleSpinBox* xTranslationSpinBox  , QDoubleSpinBox* yTranslationSpinBox   , QDoubleSpinBox* zTranslationSpinBox,
                                  QDoubleSpinBox* xCoordsSpinBox       , QDoubleSpinBox* yCoordsSpinBox        , QDoubleSpinBox* zCoordsSpinBox,
                                  const kvs::Vec3& initialTranslation );

    void updateCoordsSpinBox( kvs::PointObject* point,
                             QDoubleSpinBox* xTranslationSpinBox  , QDoubleSpinBox* yTranslationSpinBox   , QDoubleSpinBox* zTranslationSpinBox,
                             QDoubleSpinBox* xCoordsSpinBox       , QDoubleSpinBox* yCoordsSpinBox        , QDoubleSpinBox* zCoordsSpinBox,
                             const kvs::Vec3& initialTranslation );

private slots:
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

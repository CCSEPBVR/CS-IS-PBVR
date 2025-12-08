#ifndef PLOTOVERLINEEDITORWIP_H
#define PLOTOVERLINEEDITORWIP_H

#include <QDockWidget>

#include "Screen.h"
#include "WebSocketPair.h"
#include <kvs/PointObject>
#include <kvs/LineObject>
#include <kvs/StochasticPointRenderer>
#include <kvs/StochasticLineRenderer>

// NOTE:glew.hより先にgl.hが読み込まれるのを防ぐため、qcustomplot.hを最後にインクルードする
#include "qcustomplot.h"

namespace Ui {
class PlotOverLineEditorWIP;
}

class PlotOverLineEditorWIP : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLineEditorWIP( WebSocketPair* websockets, kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~PlotOverLineEditorWIP();

public slots:
    void updateOperatorState( bool operatorState ); // true:権限あり
    void reset();
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

    void updateNumberOfVector( const int numberOfVector );
    void updateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void updateTranslation();
    void setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );//
    // void drawPlotOverLineFromVRHands( kvs::Real32 CoordArray[ 2 * 3 ] ); // FIXME:onCreateLineと機能が重複しています。
    // void togglePlotOverLineFromVRHands();
    void receivePlotOverLineParameter( const QJsonObject& plotOverLineParameter );


private:
    // kvs::LineObject用定数
    static constexpr size_t N = 2; // 点の数
    static constexpr size_t L = 1; // 線の数
    static constexpr kvs::Real32 k_initial_coords[N * 3] = // 座標
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
        };
    static constexpr kvs::UInt8 k_initial_colors[N * 3] = // 色
        {
            0, 255, 0,
            0, 255, 0,
        };
    static constexpr kvs::UInt32 k_initial_connections[L * 2] = // 接続情報
        {
            0, 1,
        };

    Ui::PlotOverLineEditorWIP *ui;

    WebSocketPair* m_web_sockets = nullptr;
    bool m_is_operator;

    QStringList m_vector_list;
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

    QCustomPlot* m_q_custom_plot = nullptr;

    kvs::qt::jaea::Screen* m_screen             = nullptr;

    kvs::Vec3 m_start_point_initial_translation;
    kvs::Vec3 m_end_point_initial_translation;

    kvs::PointObject* m_start_point_object      = nullptr;  // 始点 左手
    kvs::PointObject* m_end_point_object        = nullptr;  // 終点 右手
    kvs::LineObject* m_plot_over_line_object    = nullptr;
    QPair<int,int> m_plot_over_line_object_id = { -1, -1 };

    void initialize();
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

#endif // PLOTOVERLINEEDITORWIP_H

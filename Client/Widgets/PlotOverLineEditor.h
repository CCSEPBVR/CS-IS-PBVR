#ifndef PLOTOVERLINEEDITOR_H
#define PLOTOVERLINEEDITOR_H

#include <QDockWidget>

#include "PlotOverLineItem.h"

#include "Screen.h"
#include "WebSocketPair.h"
#include <kvs/PointObject>
#include <kvs/LineObject>
#include <kvs/Vector>
#include <kvs/StochasticPointRenderer>
#include <kvs/StochasticLineRenderer>

// HACK:glew.hより先にgl.hが読み込まれるのを防ぐため、qcustomplot.hを最後にインクルードする
#include "qcustomplot.h"

namespace Ui
{
class PlotOverLineEditor;
}

class PlotOverLineEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLineEditor( WebSocketPair* websockets, kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~PlotOverLineEditor();

public slots:
    void updateNumberOfVector( const int numberOfVector );
    void updateFocus( kvs::Vec3 resultMinObjectCoords, kvs::Vec3 resultMaxObjectCoords );
    void updateTranslation();
    void setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );
    void drawPlotOverLineFromVRHands( kvs::Real32 CoordArray[ 2 * 3 ] ); // FIXME:onCreateLineと機能が重複しています。
    void togglePlotOverLineFromVRHands();
    void updateOperatorState( bool operatorState ); // true:権限あり
    void reset();
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

private:    
    Ui::PlotOverLineEditor *ui;

    WebSocketPair* m_web_sockets = nullptr;
    QStandardItemModel *m_model = nullptr;

    QCustomPlot* m_q_custom_plot = nullptr;

    kvs::qt::jaea::Screen* m_screen;
    kvs::Vec3 m_start_point_initial_translation;
    kvs::Vec3 m_end_point_initial_translation;
    kvs::PointObject* m_start_point_object = nullptr;   // 始点
    kvs::PointObject* m_end_point_object = nullptr;     // 終点
    kvs::LineObject* m_plot_over_line_object = nullptr;
    QPair<int,int> m_plot_over_line_object_id = { -1, -1 };

    QStringList m_vector_list;
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

    // bool m_is_enable;   // FIXME:ui->からとればいいかも
    // int m_resolution;   // FIXME:ui->からとればいいかも
    // QString variable;   // FIXME:ui->からとればいいかも
    kvs::Vec3 m_start_coords;
    kvs::Vec3 m_end_coords;
    bool m_is_send_available = false;

    // メソッド群
    void initialize();
    void calculateInitialTranslation();
    void updateTranslationXYZ( kvs::PointObject* point,
                              QDoubleSpinBox* xTransSpin, QDoubleSpinBox* yTransSpin, QDoubleSpinBox* zTransSpin,
                              QDoubleSpinBox* xCoordSpin, QDoubleSpinBox* yCoordSpin, QDoubleSpinBox* zCoordSpin,
                              const kvs::Vec3& initialTranslation );

    void updateCoordinateXYZ( kvs::PointObject* point,
                             QDoubleSpinBox* xTransSpin, QDoubleSpinBox* yTransSpin, QDoubleSpinBox* zTransSpin,
                             QDoubleSpinBox* xCoordSpin, QDoubleSpinBox* yCoordSpin, QDoubleSpinBox* zCoordSpin,
                             const kvs::Vec3& initialTranslation );

private slots:
    void onResetPlotView();
    void onPlotOverLineGroupBoxCheckBox();
    void startTranslationUpdateXYZ();
    void startCoordsUpdateXYZ();
    void endTranslationUpdateXYZ();
    void endCoordsUpdateXYZ();
    void onCreateLine();
    void onApply();
};

#endif // PLOTOVERLINEEDITOR_H

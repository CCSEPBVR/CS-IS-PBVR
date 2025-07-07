#ifndef PLOTOVERLINE_H
#define PLOTOVERLINE_H

#include <QDockWidget>
#include <QDoubleSpinBox>

#include "Screen.h"
#include <kvs/PointObject>
#include <kvs/StochasticPointRenderer>
#include <kvs/LineObject>
#include <kvs/StochasticLineRenderer>
#include <kvs/ObjectManager>

#include "PlotOverLineItem.h"

namespace Ui {
class PlotOverLine;
}

class PlotOverLine : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLine( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~PlotOverLine();

private:
    void initialize();
    void updateTranslationXYZ( kvs::PointObject* point,
                              QDoubleSpinBox* xTransSpin, QDoubleSpinBox* yTransSpin, QDoubleSpinBox* zTransSpin,
                              QDoubleSpinBox* xCoordSpin, QDoubleSpinBox* yCoordSpin, QDoubleSpinBox* zCoordSpin,
                              const kvs::Vec3& initialTranslation );

    void updateCoordinateXYZ( kvs::PointObject* point,
                             QDoubleSpinBox* xTransSpin, QDoubleSpinBox* yTransSpin, QDoubleSpinBox* zTransSpin,
                             QDoubleSpinBox* xCoordSpin, QDoubleSpinBox* yCoordSpin, QDoubleSpinBox* zCoordSpin,
                             const kvs::Vec3& initialTranslation );

    void focus( kvs::Vec3, kvs::Vec3 );
    void updateCalTrans();

private:
    Ui::PlotOverLine *ui;
    QStandardItemModel *m_model = nullptr;
    kvs::qt::jaea::Screen* m_screen       = nullptr;
    kvs::Vec3 m_start_point_initial_translation;
    kvs::Vec3 m_end_point_initial_translation;
    kvs::PointObject* m_start_point = nullptr;
    kvs::PointObject* m_end_point   = nullptr;
    kvs::LineObject* m_plot_over_line = nullptr;
    QVector<QPair<int,int>> m_local_object_ids; // ObjectEditorから渡される予定なので削除予定
    QPair<int,int> m_line_object_id;
    QStringList m_vector_list;
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

signals:
    void updatePlotOverLineParameterClientMessage( QStandardItemModel* model );
    void requestReplaceServerPointObject();

public slots:
    void updateFocus( kvs::Vec3 min, kvs::Vec3 max );
    void updatePointsTranslation();
    void updateNumberOfVector( const int numberOfVector );    

public slots:
    void setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );
    void drawPlotOverLine( kvs::Real32 CoordArray[ 2 * 3 ] );
    void showHidePlotOverLine();

private slots:
    void onReset();
    void onEnable();    
    void startTranslationUpdateXYZ();
    void startCoordUpdateXYZ();
    void endTranslationUpdateXYZ();
    void endCoordUpdateXYZ();
    void onApply();
};

#endif // PLOTOVERLINE_H

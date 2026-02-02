#ifndef POINTSIZECONTROL_H
#define POINTSIZECONTROL_H

#include <QDockWidget>

#include <kvs/IDManager>
#include <kvs/ParticleBasedRenderer>
#include <kvs/PointObject>
#include <kvs/RendererManager>
#include <kvs/StochasticRendererBase>
#include "Screen.h" // <kvs/qt/Screen>

namespace Ui
{
class PointSizeControl;
}

class PointSizeControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit PointSizeControl( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~PointSizeControl();

public slots:
    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::PointSizeControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

private slots:
    void onPointSizeValueChanged();
};

#endif // POINTSIZECONTROL_H

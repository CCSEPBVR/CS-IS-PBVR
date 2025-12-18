#ifndef POINTSIZECONTROL_H
#define POINTSIZECONTROL_H

#include <QDockWidget>

#include "Screen.h"
#include <kvs/IDManager>
#include <kvs/RendererManager>
#include <kvs/StochasticRendererBase>
#include <kvs/ParticleBasedRenderer>

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
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

private:
    Ui::PointSizeControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

private slots:
    void onPointSizeValueChanged();
};

#endif // POINTSIZECONTROL_H

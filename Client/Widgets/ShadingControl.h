#ifndef SHADINGCONTROL_H
#define SHADINGCONTROL_H

#include <QDockWidget>
#include <QButtonGroup>

#include "Screen.h"
#include <kvs/StochasticRendererBase>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/ParticleBasedRenderer>
#include <kvs/StochasticTexturedPolygonRenderer>
#include <kvs/IDManager>
#include <kvs/RendererManager>

namespace Ui
{
class ShadingControl;
}

class ShadingControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit ShadingControl( kvs::qt::jaea::Screen*, QWidget *parent = nullptr );
    ~ShadingControl();

public slots:
    void onShading( kvs::RendererBase* rendererBase );
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

private:
    Ui::ShadingControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

private slots:
    void onChangedShadingParameter();
};

#endif // SHADINGCONTROL_H

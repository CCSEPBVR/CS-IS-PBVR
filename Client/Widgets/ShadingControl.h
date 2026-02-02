#ifndef SHADINGCONTROL_H
#define SHADINGCONTROL_H

#include <QDockWidget>
#include <QButtonGroup>

#include <kvs/IDManager>
#include <kvs/ParticleBasedRenderer>
#include <kvs/RendererManager>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/StochasticRendererBase>
#include <kvs/StochasticTexturedPolygonRenderer>
#include "Screen.h" // <kvs/qt/Screen>

// FIXME:lasフォーマットのオブジェクトにシェーディングは必要ありません
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
    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::ShadingControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

private slots:
    void onChangedShadingParameter();
};

#endif // SHADINGCONTROL_H

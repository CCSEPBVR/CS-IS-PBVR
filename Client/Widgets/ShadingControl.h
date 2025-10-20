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

private:
    // メンバ変数群
    Ui::ShadingControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    // メソッド群
    void initialize();

public slots:
    void shading( kvs::RendererBase* rendererBase );

private slots:
    void onChangedShadingParameter();
};

#endif // SHADINGCONTROL_H

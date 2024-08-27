#ifndef SHADINGCONTROLLER_H
#define SHADINGCONTROLLER_H

#include <QDockWidget>
#include "Widgets/Preference.h"
#include "ExtendedKVS/Screen.h"

namespace Ui {
class ShadingController;
}

class ShadingController : public QDockWidget
{
    Q_OBJECT

public:
    explicit ShadingController(QWidget *parent = nullptr);
    ~ShadingController();

    void setScreen( kvs::qt::jaea::Screen* screen ) { m_screen = screen; }
    void applyShading( kvs::RendererBase*& rendererBase );

private:
    Ui::ShadingController *ui;
    kvs::qt::jaea::Screen* m_screen;

private slots:
    void onChangeShaderParameter();
};

#endif // SHADINGCONTROLLER_H

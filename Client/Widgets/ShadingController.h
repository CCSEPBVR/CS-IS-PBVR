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
    explicit ShadingController(QWidget *parent = nullptr, PBVRGUI *pbvr_gui = nullptr);
    ~ShadingController();

    void applyShading( kvs::RendererBase*& rendererBase );

private:
    Ui::ShadingController *ui;
    PBVRGUI *m_pbvr_gui;

private slots:
    void onChangeShaderParameter();
};

#endif // SHADINGCONTROLLER_H

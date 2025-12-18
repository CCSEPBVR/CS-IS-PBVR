#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>

#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ParticleBasedRenderer>

namespace Ui
{
class RepetitionLevelControl;
}

class RepetitionLevelControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit RepetitionLevelControl( kvs::qt::jaea::Screen* screen, kvs::StochasticRenderingCompositor* compositor, QWidget *parent = nullptr );
    ~RepetitionLevelControl();

public slots:
    void onUpdateCurrentRepetitionLevel();
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

signals:
    void shading( kvs::RendererBase* rendererBase );

private:
    Ui::RepetitionLevelControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor = nullptr;

private slots:
    void onApply();
};

#endif // REPETITIONLEVELCONTROL_H

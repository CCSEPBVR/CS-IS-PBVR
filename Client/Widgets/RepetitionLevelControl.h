#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>

#include <kvs/ParticleBasedRenderer>
#include <kvs/PointObject>
#include <kvs/StochasticRenderingCompositor>
#include "Screen.h" // <kvs/qt/Screen>

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

signals:
    void shading( kvs::RendererBase* rendererBase );

public slots:
    void onUpdateCurrentRepetitionLevel();
    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::RepetitionLevelControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor = nullptr;

private slots:
    void onApply();
};

#endif // REPETITIONLEVELCONTROL_H

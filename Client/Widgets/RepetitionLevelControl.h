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

private:
    // メンバ変数群
    Ui::RepetitionLevelControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor = nullptr;

    // メソッド群
    void initialize();

private slots:
    void onApply();

public slots:
    void updateCurrentRepetitionLevel();
    void loadParameter();
    void saveParameter();

signals:
    void shading( kvs::RendererBase* rendererBase );
};

#endif // REPETITIONLEVELCONTROL_H

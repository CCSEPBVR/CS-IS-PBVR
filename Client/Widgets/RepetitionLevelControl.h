#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>

#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ParticleBasedRenderer>

namespace Ui {
class RepetitionLevelControl;
}

class RepetitionLevelControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit RepetitionLevelControl( kvs::qt::jaea::Screen*,
                                     kvs::StochasticRenderingCompositor*,
                                     QWidget *parent = nullptr );
    ~RepetitionLevelControl();
    void updateUI();

private:
    Ui::RepetitionLevelControl *ui;
    kvs::qt::jaea::Screen* m_screen                     = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor    = nullptr;

    void initialize();

signals:
    void shading( kvs::RendererBase* );

private slots:
    void onApply();
};

#endif // REPETITIONLEVELCONTROL_H

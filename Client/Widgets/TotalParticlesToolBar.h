#ifndef TOTALPARTICLESTOOLBAR_H
#define TOTALPARTICLESTOOLBAR_H

#include <QToolBar>
#include <QLabel>
#include <QHBoxLayout>

#include "Screen.h"
#include <kvs/IDManager>
#include <kvs/PointObject>
#include <kvs/RendererManager>
#include <kvs/StochasticRendererBase>
#include <kvs/ParticleBasedRenderer>

class TotalParticlesToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TotalParticlesToolBar( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~TotalParticlesToolBar();

public slots:
    void onUpdateTotalParticles();

private:
    static constexpr const char* k_no_point_objects_text = "NO POINT OBJECTS";

    kvs::qt::jaea::Screen* m_screen = nullptr;

    QLabel* m_total_particles_label;
    QLabel* m_total_particles_display;
};

#endif // TOTALPARTICLESTOOLBAR_H

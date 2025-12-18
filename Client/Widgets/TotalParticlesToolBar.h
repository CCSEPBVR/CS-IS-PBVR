#ifndef TOTALPARTICLESTOOLBAR_H
#define TOTALPARTICLESTOOLBAR_H

#include <QToolBar>
#include <QLabel>
#include <QHBoxLayout>

class TotalParticlesToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit TotalParticlesToolBar( QWidget *parent = nullptr );
    ~TotalParticlesToolBar();

public slots:
    // FIXME:引数ではなく、ParticleBasedRendererを使用しているオブジェクトから算出したほうが良いかもしれません。
    void updateTotalParticles( int );

private:
    QLabel* m_total_particles_label;
    QLabel* m_total_particles_display;
};

#endif // TOTALPARTICLESTOOLBAR_H

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
    void updateTotalParticles( int ); // FIXME:引数ではなく、ParticleBasedRendererを使用しているオブジェクトから参照したほうが良いかもしれません。

private:
    QLabel* m_total_particles_label;
    QLabel* m_total_particles_display;

    void initialize();
};

#endif // TOTALPARTICLESTOOLBAR_H

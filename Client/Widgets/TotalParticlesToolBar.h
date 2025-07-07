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

private:
    void initialize();

private:
    QLabel* m_total_particles_label;
    QLabel* m_total_particles_display;

public slots:
    void updateTotalParticles( int );
};

#endif // TOTALPARTICLESTOOLBAR_H

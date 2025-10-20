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
    // メンバ変数群
    QLabel* m_total_particles_label;
    QLabel* m_total_particles_display;

    // メソッド群
    void initialize();

public slots:
    void updateTotalParticles( int ); // FIXME:引数ではなく、ParticleBasedRendererを使用しているオブジェクトから参照したほうが良いかもしれません。
};

#endif // TOTALPARTICLESTOOLBAR_H

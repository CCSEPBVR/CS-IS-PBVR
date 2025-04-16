#ifndef OPACITYMAP_H
#define OPACITYMAP_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>

class OpacityMap : public QWidget
{
    Q_OBJECT
public:
    OpacityMap( QWidget *parent = nullptr,
               const QVector<float> &opacities = { 0.0, 0.5, 1.0 },
               int resolution = 256 );
    ~OpacityMap();

    void setOpacities( const QVector<float> &opacities );
    void setResolution( int resolution );
    const QVector<float> getOpacities();

protected:
    void paintEvent( QPaintEvent *event ) override;

private:
    void interpolateOpacities();

protected:
    QVector<float> m_opacity_map;
    int m_resolution;
};

#endif // OPACITYMAP_H

#ifndef COLORMAP_H
#define COLORMAP_H

#include <QWidget>
#include <QPainter>

class ColorMap : public QWidget
{
    Q_OBJECT
public:
    ColorMap( QWidget *parent = nullptr,
             const QVector<QColor> &colors = { QColor( 255, 0, 0 ), QColor( 0, 255, 0 ), QColor( 0, 0, 255 ) },
             int resolution = 256 );
    ~ColorMap();
    void setColors( const QVector<QColor> &colors );
    void setResolution( int resolution );
    const QVector<QColor> getColors();

protected:
    void paintEvent( QPaintEvent *event ) override;

private:
    void interpolateColors();

protected:
    QVector<QColor> m_color_map;
    int m_resolution;
};

#endif // COLORMAP_H

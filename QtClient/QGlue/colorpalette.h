#ifndef ColorPalette_H
#define ColorPalette_H

#include <QGlue/qglue_base.h>

#include <kvs/HSVColor>
#include <kvs/ClassName>

namespace QGlue {


class ColorPalette : public QGLUEBaseWidget
{
public:

    ColorPalette(QWidget* screen);
    const kvs::RGBColor color( void ) const;

    virtual ~ColorPalette( void );

protected:

    std::string m_caption;        ///< caption
    QRect   m_SV_palette;         ///< SV (saturation/value) palette
    QRect   m_H_palette;          ///< H (hue) palette
    QRect   m_selected_color_box; ///< selected color box
    int     m_H_indicator;        ///< hue indicator
    int     m_S_indicator;        ///< saturation indicator
    int     m_V_indicator;        ///< value indicator
    //    MOD BY)T.Osaki 2020.04.28
    float     pixelRatio=1;
    QColor  m_upper_edge_color;   ///< upper edge color
    QColor  m_lower_edge_color;   ///< lower edge color

public:

    virtual void paintGL( void );
    void resizeGL(int w, int h);
protected:

    const float get_H_value( void ) const;
    const float get_S_value( void ) const;
    const float get_V_value( void ) const;

    void draw_SV_palette( void );
    void draw_H_palette( void );
    void draw_selected_color_box( void );
    void draw_selected_color_value( void );

    void mouseMoveEvent( QMouseEvent* event );
    void mousePressEvent(QMouseEvent *event);

private:
        typedef QGLUEBaseWidget BaseClass;
};
}
#endif // ColorPalette_H

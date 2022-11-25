#ifndef LEGENDBAR_H
#define LEGENDBAR_H
#include <QGlue/qglue_base.h>

#include <Client/Command.h>
#include "deferredtexture.h"
#include <kvs/ScreenBase>
#include <kvs/EventListener>

using namespace kvs::visclient;

namespace QGlue
{

class LegendBar: public QGLUEBase
{

public:
    //MOD BY)T.Osaki 2020.06.29
    LegendBar( kvs::ScreenBase* screen, const Command& command ) ;
//    LegendBar( kvs::Scene* screen, const Command& command ) ;

    Label captionLabel, minLabel, maxLabel;
public:

    enum OrientationType
    {
        Horizontal = 0, ///< horizontal bar
        Vertical        ///< vertical bar
    };

    void setWidth(int w){
        m_width=w;
        if (this->m_orientation){m_dim2=w;} else{m_dim1=w;}
    }
    void setHeight(int h){
        m_height=h;
        if (this->m_orientation){m_dim1=h;} else{m_dim2=h;}
    }

    void setNumberOfDivisions( const size_t ndivisions )
    {
        m_ndivisions = ndivisions;
    }


    void setDivisionLineWidth( const float division_line_width )
    {
        m_division_line_width = division_line_width;
    }

    void setDivisionLineColor( const kvs::RGBColor& division_line_color )
    {
        m_division_line_color = division_line_color;
    }


    void setCaption( std::string caption )
    {
        m_caption = caption;
        QString qs = QString::fromStdString(m_caption);
        captionLabel.setText(qs,Qt::black);
    }

    void setBorderColor( const kvs::RGBColor& border_color )
    {
        m_border_color = border_color;
    }

    void setBorderWidth( int width){
        m_border_width=width;
    }
    void setVisible(bool state)
    {
        m_visible=state;
    }

    void setPosition(int x, int y){
        position_x=x;
        position_y=y;
    }

    void setFont(const QFont& f);

    void paintEvent();

    void screenUpdated();

    void create_texture( void );

    void screenResizedAfterSelectTransferFunction( const int i );

    void setOrientation( const OrientationType orientation );

    void setRange( const double min_value, const double max_value );

    void setColorMap( const kvs::ColorMap& colormap );

    void screenResized();

    void set_hide();

    void set_show();

    void show(bool flag=true){
        m_visible=flag;
    }


private:
    const Command* m_command;
    int m_selected_transfer_function;
    DeferredTexture2D  m_texture;      ///< texture data

    kvs::ColorMap   m_colormap;     ///< color map
    double          m_min_value;    ///< min. value
    double          m_max_value;    ///< max. value
    bool m_reset_height;
    bool m_reset_width;
    float m_dim1=50;
    float m_dim2=180;
    float m_width=m_dim1;
    float m_height=m_dim2;
    float position_x=10;
    float position_y=10;


    std::string     m_caption;      ///< caption
    OrientationType m_orientation;  ///< bar layout
    size_t          m_ndivisions;   ///< number of divisions of the bar
    float           m_division_line_width; ///< division line width
    kvs::RGBColor   m_division_line_color; ///< division line color

    float           m_border_width; ///< border line width
    kvs::RGBColor   m_border_color; ///< border line color


    bool m_show_range_value;        ///< range value showing flag
    bool m_enable_anti_aliasing;    ///< check flag for anti-aliasing



    bool m_visible;
    int  m_margin=0;

    void draw_color_bar( const int x, const int y, const int width, const int height );
    void draw_border( const int x, const int y, const int width, const int height );
};
}
#endif // LEGENDBAR_H

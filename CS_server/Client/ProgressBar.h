#ifndef PBVR__KVS__VISCLIENT__PROGRESS_BAR_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PROGRESS_BAR_H_INCLUDE

#include "kvs/glut/WidgetBase"
#include "kvs/glut/Screen"
#include "kvs/ClassName"
#include "kvs/RGBColor"


namespace kvs
{

namespace visclient
{

class ProgressBar : public kvs::glut::WidgetBase
{
    // Class name.
    kvsClassName( kvs::glut::ProgressBar );

public:

    typedef kvs::glut::WidgetBase BaseClass;

protected:

    std::string   m_caption; ///< caption
    float         m_value; ///< value
    bool          m_change_value; ///< value change flag
    bool          m_show_range_value; ///< range value showing flag
    kvs::RGBColor m_slider_color; ///< slider (cursor) color
    kvs::RGBColor m_clicked_slider_color; ///< clicked slider (cursor) color
    kvs::RGBColor m_upper_edge_color; ///< upper edge color
    kvs::RGBColor m_lower_edge_color; ///< lower edge color

public:

    ProgressBar( kvs::ScreenBase* screen = 0 );

public:

    virtual void sliderPressed() {};

    virtual void sliderMoved() {};

    virtual void sliderReleased() {};

    virtual void valueChanged() {};

    virtual void screenUpdated() {};

    virtual void screenResized() {};

public:

    const std::string& caption() const;

    const float value() const;

    void setCaption( const std::string& caption );

    void setValue( const float value );

    void setProgressBarColor( const kvs::RGBColor& color );

    void showRange();

    void hideRange();

private:

    void draw_slider_bar( const int x, const int y, const int width );

    //void draw_cursor( const int x, const int y, const int width );

    float get_value( const int x );

    int get_fitted_width();

    int get_fitted_height();

private:

    void paintEvent();

    void resizeEvent( const int width, const int height );
};

}

}

#endif    // PBVR__KVS__VISCLIENT__PROGRESS_BAR_H_INCLUDE


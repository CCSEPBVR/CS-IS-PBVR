//#ifndef WidgetBase_H
//#define WidgetBase_H
//#include <QOpenGLWidget>
//#include <QOpenGLFunctions>
//#include <QMouseEvent>

//#include "qglue_base.h"

//#include <kvs/RGBAColor>
//#include <kvs/glut/Rectangle>
//namespace  QGlue {


//class WidgetBase: public QGLUEBase
//{

//public:

//    WidgetBase( QWidget* parent=0 );

////    ~WidgetBase( );

////    enum FontType
////    {
////      BITMAP_8_BY_13,
////      BITMAP_9_BY_15,
////      BITMAP_TIMES_ROMAN_10,
////      BITMAP_TIMES_ROMAN_24,
////      BITMAP_HELVETICA_10,
////      BITMAP_HELVETICA_12,
////      BITMAP_HELVETICA_18
////    };

////protected:

////    QWidget*   m_parent; ///< pointer to the screen
////    int                m_margin; ///< margin
////    bool               m_is_shown; ///< check flag whether the widget is shown or not
////    kvs::RGBColor      m_text_color; ///< text color
////    kvs::RGBAColor     m_background_color; ///< background color
////    kvs::RGBAColor     m_background_border_color; ///< background border color
////    float              m_background_border_width; ///< background border width
////    int                m_character_width; ///< character width
////    int                m_character_height; ///< character height
////    QPoint             m_active_mouse_pos;
////    FontType           m_font;


////private:

////    WidgetBase( void ){}

////protected:

////    QWidget* screen( void );

////public:

////    const int margin( void ) const;

////    const bool isShown( void ) const;

////    const kvs::RGBColor& textColor( void ) const;

////    const kvs::RGBAColor& backgroundColor( void ) const;

////    const kvs::RGBAColor& backgroundBorderColor( void ) const;

////    const float backgroundBorderWidth( void ) const;

////    const int characterWidth( void ) const;

////    const int characterHeight( void ) const;

////    int x0(){return 0;}
////    int x1(){return this->width();}
////    int y0(){return -this->characterHeight();}
////    int y1(){return this->height()+this->characterHeight();}

////public:

////    void setMargin( const int margin );

////    void setTextColor( const kvs::RGBColor& text_color );

////    void setBackgroundColor( const kvs::RGBAColor& background_color );

////    void setBackgroundOpacity( const float opacity );

////    void setBackgroundBorderColor( const kvs::RGBAColor& border_color );

////    void setBackgroundBorderOpacity( const float opacity );

////    void setBackgroundBorderWidth( const float border_width );

//////    void setCharacterType( FontType font );

////public:

//////    void show( void );

//////    void hide( void );


//////    virtual void paintEvent( void );

////protected:


////    virtual void begin_draw( void );

////    virtual void end_draw( void );

////    virtual void draw_background( void );

//////    virtual void draw_text( const int x, const int y, const std::string& text );

////    virtual void swap_color( kvs::RGBColor& color1, kvs::RGBColor& color2 );

////    virtual kvs::RGBColor get_darkened_color( const kvs::RGBColor& color, const float darkness );

////    virtual int get_fitted_width( void );

////    virtual int get_fitted_height( void );

////    void drawRectangle(const QRect r,  const float w, const kvs::RGBColor& uec, const kvs::RGBColor& lec );

////    void mouseMoveEvent(QMouseEvent *event);
//};

//}
//#endif // WidgetBase_H

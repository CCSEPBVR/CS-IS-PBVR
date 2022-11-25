//#include "WidgetBase.h"
////#include <kvs/OpenGL>
////#include <QGLFunctions>
////#include <kvs/HSVColor>
////#include <kvs/RGBColor>
////#include <kvs/EventHandler>

////// Default parameters.
////namespace { namespace Default
////{
//////void*                CharacterFont = GLUT_BITMAP_8_BY_13;
////const int            CharacterWidth = 8;
////const int            CharacterHeight = 12;
////const kvs::RGBColor  TextColor = kvs::RGBColor( 0, 0, 0 );
////const kvs::RGBAColor BackgroundColor = kvs::RGBAColor( 200, 200, 200, 0.0f );
////const kvs::RGBAColor BackgroundBorderColor = kvs::RGBAColor( 0, 0, 0, 1.0f );
////const float          BackgroundBorderWidth = 0.0f;
////} }


//namespace QGlue
//{

///*===========================================================================*/
///**
// *  @brief  Constructs a new WidgetBase class.
// *  @param  screen [in] pointer to the screen
// */
///*===========================================================================*/
//WidgetBase::WidgetBase( QWidget* parent )
////    m_parent( parent ),
////    m_margin( 10 ),
////    m_is_shown( false )
//{
////    if ( screen ) screen->eventHandler()->attach( this );
////    if (screen) screen->widget=this;
//    // Set default parameters.
////    this->setTextColor( ::Default::TextColor );
////    this->setBackgroundColor( ::Default::BackgroundColor );
////    this->setBackgroundBorderColor( ::Default::BackgroundBorderColor );
////    this->setBackgroundBorderWidth( ::Default::BackgroundBorderWidth );

//    m_margin=0;
////    m_font = BITMAP_8_BY_13;
////    m_character_width = ::Default::CharacterWidth;
////    m_character_height = ::Default::CharacterHeight;
////    Rectangle::setGeometry(0,0,QWidget::width(),QWidget::height());
//}


/////*===========================================================================*/
/////**
//// *  @brief  Destructs the WidgetBase class.
//// */
/////*===========================================================================*/
////WidgetBase::~WidgetBase( void )
////{
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the pointer to the screen.
//// *  @return pointer to the screen
//// */
/////*===========================================================================*/
////QWidget* WidgetBase::screen( void )
////{
////    return( m_parent );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the margin.
//// *  @return margin
//// */
/////*===========================================================================*/
////const int WidgetBase::margin( void ) const
////{
////    return( m_margin );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the text color.
//// *  @return text color
//// */
/////*===========================================================================*/
////const kvs::RGBColor& WidgetBase::textColor( void ) const
////{
////    return( m_text_color );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the background color.
//// *  @return background color
//// */
/////*===========================================================================*/
////const kvs::RGBAColor& WidgetBase::backgroundColor( void ) const
////{
////    return( m_background_color );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the background border color.
//// *  @return background border color
//// */
/////*===========================================================================*/
////const kvs::RGBAColor& WidgetBase::backgroundBorderColor( void ) const
////{
////    return( m_background_border_color );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the background width.
//// *  @return background width
//// */
/////*===========================================================================*/
////const float WidgetBase::backgroundBorderWidth( void ) const
////{
////    return( m_background_border_width );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the character width.
//// *  @return character width
//// */
/////*===========================================================================*/
////const int WidgetBase::characterWidth( void ) const
////{
////    return( m_character_width );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the character height.
//// *  @return character height
//// */
/////*===========================================================================*/
////const int WidgetBase::characterHeight( void ) const
////{
////    return( m_character_height );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Checks whether the widget is shown or not.
//// *  @return true, if the widget is shown
//// */
/////*===========================================================================*/
////const bool WidgetBase::isShown( void ) const
////{
////    return( QWidget::isVisible() );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a margin.
//// *  @param  margin [in] margin
//// */
/////*===========================================================================*/
////void WidgetBase::setMargin( const int margin )
////{
////    m_margin = margin;
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a text color.
//// *  @param  color [in] text color
//// */
/////*===========================================================================*/
////void WidgetBase::setTextColor( const kvs::RGBColor& color )
////{
////    m_text_color = color;
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a background color.
//// *  @param  background_color [in] background color
//// */
/////*===========================================================================*/
////void WidgetBase::setBackgroundColor( const kvs::RGBAColor& background_color )
////{
////    m_background_color = background_color;
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a background opacity value.
//// *  @param  opacity [in] opacity value
//// */
/////*===========================================================================*/
////void WidgetBase::setBackgroundOpacity( const float opacity )
////{
////    const kvs::UInt8 r = m_background_color.r();
////    const kvs::UInt8 g = m_background_color.g();
////    const kvs::UInt8 b = m_background_color.b();
////    m_background_color = kvs::RGBAColor( r, g, b, opacity );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a background color.
//// *  @param  background_border_color [in] background color
//// */
/////*===========================================================================*/
////void WidgetBase::setBackgroundBorderColor( const kvs::RGBAColor& background_border_color )
////{
////    m_background_border_color = background_border_color;
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a background border opacity value.
//// *  @param  opacity [in] background opacity value
//// */
/////*===========================================================================*/
////void WidgetBase::setBackgroundBorderOpacity( const float opacity )
////{
////    const kvs::UInt8 r = m_background_border_color.r();
////    const kvs::UInt8 g = m_background_border_color.g();
////    const kvs::UInt8 b = m_background_border_color.b();
////    m_background_border_color = kvs::RGBAColor( r, g, b, opacity );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Sets a background border width.
//// *  @param  background_border_width [in] background border width
//// */
/////*===========================================================================*/
////void WidgetBase::setBackgroundBorderWidth( const float background_border_width )
////{
////    m_background_border_width = background_border_width;
////}



/////*===========================================================================*/
/////**
//// *  @brief  Delimits the drawing.
//// */
/////*===========================================================================*/
////void WidgetBase::begin_draw( void )
////{
////    GLint vp[4]; glGetIntegerv( GL_VIEWPORT, vp );
////    const GLint left   = vp[0];
////    const GLint bottom = vp[1];
////    const GLint right  = vp[2];
////    const GLint top    = vp[3];

////    glPushAttrib( GL_ALL_ATTRIB_BITS );
////    glMatrixMode( GL_MODELVIEW );  glPushMatrix(); glLoadIdentity();
////    glMatrixMode( GL_PROJECTION ); glPushMatrix(); glLoadIdentity();
////    glOrtho( left, right, top, bottom, -1, 1 ); // The origin is upper-left.
////    glDisable( GL_DEPTH_TEST );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Draws a rectangle as lines.
//// *  @param  rect [in] rectangle
//// *  @param  width [in] line width
//// *  @param  upper_edge_color [in] upper edge color
//// *  @param  lower_edge_color [in] lower edge color
//// */
/////*===========================================================================*/
////void WidgetBase::drawRectangle(
////    const QRect rect,
////    const float width,
////    const kvs::RGBColor& upper_edge_color,
////    const kvs::RGBColor& lower_edge_color )
////{
////    GLfloat x0 = static_cast<GLfloat>( rect.x() );
////    GLfloat y0 = static_cast<GLfloat>( rect.y() );
////    GLfloat x1 = static_cast<GLfloat>( rect.x() + rect.width() );
////    GLfloat y1 = static_cast<GLfloat>( rect.y() + rect.height() );

////    glLineWidth( width );
////    glBegin( GL_LINES );
////    {
////        glColor3ub( upper_edge_color.r(), upper_edge_color.g(), upper_edge_color.b() );
////        glVertex2f( x0, y0 ); glVertex2f( x1, y0 ); // top
////        glVertex2f( x0, y0 ); glVertex2f( x0, y1 ); // left

////        glColor3ub( lower_edge_color.r(), lower_edge_color.g(), lower_edge_color.b() );
////        glVertex2f( x1, y1 ); glVertex2f( x0, y1 ); // bottom
////        glVertex2f( x1, y0 ); glVertex2f( x1, y1 ); // right
////    }
////    glEnd();
////}
/////*===========================================================================*/
/////**
//// *  @brief  Delimits the drawing.
//// */
/////*===========================================================================*/
////void WidgetBase::end_draw( void )
////{
////    glPopMatrix();
////    glMatrixMode( GL_MODELVIEW );
////    glPopMatrix();
////    glPopAttrib();
////}

/////*==========================================================================*/
/////**
//// *  @brief  Draws the background.
//// */
/////*==========================================================================*/
////void WidgetBase::draw_background( void )
////{
////    glPushAttrib( GL_ALL_ATTRIB_BITS );
////    {
////        glDisable( GL_TEXTURE_1D );
////        glDisable( GL_TEXTURE_2D );
////        glEnable( GL_BLEND );
////        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

////        if ( m_background_color.a() > 0.0f )
////        {
////            // Draw background.
////            glBegin( GL_POLYGON );
////            {
////                const GLubyte r = static_cast<GLubyte>( m_background_color.r() );
////                const GLubyte g = static_cast<GLubyte>( m_background_color.g() );
////                const GLubyte b = static_cast<GLubyte>( m_background_color.b() );
////                const GLubyte a = static_cast<GLubyte>( m_background_color.a() * 255.0f );
////                const GLfloat x0 = static_cast<GLfloat>( this->x0() );
////                const GLfloat x1 = static_cast<GLfloat>( this->x1() );
////                const GLfloat y0 = static_cast<GLfloat>( this->y0() );
////                const GLfloat y1 = static_cast<GLfloat>( this->y1() );
////                glColor4ub( r, g, b, a );
////                glVertex2f( x0, y1 );
////                glVertex2f( x0, y0 );
////                glVertex2f( x1, y0 );
////                glVertex2f( x1, y1 );
////            }
////            glEnd();
////        }

////        if ( m_background_border_width > 0.0f && m_background_border_color.a() > 0.0f )
////        {
////            // Draw outline of the background.
////            glLineWidth( m_background_border_width );
////            glBegin( GL_LINE_LOOP );
////            {
////                const GLubyte r = static_cast<GLubyte>( m_background_border_color.r() );
////                const GLubyte g = static_cast<GLubyte>( m_background_border_color.g() );
////                const GLubyte b = static_cast<GLubyte>( m_background_border_color.b() );
////                const GLubyte a = static_cast<GLubyte>( m_background_border_color.a() * 255.0f );
////                const GLfloat x0 = static_cast<GLfloat>( this->x0() );
////                const GLfloat x1 = static_cast<GLfloat>( this->x1() );
////                const GLfloat y0 = static_cast<GLfloat>( this->y0() );
////                const GLfloat y1 = static_cast<GLfloat>( this->y1() );
////                glColor4ub( r, g, b, a );
////                glVertex2f( x0, y1 );
////                glVertex2f( x0, y0 );
////                glVertex2f( x1, y0 );
////                glVertex2f( x1, y1 );
////            }
////            glEnd();
////        }
////    }
////    glPopAttrib();
////}

/////*===========================================================================*/
/////**
//// *  @brief  Swaps the colors.
//// *  @param  color1 [in] color 1
//// *  @param  color2 [in] color 2
//// */
/////*===========================================================================*/
////void WidgetBase::swap_color( kvs::RGBColor& color1, kvs::RGBColor& color2 )
////{
////    const kvs::RGBColor temp = color1;
////    color1 = color2;
////    color2 = temp;
////}

/////*===========================================================================*/
/////**
//// *  @brief  Retruns the darkened color.
//// *  @param  color [in] source color
//// *  @param  darkness [in] coefficient value [0,1]
//// *  @return darkened color
//// */
/////*===========================================================================*/
////kvs::RGBColor WidgetBase::get_darkened_color( const kvs::RGBColor& color, const float darkness )
////{
////    kvs::HSVColor hsv( color );
////    hsv.set( hsv.h(), hsv.s(), hsv.v() * darkness );

////    return( kvs::RGBColor( hsv ) );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the fitted width.
//// *  @return fitted width
//// */
/////*===========================================================================*/
////int WidgetBase::get_fitted_width( void )
////{
////    return( 0 );
////}

/////*===========================================================================*/
/////**
//// *  @brief  Returns the fitted height.
//// *  @return fitted height
//// */
/////*===========================================================================*/
////int WidgetBase::get_fitted_height( void )
////{
////    return( 0 );
////}


////void WidgetBase::mouseMoveEvent(QMouseEvent *event){
//////    //noqinfo// qinfo("mouse event %i %i %i",event->x(),event->y(),event->buttons());
////    if (event->button() ==1){
////        this->m_active_mouse_pos=event->pos();
////    }
////}

//}

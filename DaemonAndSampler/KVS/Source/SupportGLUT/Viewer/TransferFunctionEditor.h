/*****************************************************************************/
/**
 *  @file   TransferFunctionEditor.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunctionEditor.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GLUT__TRANSFER_FUNCTION_EDITOR_H_INCLUDE
#define KVS__GLUT__TRANSFER_FUNCTION_EDITOR_H_INCLUDE

#include <string>
#include <kvs/ClassName>
#include <kvs/OpenGL>
#include <kvs/RGBColor>
#include <kvs/RGBAColor>
#include <kvs/Vector2>
#include <kvs/TransferFunction>
#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/MouseReleaseEventListener>
#include <kvs/glut/Screen>
#include <kvs/glut/PushButton>
#include <kvs/glut/Text>
#include <kvs/glut/ColorPalette>
#include <kvs/glut/ColorMapPalette>
#include <kvs/glut/OpacityMapPalette>
#include <kvs/glut/Histogram>


namespace kvs
{

namespace glut
{

/*===========================================================================*/
/**
 *  @brief  Transfer function editor class.
 */
/*===========================================================================*/
class TransferFunctionEditor : public kvs::glut::Screen
{
    kvsClassName( kvs::glut::TransferFunctionEditor );

public:

    typedef kvs::glut::Screen SuperClass;

private:

    class StackEvent;

    std::deque<kvs::TransferFunction> m_undo_stack;     ///< undo stack
    std::deque<kvs::TransferFunction> m_redo_stack;     ///< redo stack
    StackEvent*                       m_stack_event;    ///< stack event
    size_t                            m_max_stack_size; ///< max. stack size

protected:

    kvs::ScreenBase*              m_screen;              ///< pointer to screen
    kvs::glut::ColorPalette*      m_color_palette;       ///< color palette
    kvs::glut::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    kvs::glut::OpacityMapPalette* m_opacity_map_palette; ///< opacity map palette
    kvs::glut::Histogram*         m_histogram;           ///< histogram
    kvs::glut::PushButton*        m_reset_button;        ///< reset button
    kvs::glut::PushButton*        m_undo_button;         ///< undo button
    kvs::glut::PushButton*        m_redo_button;         ///< redo button
    kvs::glut::PushButton*        m_save_button;         ///< save button
    kvs::glut::PushButton*        m_apply_button;        ///< apply button
    kvs::TransferFunction m_initial_transfer_function;   ///< initial transfer function
    kvs::Real32           m_min_value; ///< min value
    kvs::Real32           m_max_value; ///< max value

public:

    TransferFunctionEditor( kvs::ScreenBase* parent = 0 );

    virtual ~TransferFunctionEditor( void );

public:

    kvs::ScreenBase* screen( void );

    const kvs::glut::ColorPalette* colorPalette( void ) const;

    const kvs::glut::ColorMapPalette* colorMapPalette( void ) const;

    const kvs::glut::OpacityMapPalette* opacityMapPalette( void ) const;

    const kvs::ColorMap colorMap( void ) const;

    const kvs::OpacityMap opacityMap( void ) const;

    const kvs::TransferFunction transferFunction( void ) const;

    const size_t undoStackSize( void ) const;

    const size_t redoStackSize( void ) const;

    const size_t maxStackSize( void ) const;

public:

    void setTransferFunction( const kvs::TransferFunction& transfer_function );

    void setVolumeObject( const kvs::VolumeObjectBase* object );

    void setMaxStackSize( const size_t stack_size );

public:

    virtual void reset( void );

    virtual void undo( void );

    virtual void redo( void );

    virtual void save( void );

    virtual void apply( void );
};

/*===========================================================================*/
/**
 *  @brief  Stack event for undo/redo.
 */
/*===========================================================================*/
class TransferFunctionEditor::StackEvent : public kvs::MouseReleaseEventListener
{
    kvs::glut::TransferFunctionEditor* m_editor;

public:

    StackEvent( kvs::glut::TransferFunctionEditor* editor );

    void update( kvs::MouseEvent* event );
};




namespace old
{

/*===========================================================================*/
/**
 *  @brief  TransferFunctionEditor class.
 */
/*===========================================================================*/
class TransferFunctionEditor : public kvs::TransferFunction
{
    typedef kvs::TransferFunction BaseClass;

public:

    class Mouse;
    class Widget;

protected:

    int m_id; ///< window ID
    size_t m_x; ///< window x position
    size_t m_y; ///< window y position
    size_t m_width; ///< window width
    size_t m_height; ///< window height
    std::string m_title; ///< window title
    kvs::RGBColor m_background_color; ///< background color
    Mouse* m_mouse; ///< mouse for transfer function editor
    Widget* m_SV_palette; ///< SV (saturation and value) palette
    Widget* m_H_palette; ///< H (hue) palette
    Widget* m_selected_color_box; ///< seleceted RGB color box
    Widget* m_color_map_box; ///< color map box
    Widget* m_opacity_map_box; ///< opacity map box

    kvs::Texture1D m_color_map_texture; ///< color map texture
    kvs::Texture1D m_opacity_map_texture; ///< opacity map texture
    kvs::Texture2D m_checkerboard_texture; ///< checkerboard texture

    kvs::glut::Text m_SV_palette_label; ///< SV palette label
    kvs::glut::Text m_color_map_label; ///< color map label
    kvs::glut::Text m_selected_color_value; ///< selected color value
    kvs::glut::Text m_color_map_value; ///< color value on the color and opacity map

    int m_H_cursor; ///< H (hue) indicator
    int m_S_cursor; ///< S (saturation) indicator
    int m_V_cursor; ///< V (value) indicator
    int m_scalar_cursor; ///< scalar value (field value) indicator

protected:

    friend void initialize_function( void );
    friend void paint_event( void );
    friend void resize_event( int width, int height );
    friend void mouse_press_event( int button, int state, int x, int y );
    friend void mouse_move_event( int x, int y );

public:

    TransferFunctionEditor( void );

    virtual ~TransferFunctionEditor( void );

public:

    void create( const bool show = false );

    void setPosition( const size_t x, const size_t y );

    void setBackgroundColor( const kvs::RGBColor color );

    void hideWindow( void );

    void showWindow( void );

protected:

    void lay_out_widgets( void );

protected:

    void draw_SV_palette( void );

    void draw_H_palette( void );

    void draw_selected_color_box( void );

    void draw_color_map_box( void );

    void draw_opacity_map_box( void );

    void draw_H_cursor( void );

    void draw_S_cursor( void );

    void draw_V_cursor( void );

    void draw_scalar_cursor( void );

    void draw_selected_color_value( void );

    void draw_color_map_value( void );

protected:

    void press_SV_palette( void );

    void press_H_palette( void );

    void press_color_map_box( void );

    void press_opacity_map_box( void );

protected:

    void move_SV_palette( void );

    void move_H_palette( void );

    void move_color_map_box( void );

    void move_opacity_map_box( void );

protected:

    void initialize_color_map_texture( void );

    void initialize_opacity_map_texture( void );

    void initialize_checkerboard_texture( void );

protected:

    const kvs::RGBColor get_selected_color( void );
};

/*===========================================================================*/
/**
 *  @brief  Mouse class for transfer function editor.
 */
/*===========================================================================*/
class TransferFunctionEditor::Mouse
{
protected:

    kvs::Vector2i m_previous; ///< previous mouse position
    kvs::Vector2i m_current; ///< current mouse position

public:

    Mouse( void );

public:

    const kvs::Vector2i previous( void ) const;

    const kvs::Vector2i current( void ) const;

    void press( const int x, const int y );

    void move( const int x, const int y );

    void stop( void );
};

/*===========================================================================*/
/**
 *  @brief  Widget class for transfer function editor
 */
/*===========================================================================*/
class TransferFunctionEditor::Widget
{
protected:

    int m_x; ///< x position of widget
    int m_y; ///< y position of widet
    int m_width; ///< widget width
    int m_height; ///< widget height
    bool m_is_active; ///< check flag for widget activation

public:

    Widget( void );

    Widget( const int x, const int y, const int width, const int height, const bool active = false );

public:

    const int x( void ) const;

    const int y( void ) const;

    const int width( void ) const;

    const int height( void ) const;

    const int x0( void ) const;

    const int y0( void ) const;

    const int x1( void ) const;

    const int y1( void ) const;

    const bool isActive( void ) const;

public:

    void setX( const int x );

    void setY( const int y );

    void setWidth( const int width );

    void setHeight( const int height );

public:

    void activate( void );

    void deactivate( void );

    const bool contains( const int x, const int y, const bool proper = false );
};

} // end of namespace old

} // end of namespace glut

} // end of namespace kvs

#endif // KVS__GLUT__TRANSFER_FUNCTION_EDITOR_H_INCLUDE


/*****************************************************************************/
/**
 *  @file   OpacityMapPalette.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpacityMapPalette.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef QGLUE_OPACITY_MAP_PALETTE_H_INCLUDE
#define QGLUE_OPACITY_MAP_PALETTE_H_INCLUDE

#include <kvs/OpenGL>

#include <QGlue/colormappalette.h>

#include <string>
#include <kvs/ClassName>
#include <kvs/Texture1D>
#include <kvs/Texture2D>
#include <kvs/OpacityMap>

//ADD BY)T.Osaki 2020.02.28
#include <QUndoStack>

namespace QGlue
{


class OpacityMapPalette : public QGLUEBaseWidget
{


public:

    typedef QGLUEBaseWidget BaseClass;
    //ADD BY)T.Osaki 2020.02.04
    void resetOpacityMapPalette( void );
    kvs::OpacityMap      m_opacity_map_initial;      ///< opacity map initial

protected:
    bool                 m_palette_active;
    size_t              m_resolution;
    //    MOD BY)T.Osaki 2020.04.28
    float                 pixelRatio=1;
    std::string          m_caption;          ///< caption
    kvs::OpacityMap      m_opacity_map;      ///< opacity map

    //ADD BY)T.Osaki 2020.02.28
    kvs::OpacityMap      m_fromOpacityMap;

    DeferredTexture1D    m_texture;          ///< opacity map texture
    DeferredTexture2D    m_checkerboard;     ///< checkerboard texture
    QRect                m_palette;          ///< palette
    QColor               m_upper_edge_color; ///< upper edge color
    QColor               m_lower_edge_color; ///< lower edge color
    kvs::Vector2i        m_pressed_position; ///< mouse pressed position
    kvs::Vector2i        m_previous_position; ///< mouse previous position
    const QGlue::ColorMapPalette* m_color_map_palette; ///< pointer to the color map palette

    static int           InstanceCounter;

    //ADD BY)T.Osaki 2020.02.28
public:
    QUndoStack *m_cundoStack = NULL;

public:

    OpacityMapPalette( QWidget* parent = 0 );

    virtual ~OpacityMapPalette( void );

public:

    virtual void screenUpdated( void ) {};

    virtual void screenResized( void ) {};

public:

    const std::string& caption( void ) const;

    const QRect& palette( void ) const;

    const kvs::OpacityMap opacityMap( void ) const;

public:

    void setEditable(bool editable);

    void setCaption( const std::string& caption );

    uint setOpacityExpression(std::string ofe);

    void setOpacityMap( const kvs::OpacityMap& opacity_map );

public:

    void paintGL( void );

    void resizeGL( int width, int height );

    void mousePressEvent( QMouseEvent* event );

    void mouseMoveEvent( QMouseEvent* event );

    void mouseReleaseEvent(QMouseEvent* event );

protected:

    void initialize_texture( const kvs::OpacityMap& opacity_map );

    void initialize_checkerboard( void );

    void draw_palette( void );

    void draw_free_hand_line(QMouseEvent* event );

    void draw_straight_line( QMouseEvent* event );
};

} // end of namespace QGLUE

#endif // QGLUE_OPACITY_MAP_PALETTE_H_INCLUDE

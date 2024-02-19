#ifndef COLORMAPPALETTE_H
#define COLORMAPPALETTE_H

#include "colorpalette.h"
#include "qglue_base.h"

#include "deferredtexture.h"
#include <Client/ColorMapLibrary.h>
#include <kvs/ColorMap>
#include <kvs/Vector2>

// ADD BY)T.Osaki 2020.02.28
#include <QUndoStack>

namespace QGlue {

class ColorMapPalette : public QGLUEBaseWidget {

public:
  ColorMapPalette(QWidget *parent = 0);
  QStringList getLibraryEntries();
  void setEditable(bool m_editable, ColorPalette *color_palette);
  uint setColorMapEquation(std::string rfe, std::string gfe, std::string bfe);
  void setColorMap(const kvs::ColorMap &color_map);
  void setColorMapIndex(int val);
  // ADD BY)T.Osaki 2020.02.03
  void resetColorMapPalette(void);
  const kvs::ColorMap colorMap(void) const;

  virtual ~ColorMapPalette(void);

public:
  typedef QGLUEBaseWidget BaseClass;

  static const uint RED_EXPRESSION_ERROR = 1;
  static const uint GREEN_EXPRESSION_ERROR = 2;
  static const uint BLUE_EXPRESSION_ERROR = 4;
  // ADD BY)T.Osaki 2020.02.03
  kvs::ColorMap m_color_map_initial; /// color map initial    kvs::ColorMap        m_color_map_initial; ///color map initial

private:
  bool m_editable = true;
  bool m_palette_active;
  //    MOD BY)T.Osaki 2020.04.28
  float pixelRatio = 1;
  size_t m_resolution;

  std::string m_caption;     ///< caption
  kvs::ColorMap m_color_map; ///< color map

  // ADD BY)T.Osaki 2020.02.28
  kvs::ColorMap m_fromColorMap; ///< Undo from color map

  DeferredTexture1D m_texture;         ///< color map texture
  QRect m_palette;                     ///< palette
  QColor m_upper_edge_color;           ///< upper edge color
  QColor m_lower_edge_color;           ///< lower edge color
  kvs::RGBColor m_drawing_color;       ///< drawing color
  kvs::Vector2i m_pressed_position;    ///< mouse pressed position
  const ColorPalette *m_color_palette; ///< pointer to the color palette
  kvs::visclient::ColorMapLibrary *m_color_map_library;

  // ADD BY)T.Osaki 2020.02.28
public:
  QUndoStack *m_cundoStack = NULL;

private:
  virtual void screenUpdated(void){};
  virtual void screenResized(void){};

private:
  const std::string &caption(void) const;
  const QRect &palette(void) const;

private:
  void setCaption(const std::string &caption);
  void setDrawingColor(const kvs::RGBColor &color);
  void attachColorPalette(const ColorPalette *palette);
  void detachColorPalette(void);

private:
  void paintGL(void);
  void resizeGL(int w, int h);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

private:
  void initialize_texture(const kvs::ColorMap &color_map);
  void draw_palette(void);
};

} // namespace QGlue

#endif // COLORMAPPALETTE_H

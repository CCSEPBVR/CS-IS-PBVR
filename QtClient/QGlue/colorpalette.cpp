#ifdef KVS_ENABLE_GLEW
#include <GL/glew.h>
#endif

#include "colorpalette.h"
#include <QApplication>
#include <QMouseEvent>
#include <kvs/RGBAColor>
// Default parameters.
namespace {
namespace Default {
const size_t Margin = 10;
size_t Width = 350;
size_t Height = 150;
int HPaletteWidth = 10;
int ColorBoxSize = 50;
const QColor RectColor = QColor(255, 255, 255);
const QColor RectEdgeColor = QColor(230, 230, 230);
} // namespace Default
} // namespace

// Instance counter.
static int InstanceCounter = 0;

namespace {

/*===========================================================================*/
/**
 *  @brief  Interpolates color values.
 *  @param  ratio [in] ratio [0-1]
 *  @param  c0 [in] color value 0
 *  @param  c1 [in] color value 1
 *  @return interpolated color value
 */
/*===========================================================================*/
const kvs::RGBColor GetInterpolatedColor(const float ratio, const kvs::RGBColor &c0, const kvs::RGBColor &c1) {
  const kvs::UInt8 r = static_cast<kvs::UInt8>(ratio * c0.r() + (1.0f - ratio) * c1.r() + 0.5f);
  const kvs::UInt8 g = static_cast<kvs::UInt8>(ratio * c0.g() + (1.0f - ratio) * c1.g() + 0.5f);
  const kvs::UInt8 b = static_cast<kvs::UInt8>(ratio * c0.b() + (1.0f - ratio) * c1.b() + 0.5f);

  return (kvs::RGBColor(r, g, b));
}

} // end of namespace

namespace QGlue {

ColorPalette::ColorPalette(QWidget *parent) : QGLUEBaseWidget(parent), m_H_indicator(-1), m_S_indicator(-1), m_V_indicator(-1) {
  this->setAutoFillBackground(false);
  //    MOD BY)T.Osaki 2020.04.28
  // pixelRatio = QPaintDevice::devicePixelRatioF();
  pixelRatio = pixelRatio = parent->screen()->devicePixelRatio();
}

ColorPalette::~ColorPalette(void) {}

/**
 * @brief ColorPalette::color
 * @return
 */
const kvs::RGBColor ColorPalette::color(void) const {
  const float h = this->get_H_value();
  const kvs::RGBColor c0(kvs::HSVColor(h, 1, 1)); // top-right
  const kvs::RGBColor c1(kvs::HSVColor(h, 0, 1)); // top-left
  const kvs::RGBColor c2(kvs::HSVColor(h, 0, 0)); // bottom-left
  const kvs::RGBColor c3(kvs::HSVColor(h, 1, 0)); // bottom-right

  const float v = this->get_V_value();
  const kvs::RGBColor v0(::GetInterpolatedColor(v, c1, c2));
  const kvs::RGBColor v1(::GetInterpolatedColor(v, c0, c3));

  const float s = this->get_S_value();
  return (::GetInterpolatedColor(s, v1, v0));
}

/**
 * @brief GetInterpolatedColor
 * @param ratio
 * @param c0
 * @param c1
 * @return
 */
const kvs::RGBColor GetInterpolatedColor(const float ratio, const kvs::RGBColor &c0, const kvs::RGBColor &c1) {
  const kvs::UInt8 r = static_cast<kvs::UInt8>(ratio * c0.r() + (1.0f - ratio) * c1.r() + 0.5f);
  const kvs::UInt8 g = static_cast<kvs::UInt8>(ratio * c0.g() + (1.0f - ratio) * c1.g() + 0.5f);
  const kvs::UInt8 b = static_cast<kvs::UInt8>(ratio * c0.b() + (1.0f - ratio) * c1.b() + 0.5f);

  return (kvs::RGBColor(r, g, b));
}

/*===========================================================================*/
/**
 *  @brief resizeGL
 */
/*===========================================================================*/
void ColorPalette::resizeGL(int w, int h) {
  int h_scaled = h * pixelRatio;
  int w_scaled = w * pixelRatio;

  // Draw the caption.
  {
    const int x = BaseClass::x0() + m_margin;
    const int y = BaseClass::y0() + m_margin;
    //        BaseClass::draw_text( x, y + CharacterHeight, m_caption );
    //        BaseClass::drawText(10,10,"TESTTESTTEST");
  }

  // Selected color box.
  {
    const int x = BaseClass::x1() * pixelRatio - m_margin - ::Default::ColorBoxSize;
    const int y = BaseClass::y0() + m_margin + CharacterHeight + 5;
    const int width = ::Default::ColorBoxSize * pixelRatio;
    const int height = ::Default::ColorBoxSize * pixelRatio;
    m_selected_color_box.setRect(x, y, width, height);
  }

  // H palette.
  {
    const int x = m_selected_color_box.x() - ::Default::HPaletteWidth - 10;
    const int y = BaseClass::y0() + m_margin + CharacterHeight + 5;
    const int width = ::Default::HPaletteWidth * pixelRatio;
    const int height = this->height() * pixelRatio - m_margin * 2 - CharacterHeight;
    m_H_palette.setRect(x, y, width, height);

    // Initialize the H indicator.
    if (m_H_indicator < 0)
      m_H_indicator = m_H_palette.y() + m_H_palette.height() / 2;
  }

  // SV palette
  {
    const int x = BaseClass::x0() + m_margin;
    const int y = BaseClass::y0() + m_margin + CharacterHeight + 5;
    const int width = m_H_palette.x() - BaseClass::x0() - m_margin * 2;
    const int height = this->height() * pixelRatio - m_margin * 2 - CharacterHeight;
    m_SV_palette.setRect(x, y, width, height);

    // Initialize the S and V indicator.
    if (m_S_indicator < 0)
      m_S_indicator = m_SV_palette.x() + m_SV_palette.width() * pixelRatio / 2;
    if (m_V_indicator < 0)
      m_V_indicator = m_SV_palette.y() + m_SV_palette.height() * pixelRatio / 2;
  }
}

/**
 * @brief ColorPalette::paintGL
 */
void ColorPalette::paintGL(void) {

  if (!isVisible())
    return;

  BaseClass::begin_draw();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  this->draw_SV_palette();
  this->draw_H_palette();
  this->draw_selected_color_box();

  this->draw_selected_color_value();
  BaseClass::end_draw();
}

/**
 * @brief ColorPalette::get_H_value
 * @return
 */
const float ColorPalette::get_H_value(void) const {
  return (static_cast<float>((m_H_palette.y() + m_H_palette.height()) - m_H_indicator) / m_H_palette.height());
}

/**
 * @brief ColorPalette::get_S_value
 * @return
 */
const float ColorPalette::get_S_value(void) const { return (static_cast<float>(m_S_indicator - m_SV_palette.x()) / m_SV_palette.width()); }

/**
 * @brief ColorPalette::get_V_value
 * @return
 */
const float ColorPalette::get_V_value(void) const {
  return (static_cast<float>((m_SV_palette.y() + m_SV_palette.height()) - m_V_indicator) / m_SV_palette.height());
}

/**
 * @brief ColorPalette::draw_SV_palette
 */
void ColorPalette::draw_SV_palette(void) {
  const int x0 = m_SV_palette.x();
  const int y0 = m_SV_palette.y();
  const int x1 = m_SV_palette.x() + m_SV_palette.width();
  const int y1 = m_SV_palette.y() + m_SV_palette.height();

  const float h = this->get_H_value();
  const kvs::RGBColor c0(kvs::HSVColor(h, 1, 1)); // top-right
  const kvs::RGBColor c1(kvs::HSVColor(h, 0, 1)); // top-left
  const kvs::RGBColor c2(kvs::HSVColor(h, 0, 0)); // bottom-left
  const kvs::RGBColor c3(kvs::HSVColor(h, 1, 0)); // bottom-right

  // Draw SV palette.
  glBegin(GL_QUADS);
  glColor3ub(c0.r(), c0.g(), c0.b());
  glVertex2i(x1, y0);
  glColor3ub(c1.r(), c1.g(), c1.b());
  glVertex2i(x0, y0);
  glColor3ub(c2.r(), c2.g(), c2.b());
  glVertex2i(x0, y1);
  glColor3ub(c3.r(), c3.g(), c3.b());
  glVertex2i(x1, y1);
  glEnd();

  // Draw the cross lines.
  {
    const int margin = 4;
    const float color = 1.0f;
    const int cursor_x0 = kvs::Math::Max(m_S_indicator - margin, m_SV_palette.x());
    const int cursor_y0 = kvs::Math::Max(m_V_indicator - margin, m_SV_palette.y());
    const int cursor_x1 = kvs::Math::Min(m_S_indicator + margin, m_SV_palette.x() + m_SV_palette.width());
    const int cursor_y1 = kvs::Math::Min(m_V_indicator + margin, m_SV_palette.y() + m_SV_palette.height());

    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(color, color, color);
    glVertex2i(cursor_x0, cursor_y0);
    glVertex2i(cursor_x1, cursor_y0); // top
    glVertex2i(cursor_x0, cursor_y1);
    glVertex2i(cursor_x1, cursor_y1); // bottom
    glVertex2i(cursor_x0, cursor_y0);
    glVertex2i(cursor_x0, cursor_y1 + 1); // left
    glVertex2i(cursor_x1, cursor_y0);
    glVertex2i(cursor_x1, cursor_y1 + 1); // right
    glEnd();

    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(color, color, color);
    // Horizontal lines.
    glVertex2i(x0, m_V_indicator);
    glVertex2i(cursor_x0, m_V_indicator);
    glVertex2i(cursor_x1, m_V_indicator);
    glVertex2i(x1, m_V_indicator);
    // Vertical lines.
    glVertex2i(m_S_indicator, y0);
    glVertex2i(m_S_indicator, cursor_y0);
    glVertex2i(m_S_indicator, cursor_y1);
    glVertex2i(m_S_indicator, y1);
    glEnd();
  }

  // Draw border.
  this->drawRectangle(m_SV_palette, 1, m_upper_edge_color, m_lower_edge_color);
}

/**
 * @brief ColorPalette::draw_H_palette
 */
void ColorPalette::draw_H_palette(void) {
  const int x0 = m_H_palette.x();
  const int y0 = m_H_palette.y();
  const int x1 = m_H_palette.x() + m_H_palette.width();
  const int y1 = m_H_palette.y() + m_H_palette.height();

  const int stride = (y1 - y0) / 6;
  const int dx = m_H_palette.width() / 2;

  // Draw H palette.
  glLineWidth(static_cast<GLfloat>(m_H_palette.width()));
  // clang-format off
  glBegin(GL_LINE_STRIP);
  glColor3ub(255,   0,   0);  glVertex2i(x0 + dx, y1);
  glColor3ub(255, 255,   0);  glVertex2i(x0 + dx, y0 + stride * 5);
  glColor3ub(  0, 255,   0);  glVertex2i(x0 + dx, y0 + stride * 4);
  glColor3ub(  0, 255, 255);  glVertex2i(x0 + dx, y0 + stride * 3);
  glColor3ub(  0,   0, 255);  glVertex2i(x0 + dx, y0 + stride * 2);
  glColor3ub(255,   0, 255);  glVertex2i(x0 + dx, y0 + stride * 1);
  glColor3ub(255,   0,   0);  glVertex2i(x0 + dx, y0);
  glEnd();
  // clang-format on

  // Draw the line
  const float color = 1.0f;
  glLineWidth(1);
  glBegin(GL_LINES);
  glColor3f(color, color, color);
  glVertex2i(x0, m_H_indicator);
  glVertex2i(x1, m_H_indicator);
  glEnd();

  // Draw border.
  this->drawRectangle(m_H_palette, 1, m_upper_edge_color, m_lower_edge_color);
}

/**
 * @brief ColorPalette::draw_selected_color_box
 */
void ColorPalette::draw_selected_color_box(void) {
  const int x0 = m_selected_color_box.x();
  const int y0 = m_selected_color_box.y();
  const int x1 = m_selected_color_box.x() + m_selected_color_box.width();
  const int y1 = m_selected_color_box.y() + m_selected_color_box.height();

  const kvs::RGBColor current_color = this->color();

  glBegin(GL_QUADS);
  glColor3ub(current_color.r(), current_color.g(), current_color.b());
  glVertex2i(x1, y1);
  glVertex2i(x0, y1);
  glVertex2i(x0, y0);
  glVertex2i(x1, y0);
  glEnd();

  // Draw border.
  this->drawRectangle(m_selected_color_box, 1, m_upper_edge_color, m_lower_edge_color);
}

/**
 * @brief ColorPalette::draw_selected_color_value
 */
void ColorPalette::draw_selected_color_value(void) {
  const kvs::RGBColor current_color = this->color();
  const int r = static_cast<int>(current_color.r());
  const int g = static_cast<int>(current_color.g());
  const int b = static_cast<int>(current_color.b());
  const float h = this->get_H_value();
  const float s = this->get_S_value();
  const float v = this->get_V_value();

  // kawamura
  char R[10];
  sprintf(R, "R:%4.2f", (float)r / 255);
  char G[10];
  sprintf(G, "G:%4.2f", (float)g / 255);
  char B[10];
  sprintf(B, "B:%4.2f", (float)b / 255);
  /*  char R[10]; sprintf( R, "R: %3d", r );
  char G[10]; sprintf( G, "G: %3d", g );
  char B[10]; sprintf( B, "B: %3d", b );*/

  char H[10];
  sprintf(H, "H: %3d", static_cast<int>(h * 255.0f + 0.5f));
  char S[10];
  sprintf(S, "S: %3d", static_cast<int>(s * 255.0f + 0.5f));
  char V[10];
  sprintf(V, "V: %3d", static_cast<int>(v * 255.0f + 0.5f));
  // MOD BY)T.Osaki 2020.04.24
  int x = m_selected_color_box.x() / pixelRatio;
  int y = (m_selected_color_box.y() + m_selected_color_box.height() + 10) / pixelRatio;
  BaseClass::drawText(x, y += CharacterHeight, R);
  BaseClass::drawText(x, y += CharacterHeight, G);
  BaseClass::drawText(x, y += CharacterHeight, B);
  BaseClass::drawText(x, y += CharacterHeight, "");
  BaseClass::drawText(x, y += CharacterHeight, H);
  BaseClass::drawText(x, y += CharacterHeight, S);
  BaseClass::drawText(x, y += CharacterHeight, V);
}

/**
 * @brief ColorPalette::mousePressEvent
 * @param event
 */
void ColorPalette::mousePressEvent(QMouseEvent *event) {
  //    MOD BY)T.Osaki 2020.04.28
  const int x = event->x() * pixelRatio;
  const int y = event->y() * pixelRatio;
  // MOD BY)T.Osaki 2020.04.24
  if (isVisible()) {
    //        if ( m_SV_palette.isActive() )
    if (m_SV_palette.contains(event->x() * pixelRatio, event->y() * pixelRatio)) {
      m_S_indicator = kvs::Math::Clamp(x, m_SV_palette.x(), m_SV_palette.x() + m_SV_palette.width());
      m_V_indicator = kvs::Math::Clamp(y, m_SV_palette.y(), m_SV_palette.y() + m_SV_palette.height());
    }

    //        if ( m_H_palette.isActive() )
    if (m_H_palette.contains(event->x() * pixelRatio, event->y() * pixelRatio)) {
      m_H_indicator = kvs::Math::Clamp(y, m_H_palette.y(), m_H_palette.y() + m_H_palette.height());
    }
  }
  update();
}

/**
 * @brief ColorPalette::mouseMoveEvent
 * @param event
 */
void ColorPalette::mouseMoveEvent(QMouseEvent *event) {
  //    MOD BY)T.Osaki 2020.04.28
  const int x = event->x() * pixelRatio;
  const int y = event->y() * pixelRatio;
  // MOD BY)T.Osaki 2020.04.24
  if (isVisible()) {
    //        if ( m_SV_palette.isActive() )
    if (m_SV_palette.contains(event->x() * pixelRatio, event->y() * pixelRatio)) {
      m_S_indicator = kvs::Math::Clamp(x, m_SV_palette.x(), m_SV_palette.x() + m_SV_palette.width());
      m_V_indicator = kvs::Math::Clamp(y, m_SV_palette.y(), m_SV_palette.y() + m_SV_palette.height());
    }

    //        if ( m_H_palette.isActive() )
    if (m_H_palette.contains(event->x() * pixelRatio, event->y() * pixelRatio)) {
      m_H_indicator = kvs::Math::Clamp(y, m_H_palette.y(), m_H_palette.y() + m_H_palette.height());
    }
  }
  update();
}
} // namespace QGlue

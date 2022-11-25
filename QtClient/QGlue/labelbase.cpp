//KVS2.7.0
//ADD BY)T.Osaki 2020.06.08
#include <QOpenGLContext>

#include "labelbase.h"
#include <QGlue/labelbase.h>
//#include <QGlue/screen.h>
#include <Panels/timecontrolpanel.h>
#include <QGlue/renderarea.h>
#include <QApplication>
#include <QDesktopWidget>

namespace QGlue
{

Label::Label(RenderArea *screen)
{
    this->m_screen=screen;
    //    MOD BY)T.Osaki 2020.04.28
    pixelRatio=QApplication::desktop()->devicePixelRatioF();
    this->setText(QString("#"));
    this->updateBitmap();
}
/*===========================================================================*/
/**
 * @brief Label::setPosition
 * @param x
 * @param y
 */
/*===========================================================================*/
void Label::setPosition( int x, int y)
{
    this->move(x,y);
}
/*===========================================================================*/
/**
 * @brief Label::setText sets the label text and optionally updates text color
 * @param text
 * @param textColor
 */
/*===========================================================================*/
void Label::setText(const QString &text, QColor textColor){
    QLabel::setText(text);
    Label::updateBitmap(textColor);
}
/*===========================================================================*/
/**
 * @brief Label::getText
 * @return
 */
/*===========================================================================*/
const std::string Label::getText()
{
    return this->text().toStdString();
}
/*===========================================================================*/
/**
 * @brief Label::nextPower2
 * Good-Old Stanford graphics bit twiddle hack to find next power of 2
 * @param size
 * @return
 */
/*===========================================================================*/
uint Label::nextPower2(uint size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return ++size;
}
/*===========================================================================*/
/**
 * @brief Label::updateBitmap , updates the bitmap with the current label text
 * @param textColor
 */
/*===========================================================================*/
void Label::updateBitmap(QColor textColor)
{
    QString lbl_text=this->text();
    QFont f=this->font();
//    f.setPointSize(33);
    //f.setPixelSize(44);
    int oldSize = f.pointSize();
    int newSize = oldSize* pixelRatio;
    f.setPointSize(newSize );
    QFontMetrics fm(f);
    bitmap_w=nextPower2(fm.boundingRect(lbl_text).width());
    bitmap_h=nextPower2(fm.boundingRect(lbl_text).height());

    bitmap_w=std::max(bitmap_w,4u);
    bitmap_h=std::max(bitmap_h,4u);

    bitmap.resize(bitmap_w*bitmap_h*4u);

    QImage qimg(&bitmap[0],bitmap_w,bitmap_h,QImage::Format_RGBA8888);
    qimg.fill(Qt::black);
    qimg.fill(Qt::transparent);

    QPainter pnt(&qimg);
    pnt.setRenderHint(QPainter::Antialiasing, true);
    pnt.setFont(f);
    pnt.setPen(QPen(textColor, 2));
    pnt.drawText(QRect(0, 0,bitmap_w,bitmap_h), lbl_text);
    pnt.end();
}
/*===========================================================================*/
/**
 * @brief Label::renderBitMap renders the bitmap using glDrawPixels
 * @param x
 * @param y
 * @param z
 */
/*===========================================================================*/
void Label::renderBitMap(float x, float y,float z)
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(   GL_GREATER, 0.5);
    glRasterPos3d( x,y,z );
    glPixelZoom(1.0,-1.0);        //QImage bitmap has flipped Y axis
    glDrawPixels(bitmap_w,bitmap_h,GL_RGBA,GL_UNSIGNED_BYTE, &bitmap[0]);
    glPixelZoom(1.0,1.0);
    glPopAttrib();
}

/**
 * @brief FPSLabel::FPSLabel
 * @param screen
 * @param renderer
 */
FPSLabel::FPSLabel(RenderArea *screen,  const kvs::RendererBase& renderer ):
    Label(screen),
    m_renderer(&renderer)
{

}
/*===========================================================================*/
/**
 * @brief FPSLabel::contentUpdate
 */
/*===========================================================================*/
void FPSLabel::contentUpdate( )
{
    QString fps;
    if(m_renderer !=NULL){
//        fps= "fps:" + QString::number( m_renderer->timer().fps(),'G',4);
        fps= "fps:" + QString::number( m_screen->m_fps,'G',4);
    }
    else {
        fps= "fps:---";
    }
    Label::setText(fps, Qt::white);
}
/*===========================================================================*/
/**
 * @brief StepLabel::StepLabel
 * @param screen
 * @param command
 */
/*===========================================================================*/
StepLabel::StepLabel(RenderArea *screen,  ExtCommand* command  ):
    Label(screen)
{
    (void)command;
}
/*===========================================================================*/
/**
 * @brief StepLabel::contentUpdate
 */
/*===========================================================================*/
void StepLabel::contentUpdate()
{
#ifdef CS_MODE
    QString step;
    if ( TimecontrolPanel::g_curStep < 0 )
        step="---";
    else
        step = "time step: " + QString::number( TimecontrolPanel::g_curStep,'G',4);
#else
    QString step;
    if ( extCommand->m_result.serverTimeStep < 0 )
        step="---";
    else
        step = "time step: " + QString::number( extCommand->m_result.serverTimeStep,'G',4);
#endif
    Label::setText(step, Qt::white );
}
} //end namspace QGLUE


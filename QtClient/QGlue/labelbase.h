#ifndef QGLUE_LABEL_BASE
#define QGLUE_LABEL_BASE

#include <QLabel>
#include <kvs/RendererBase>
#include <QGlue/extCommand.h>

namespace QGlue
{
class Screen;
/*===========================================================================*/
/**
 *  @brief  Label class.
 */
/*===========================================================================*/
class Label : public QLabel
{
    uint bitmap_h;
    uint bitmap_w;
    std::vector<uchar> bitmap;
public:
    Label(RenderArea* screen=NULL);
    void setPosition(int x, int y);
//    void drawText(QPainter* painter);

    virtual void contentUpdate(){}
    void updateBitmap(QColor textColor=Qt::black);
    void renderBitMap(float x, float y, float z);
    uint nextPower2(uint size);
    void setText(const QString &,QColor textColor=Qt::black);
    const std::string getText();
protected:
    RenderArea* m_screen;
    //    MOD BY)T.Osaki 2020.04.28
    float pixelRatio=1;
};

class FPSLabel: public Label
{
private:
     const kvs::RendererBase* m_renderer;

public:
     FPSLabel(RenderArea* screen,  const kvs::RendererBase& m_renderer );
     void contentUpdate();
};

class StepLabel: public Label
{
private:

public:
     StepLabel(RenderArea* screen,ExtCommand* m_command );
     void contentUpdate();
};


} // end of namespace QGLUE


#endif // QGLUE_LABEL_BASE

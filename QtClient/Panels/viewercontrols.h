#ifndef VIEWERCONTROLS_H
#define VIEWERCONTROLS_H

#include <QDialog>
#include "QGlue/colorpalette.h"
#include <kvs/RGBColor>

namespace Ui {
class ViewerControls;
}

class ViewerControls : public QDialog
{
//    Q_OBJECT

public:
    explicit ViewerControls(QWidget *parent = 0);
    ~ViewerControls();

    void recToggle(){ m_isRec ? m_isRec = false : m_isRec = true; }

    kvs::RGBColor selected_color;

    void setCurrentFont(const QFont &f);
    const QFont getFontSelection();
    void logPlay(std::string funcName, std::string str[]);
protected:
    QGlue::ColorPalette* m_color_palette;
private: // Event handlers
    void on_colorButton_clicked();



private:
    Ui::ViewerControls *ui;
    bool m_isRec = false;
};

#endif // VIEWERCONTROLS_H

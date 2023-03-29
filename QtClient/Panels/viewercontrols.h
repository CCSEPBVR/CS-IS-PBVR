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

    kvs::RGBColor selected_color;

    void setCurrentFont(const QFont &f);
    const QFont getFontSelection();
protected:
    QGlue::ColorPalette* m_color_palette;
private: // Event handlers
    void on_colorButton_clicked();



private:
    Ui::ViewerControls *ui;
};

#endif // VIEWERCONTROLS_H

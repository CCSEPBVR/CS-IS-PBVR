#ifndef COLOREDITDIALOG_H
#define COLOREDITDIALOG_H

#include <QDialog>
#include "QGlue/colorpalette.h"
#include <kvs/RGBColor>

namespace Ui {
class ColorEditDialog;
}

class ColorEditDialog : public QDialog
{

public:

    explicit ColorEditDialog(QWidget *parent = 0);

    void setColorSelection(kvs::RGBColor c){selected_color=c;}
    kvs::RGBColor color(){return selected_color;}
    kvs::RGBColor getColorSelection( );

    ~ColorEditDialog();

protected:

    QGlue::ColorPalette* m_color_palette;

private:

    Ui::ColorEditDialog *ui;

    kvs::RGBColor selected_color;
    kvs::RGBColor default_color;
};

#endif // COLOREDITDIALOG_H

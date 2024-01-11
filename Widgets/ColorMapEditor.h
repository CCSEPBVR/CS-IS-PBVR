#ifndef COLORMAPEDITOR_H
#define COLORMAPEDITOR_H

#include <QDialog>

namespace Ui {
class ColorMapEditor;
}

class ColorMapEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ColorMapEditor(QWidget *parent = nullptr);
    ~ColorMapEditor();

private:
    Ui::ColorMapEditor *ui;
    void readJsonFile();//Load Presets ColorMapBar

private slots:
    void onCurrentTabChanged( int index );
};

#endif // COLORMAPEDITOR_H

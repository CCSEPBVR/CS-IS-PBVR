#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include "Widgets/ColorMapEditor.h"
#include "Widgets/OpacityMapEditor.h"

namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr);
    ~TransferFunctionEditor();

private:
    Ui::TransferFunctionEditor *ui;
    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

private slots:
    void onColorMapEditorButtonClicked();
    void onOpacityMapEditorButtonClicked();
};

#endif // TRANSFERFUNCTIONEDITOR_H

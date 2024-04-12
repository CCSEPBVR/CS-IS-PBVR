#ifndef COLORMAPEDITOR_H
#define COLORMAPEDITOR_H

#include <QDialog>

#include <kvs/RGBColor>
#include <kvs/ColorMap>

#include <QUndoStack>

namespace Ui {
class ColorMapEditor;
}

class ColorMapEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ColorMapEditor(QWidget *parent = nullptr);
    ~ColorMapEditor();
    void setColorMap( kvs::ColorMap colorMap );
    void setInitialColorMap( kvs::ColorMap colorMap );
    kvs::ColorMap getColorMap();
    void clearUndoStack() { m_undo_stack->clear(); }

private:
    Ui::ColorMapEditor *ui;
    void readJsonFile();//Load Presets ColorMapBar
    QUndoStack *m_undo_stack;

private slots:
    void onCurrentTabChanged( int index );
    void onDrawingColorDoubleClicked();
    void onResetButtonClicked();
    void onColorMapBarTableWidgetCellDoubleClicked( int row, int column );
    void onExpressionChanged();
    void onNumberOfControlPointsChabged( int value );
    void onControlPointChanged();
};

#endif // COLORMAPEDITOR_H

#ifndef OPACITYMAPEDITOR_H
#define OPACITYMAPEDITOR_H

#include <QDialog>

#include <kvs/OpacityMap>

#include <QUndoStack>

namespace Ui {
class OpacityMapEditor;
}

class OpacityMapEditor : public QDialog
{
    Q_OBJECT

public:
    explicit OpacityMapEditor(QWidget *parent = nullptr);
    ~OpacityMapEditor();
    kvs::OpacityMap getOpacityMap();

private:
    Ui::OpacityMapEditor *ui;
    QUndoStack *m_undo_stack;

private slots:
    void onCurrentTabChanged( int index );
    void onExpressionChanged();
    void onNumberOfControlPointsChabged( int value );
    void onControlPointChanged();
};

#endif // OPACITYMAPEDITOR_H

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
    void setOpacityMap( kvs::OpacityMap opacityMap );
    void setInitialOpacityMap( kvs::OpacityMap opacityMap );
    kvs::OpacityMap getOpacityMap();
    void clearUndoStack() { m_undo_stack->clear(); }

private:
    Ui::OpacityMapEditor *ui;
    QUndoStack *m_undo_stack;

protected:
    void showEvent( QShowEvent* event ) override;

private slots:
    void onCurrentTabChanged( int index );
    void onResetButtonClicked();
    void onExpressionChanged();
    void onNumberOfControlPointsChabged( int value );
    void onControlPointChanged();
};

#endif // OPACITYMAPEDITOR_H

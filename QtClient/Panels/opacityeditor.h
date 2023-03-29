#ifndef OPACITYEDITOR_H
#define OPACITYEDITOR_H

#include <QDialog>
#include <QGlue/opacitymappalette.h>
#include <kvs/TransferFunction>

//ADD BY)T.Osaki 2020.02.28
#include <QUndoStack>
#include <QUndoView>

namespace Ui {
class OpacityEditor;
}

class OpacityEditor : public QDialog
{

public:

    explicit OpacityEditor(QWidget *parent = 0);
    ~OpacityEditor();

public:
    std::string     ofe; 
    kvs::Real32     m_min_value; ///< min value 
    kvs::Real32     m_max_value; ///< max value 

    QGlue::OpacityMapPalette*     m_opacity_map_palette;
    kvs::TransferFunction   m_initial_transfer_function;   ///< initial transfer function 

    size_t          m_resolution;

private:
    // Event handlers
    void onOpacityExpressionChanged(const QString &arg1);
    void onToolBoxChanged(int index);
    void onControlPointChanged(int row, int column);

private:
    Ui::OpacityEditor *ui; 
    //ADD BY)T.Osaki 2020.02.04
    void onResetButtonClicked();

    //ADD BY)T.Osaki 2020.02.28
    QUndoStack *m_undoStack;
};

#endif // OPACITYEDITOR_H

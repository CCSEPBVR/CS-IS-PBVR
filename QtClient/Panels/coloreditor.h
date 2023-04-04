#ifndef COLOREDITOR_H
#define COLOREDITOR_H

#include <QDialog>
#include <QGlue/colorpalette.h>
#include <QGlue/colormappalette.h>

//ADD BY)T.Osaki 2020.02.28
#include <QUndoStack>

namespace Ui {
class ColorEditor;
}

/**
 * @brief ColorEditor Dialog controller class
 *
 */
class ColorEditor : public QDialog
{
public:
    explicit ColorEditor(QWidget *parent = 0);
    ~ColorEditor();

public:
    QGlue::ColorMapPalette*   m_color_map_palette;


protected:

    bool m_visible;
    size_t m_resolution;

    std::string rfe;
    std::string gfe;
    std::string bfe;

    kvs::Real32         m_min_value;
    kvs::Real32         m_max_value;

    QGlue::ColorPalette*      m_color_palette;

    void onExpressionChange();
    void onControlPointChange(int row, int column);

    void onToolBoxChange(int index);
    void onColormapSelectionChange(int val);


private:
    Ui::ColorEditor *ui;
    void setColorMapEquation( );
    //ADD BY)T.Osaki 2020.01.29
    void onResetButtonClicked();

    //ADD BY)T.Osaki 2020.02.28
    QUndoStack *m_undoStack;
};

#endif // COLOREDITOR_H

#ifndef PLOTOVERLINEEDITOR_H
#define PLOTOVERLINEEDITOR_H

#include <QDockWidget>
#include "qcustomplot.h"

namespace Ui
{
class PlotOverLineEditor;
}

class PlotOverLineEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLineEditor( QWidget *parent = nullptr );
    ~PlotOverLineEditor();

private:
    // メンバ変数群
    Ui::PlotOverLineEditor *ui;
    QCustomPlot* m_q_custom_plot = nullptr;

    // メソッド群
    void initialize();
};

#endif // PLOTOVERLINEEDITOR_H

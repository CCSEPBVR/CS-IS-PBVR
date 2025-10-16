#ifndef PLOTOVERLINE_H
#define PLOTOVERLINE_H

#include <QDockWidget>

namespace Ui
{
class PlotOverLine;
}

class PlotOverLine : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLine( QWidget *parent = nullptr );
    ~PlotOverLine();

private:
    // メンバ変数群
    Ui::PlotOverLine *ui;

    // メソッド群
    void initialize();
};

#endif // PLOTOVERLINE_H

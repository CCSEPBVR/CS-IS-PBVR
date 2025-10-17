#ifndef POINTSIZECONTROL_H
#define POINTSIZECONTROL_H

#include <QDockWidget>

namespace Ui
{
class PointSizeControl;
}

class PointSizeControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit PointSizeControl( QWidget *parent = nullptr );
    ~PointSizeControl();

private:
    // メンバ変数群
    Ui::PointSizeControl *ui;

    // メソッド群
    void initialize();
};

#endif // POINTSIZECONTROL_H

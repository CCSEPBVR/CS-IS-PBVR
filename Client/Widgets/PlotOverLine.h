#ifndef PLOTOVERLINE_H
#define PLOTOVERLINE_H

#include <QDockWidget>

class Connect;

namespace Ui {
class PlotOverLine;
}

class PlotOverLine : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLine(QWidget *parent = nullptr, Connect* Connect = nullptr);
    ~PlotOverLine();

private:
    Ui::PlotOverLine *ui;
    Connect* m_connect; //クライアント/サーバメッセージ呼び出し用。

private slots:
    void onResolutionValueChanged(); //Resolutionのスピンボックスの値が変更された際に呼び出されるスロット
    void onStartPointAxisValueChanged(); //StartのXYZのダブルスピンボックスの値が変更された際に呼び出されるスロット
    void onEndPointAxisValueChanged(); //EndのXYZのダブルスピンボックスの値が変更された際に呼び出されるスロット
    void onApplyButtonClicked(); //applyボタンが押された際に呼び出されるスロット
};

#endif // PLOTOVERLINE_H

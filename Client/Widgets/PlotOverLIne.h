#ifndef PLOTOVERLINE_H
#define PLOTOVERLINE_H

#include <QDockWidget>
#include <kvs/ValueArray>
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
    void setPlotData( kvs::ValueArray<float> xAxis, kvs::ValueArray<bool> mask, kvs::ValueArray<float> values );

private:
    Ui::PlotOverLine *ui;
    Connect* m_connect; //クライアント/サーバメッセージ呼び出し用。

    //TESTDATA
    kvs::ValueArray<float> xAxis;
    kvs::ValueArray<bool>  mask;
    kvs::ValueArray<float> values;
    void initData();

    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

private slots:
    void onResetButtonClicked();
    void onResolutionValueChanged(); //Resolutionのスピンボックスの値が変更された際に呼び出されるスロット
    void onStartPointAxisValueChanged(); //StartのXYZのダブルスピンボックスの値が変更された際に呼び出されるスロット
    void onEndPointAxisValueChanged(); //EndのXYZのダブルスピンボックスの値が変更された際に呼び出されるスロット
    void onApplyButtonClicked(); //applyボタンが押された際に呼び出されるスロット
};

#endif // PLOTOVERLINE_H

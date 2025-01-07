#ifndef PLOTOVERLINE_H
#define PLOTOVERLINE_H

#include <QDockWidget>
#include <kvs/ValueArray>
#include <kvs/LineObject>
class PBVRGUI;
class Connect;

namespace Ui {
class PlotOverLine;
}

class PlotOverLine : public QDockWidget
{
    Q_OBJECT

public:
    explicit PlotOverLine(QWidget *parent = nullptr,
                          PBVRGUI *pbvr_gui = nullptr,
                          Connect* Connect = nullptr);
    ~PlotOverLine();
    void setPlotData( std::vector<float> xAxis, std::vector<bool> mask, std::vector<float> values );

private:
    Ui::PlotOverLine *ui;
    PBVRGUI* m_pbvr_gui;
    Connect* m_connect; //クライアント/サーバメッセージ呼び出し用。

    // kvs::ValueArray<float> xAxis; //for debug
    // kvs::ValueArray<bool>  mask; //for debug
    // kvs::ValueArray<float> values; //for debug

    // std::vector<float> m_x_axis; //for debug
    // std::vector<bool> m_mask; //for debug
    // std::vector<float> m_line_value; //for debug

    // void initData(); //for debug

    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;

    kvs::LineObject* m_pointer = nullptr;
    std::pair<int,int> m_pointer_id;

private slots:
    void onResetButtonClicked(); //グラフウィジェットを操作(移動、拡大)をリセットする。
    void updateXYZDoubleSpinBox(); //スピンボックスの値が変動した際に呼び出される。
    void onApplyButtonClicked(); //applyボタンが押された際に呼び出されるスロット
};

#endif // PLOTOVERLINE_H

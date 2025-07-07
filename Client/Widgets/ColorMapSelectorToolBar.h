#ifndef COLORMAPSELECTORTOOLBAR_H
#define COLORMAPSELECTORTOOLBAR_H

#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QStandardItem>

#include "Screen.h"
#include <kvs/ColorMapBar>

#include "TransferFunctionItem.h"

class ColorMapSelectorToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit ColorMapSelectorToolBar( kvs::qt::jaea::Screen*,
                                      kvs::ColorMapBar*,
                                      QWidget *parent = nullptr );
    ~ColorMapSelectorToolBar();

private:
    void initialize();

private:
    QLabel* m_color_function_label;
    QComboBox* m_color_function_combo_box;
    kvs::qt::jaea::Screen* m_screen                             = nullptr;
    kvs::ColorMapBar* m_color_map_bar       = nullptr;
    QStandardItemModel *m_model = nullptr;

public slots:
    void updateColorMapBar(QStandardItemModel* model );

private slots:
    void updateUIFromCurrentItem();

};

#endif // COLORMAPSELECTORTOOLBAR_H

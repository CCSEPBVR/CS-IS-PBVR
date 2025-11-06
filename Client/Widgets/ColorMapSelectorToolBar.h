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
    explicit ColorMapSelectorToolBar( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~ColorMapSelectorToolBar();

    void setColorMapBar( kvs::ColorMapBar* color_map_bar ) { m_color_map_bar = color_map_bar; }
    kvs::ColorMapBar* colorMapBar() const { return m_color_map_bar; }

public slots:
    void updateColorMapBar( QStandardItemModel* model );
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

private:
    QLabel* m_color_function_label;
    QComboBox* m_color_function_combo_box;
    kvs::qt::jaea::Screen* m_screen = nullptr;
    kvs::ColorMapBar* m_color_map_bar = nullptr;
    QStandardItemModel *m_model = nullptr;

    void initialize();

private slots:
    void updateUIFromCurrentItem(); // FIXME:メソッド名を変更した方がいいと思います。
};

#endif // COLORMAPSELECTORTOOLBAR_H

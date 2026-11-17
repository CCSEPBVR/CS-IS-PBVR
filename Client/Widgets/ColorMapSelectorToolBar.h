#ifndef COLORMAPSELECTORTOOLBAR_H
#define COLORMAPSELECTORTOOLBAR_H

#include <QToolBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

#include <kvs/ColorMapBar>
#include "Screen.h"

#include "../../Shared/TransferFunction.h"

class ColorMapSelectorToolBar : public QToolBar
{
    Q_OBJECT

public:
    enum class Mode
    {
        TransferFunction,
        EnsembleTransferFunction
    };

    explicit ColorMapSelectorToolBar( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~ColorMapSelectorToolBar();

    // ColorMapBar
    void setColorMapBar( kvs::ColorMapBar* color_map_bar ) { m_color_map_bar = color_map_bar; }
    kvs::ColorMapBar* colorMapBar() const { return m_color_map_bar; }

signals:

public slots:
    void setMode( Mode mode );
    void onTransferFunctionUpdate( TransferFunction* lastSentTransferFunction );
    void onEnsembleTransferFunctionUpdate( TransferFunction* ensembleTransferFunction, int selectedIndex );

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    QLabel* m_color_function_label;
    QComboBox* m_color_function_combo_box;

    kvs::qt::jaea::Screen* m_screen   = nullptr;
    kvs::ColorMapBar* m_color_map_bar = nullptr;

    Mode m_mode = Mode::TransferFunction;
    TransferFunction m_transfer_function_storage;
    TransferFunction m_ensemble_transfer_function_storage;
    TransferFunction* m_transfer_function = &m_transfer_function_storage;
    TransferFunction* m_ensemble_transfer_function = &m_ensemble_transfer_function_storage;

    void rebuildComboBox( int preferredIndex );
    TransferFunction* currentTransferFunction();
    void updateColorMapByIndex( int index );
    void updateCurrentIndex();

private slots:
    void onColorFunctionComboBoxIndexChanged( int index );
};

#endif // COLORMAPSELECTORTOOLBAR_H

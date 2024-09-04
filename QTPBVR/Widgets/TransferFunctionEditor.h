#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include "ToolBars.h"
#include "Widgets/ColorMapEditor.h"
#include "Widgets/OpacityMapEditor.h"

#include "ExtendedTransferFunctionMessage.h"
#include "ReceivedMessage.h"
#include "FunctionListEditor.h"

class MergePanel;
class Connect;

namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT
public:
    enum class Mode
    {
        CS,  //Client Server
        IS,  //In-situ
        None //No select
    };

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr, ColorMapBarSelector* colonr_map_bar_selector = nullptr, MergePanel* merge = nullptr, Connect* connect_panel = nullptr);
    ~TransferFunctionEditor();
    void applyVariableRange( const VariableRange& range );
    void updateRangeEdit();
//    void updateRangeView( const VariableRange& range );
//    void updateRangeView( const jpv::ParticleTransferServerMessage& reply );
    void updateRangeView();
    void importFile( const std::string& fileName );
    void exportFile( const std::string& fileName, const bool addition);
    ExtendedTransferFunctionMessage* getExtendedTransferFunctionMessage() { return &m_extended_transfer_function_message; }
    void importFromServerIS();
    void setMode( Mode mode ) { m_mode = mode; }
    Mode getMode() { return m_mode; }

private:
    Ui::TransferFunctionEditor *ui;
    ColorMapBarSelector* m_color_map_bar_selector;
    MergePanel* m_merge;
    Connect* m_connect;

    bool m_is_import_transfer_function_parameter;

    FunctionListEditor m_color_function_list_editor;
    FunctionListEditor m_opacity_function_list_editor;

    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    ExtendedTransferFunctionMessage m_extended_transfer_function_message;
    ExtendedTransferFunctionMessage m_extended_transfer_function_message_initial;

    QVector<bool> m_is_color_range_sync;
    QVector<bool> m_is_opacity_range_sync;

    Mode m_mode;

    void populateColorFunctionLists(int n);
    void populateOpacityFunctionLists(int n);

private slots:
    void onNumberOfTransferFunctionValueChanged( int value );

    void onColorFunctionChanged( int index );
    void onOpacityFunctionChanged( int index );

    void onColorFunctionSynthEdited( const QString &arg1 );
    void onOpacityFunctionSynthEdited( const QString &arg1 );

    void onTransferFunctionVarColorEdited( const QString &arg1 );
    void onTransferFunctionVarOpacityEdited( const QString &arg1 );

    void onColorFunctionListEditorButtonClicked();
    void onOpacityFunctionListEditorButtonClicked();

    void onColorMapEditorButtonClicked();
    void onOpacityMapEditorButtonClicked();

    void onTransferFunctionRangeColorChanged();
    void onTransferFunctionRangeOpacityChanged();

    void onImportButtonClicked();
    void onExportButtonClicked();

    void onColorRangeSyncToolButtonClicked();
    void onOpacityRangeSyncToolButtonClicked();
public slots:
    void onApplyButtonClicked();
};

#endif // TRANSFERFUNCTIONEDITOR_H

#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include "Widgets/TransferFunctionParameter.h"
#include "Common/VariableRange.h"
// #include "Widgets/FunctionListEditor.h"
#include "Widgets/VariableEditor.h"
#include "Widgets/ColorMapEditor.h"
#include "Widgets/OpacityMapEditor.h"

class MergePanel;
class Connect;

namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr, MergePanel* merge = nullptr, Connect* connect_panel = nullptr);
    ~TransferFunctionEditor();

    void exportTransferFunctionFile( const std::string& transferFunctionFile, const bool append );
    bool importTransferFunctionFromFile( const std::string& transferFunctionFile );
    void importTransferFunctionFromServer( bool isISMode );
    void apply();
    void updateRangeView();

private:
    Ui::TransferFunctionEditor *ui;
    TransferFunctionParameter m_parameter;
    MergePanel* m_merge;
    Connect* m_connect;

    // FunctionListEditor m_color_function_list_editor;
    // FunctionListEditor m_opacity_function_list_editor;
    VariableEditor m_variable_editor;
    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    void updateFunctionLists();

private slots:
    void onNumberOfTransferFunctionValueChanged( int numberOfTransferFunction );

    void onColorSynthesizerEdited( const QString &arg1 );
    void onColorFunctionChanged( int index );
    void onColorFunctionVariableEdited();
    void onColorFunctionListEditorPushButtonClicked();
    void onColorSelectedRangeClicked();
    void onColorUserDefinedChanged();
    void onEditColorMapPushButtonClicked();

    void onOpacitySynthesizerEdited( const QString &arg1 );
    void onOpacityFunctionChanged( int index );
    void onOpacityFunctionVariableEdited();
    void onOpacityFunctionListEditorPushButtonClicked();
    void onOpacitySelectedRangeClicked();
    void onOpacityUserDefinedChanged();
    void onEditOpacityMapPushButtonClicked();

    void onExportButtonClicked();
    void onImportButtonClicked();
    void onApplyButtonClicked();
};

#endif // TRANSFERFUNCTIONEDITOR_H

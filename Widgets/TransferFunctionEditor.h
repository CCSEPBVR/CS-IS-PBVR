#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include "Widgets/ColorFunctionSelector.h"
#include "Widgets/ColorMapEditor.h"
#include "Widgets/OpacityMapEditor.h"

#include "ExtendedTransferFunctionMessage.h"
#include "ReceivedMessage.h"
#include "FunctionListEditor.h"

class MergePanel;

namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr);
    ~TransferFunctionEditor();
    void setColorFunctionSelector( ColorFunctionSelector* color_function_selector ){ m_color_function_selector = color_function_selector; }
    void setMerge( MergePanel* merge ){  m_merge = merge; }
    void setClientMessage( jpv::ParticleTransferClientMessage* client_message ){ m_client_message = client_message; }
    void setServerMessage( jpv::ParticleTransferServerMessage* server_message ){ m_server_message = server_message; }
    void setReceivedMessage( kvs::visclient::ReceivedMessage* received_message ){ m_received_message = received_message; }
    void applyVariableRange( const VariableRange& range );
    void updateRangeEdit();
//    void updateRangeView( const VariableRange& range );
//    void updateRangeView( const jpv::ParticleTransferServerMessage& reply );
    void updateRangeView();
    void importFile( const std::string& fileName );
    void exportFile( const std::string& fileName, const bool addition);
    ExtendedTransferFunctionMessage* getExtendedTransferFunctionMessage() { return &m_extended_transfer_function_message; }

private:
    Ui::TransferFunctionEditor *ui;
    ColorFunctionSelector* m_color_function_selector;
    MergePanel* m_merge;

    jpv::ParticleTransferClientMessage* m_client_message;
    jpv::ParticleTransferServerMessage* m_server_message;
    kvs::visclient::ReceivedMessage* m_received_message;

    bool m_is_import_transfer_function_parameter;

    FunctionListEditor m_color_function_list_editor;
    FunctionListEditor m_opacity_function_list_editor;

    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    ExtendedTransferFunctionMessage m_extended_transfer_function_message;
    ExtendedTransferFunctionMessage m_extended_transfer_function_message_initial;

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

    void onApplyButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
};

#endif // TRANSFERFUNCTIONEDITOR_H

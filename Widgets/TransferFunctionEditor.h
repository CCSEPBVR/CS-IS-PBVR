#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include "Widgets/ColorMapEditor.h"
#include "Widgets/OpacityMapEditor.h"

#include "ExtendedTransferFunctionMessage.h"
#include "ReceivedMessage.h"
namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr);
    ~TransferFunctionEditor();
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

private:
    Ui::TransferFunctionEditor *ui;

    jpv::ParticleTransferClientMessage* m_client_message;
    jpv::ParticleTransferServerMessage* m_server_message;
    kvs::visclient::ReceivedMessage* m_received_message;

    bool m_is_import_transfer_function_parameter;

    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    ExtendedTransferFunctionMessage m_extended_transfer_function_message;
    ExtendedTransferFunctionMessage m_extended_transfer_function_message_initial;

    void populateColorFunctionLists(int n);
    void populateOpacityFunctionLists(int n);

private slots:
    jpv::ParticleTransferClientMessage::EquationToken convertToken( std::string );

    void onNumberOfTransferFunctionValueChanged( int value );

    void onColorFunctionChanged( int index );
    void onOpacityFunctionChanged( int index );

    void onColorFunctionSynthEdited( const QString &arg1 );
    void onOpacityFunctionSynthEdited( const QString &arg1 );

    void onTransferFunctionVarColorEdited( const QString &arg1 );
    void onTransferFunctionVarOpacityEdited( const QString &arg1 );

    void onColorMapEditorButtonClicked();
    void onOpacityMapEditorButtonClicked();

    void onTransferFunctionRangeColorChanged();
    void onTransferFunctionRangeOpacityChanged();

    void onApplyButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
};

#endif // TRANSFERFUNCTIONEDITOR_H

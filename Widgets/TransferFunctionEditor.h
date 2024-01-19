#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include "Widgets/ColorMapEditor.h"
#include "Widgets/OpacityMapEditor.h"

#include "ExtendedTransferFunctionMessage.h"
namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr);
    ~TransferFunctionEditor();

private:
    Ui::TransferFunctionEditor *ui;
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

    void onColorMapEditorButtonClicked();
    void onOpacityMapEditorButtonClicked();
};

#endif // TRANSFERFUNCTIONEDITOR_H

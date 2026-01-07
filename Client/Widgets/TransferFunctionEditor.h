#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include <QButtonGroup>
#include <QFileDialog>

#include "WebSocketPair.h"
#include "../../Shared/JsonKeys.h"
#include "../../Shared/TransferFunction.h"

#include "ClientUtils.h"
#include "VariableEditor.h"
#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"

// FIXME:ヒストグラム更新用処理を追加すること。

namespace Ui
{
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~TransferFunctionEditor();

public slots:
    void onOperatorStateUpdate( bool operatorState ); // true:権限あり
    void onReset();
    void onReceiveInitializeTransferFunctionParameter( const QJsonObject& dataArray );
    void onReceiveTransferFunctionParameter( const QString& colorSynth, const QString& opacitySynth, const QJsonArray& dataArray );
    // FIXME:KPI
    void onLoadParameter( const QString& filePath );
    void onSaveParameter( const QString& filePath );

signals:
    void transferFunctionUpdate();

private:
    Ui::TransferFunctionEditor *ui;
    WebSocketPair* m_web_sockets = nullptr;

    bool m_is_operator;

    TransferFunction* m_transfer_function = nullptr;

    VariableEditor m_variable_editor;
    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    void updateUI();
    void clear();
    void disable();
    void enable();
    void applyTransferFunction();

private slots:
    void onNumberOfTransferFunctionValueChanged( const int numberOfTransferFunction );

    void onColorSynthesizerChanged( const QString &colorSynthesizer );
    void onColorComboBoxChanged();
    void onColorFunctionVariableChanged( const QString &colorFunctionVariable );
    void onColorFunctionVariableEditorClicked();
    void onColorRangeModeRadioButtonClicked();
    void onColorUserDefinedMinChanged( const double &colorUserDefinedMin );
    void onColorUserDefinedMaxChanged( const double &colorUserDefinedMax );
    void onColorMapEditorClicked();

    void onOpacitySynthesizerChanged( const QString &opacitySynthesizer );
    void onOpacityComboBoxChanged();
    void onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable );
    void onOpacityRangeModeRadioButtonClicked();
    void onOpacityFunctionVariableEditorClicked();
    void onOpacityUserDefinedMinChanged( const double &opacityUserDefinedMin );
    void onOpacityUserDefinedMaxChanged( const double &opacityUserDefinedMax );
    void onOpacityMapEditorClicked();

    void onExport();
    void onImport();
    void onApply();
};

#endif // TRANSFERFUNCTIONEDITOR_H

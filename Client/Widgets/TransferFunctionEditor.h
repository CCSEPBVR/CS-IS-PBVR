#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include <QWebSocket>
#include <QFileDialog>

#include "TransferFunctionItem.h"

#include "VariableEditor.h"
#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"

namespace Ui
{
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor( QWebSocket* textSocket, QWidget *parent = nullptr );
    ~TransferFunctionEditor();

private:
    // メンバ変数群
    Ui::TransferFunctionEditor *ui;

    // 通信関連
    QWebSocket* m_web_text_socket = nullptr; // パラメータ送信用
    QStandardItemModel *m_model = nullptr;

    VariableEditor m_variable_editor;

    QString m_color_synthesizer;
    ColorMapEditor m_color_map_editor;

    QString m_opacity_synthesizer;
    OpacityMapEditor m_opacity_map_editor;

    // ウィジェット群
    // VriableEditor // FIXME

    // メソッド群
    void initialize();
    void exportTransferFunction();
    void importTransferFunction();
    void applyTransferFunction();

private slots:
    void onNumberOfTransferFunctionValueChanged( const int numberOfTransferFunction );

    void onColorSynthesizerChanged( const QString &colorSynthesizer );
    void onColorFunctionChanged();
    void onColorFunctionVariableChanged( const QString &colorFunctionVariable );
    void onEditColorVariable();
    void onColorUserDefinedMinMaxClicked();
    void onColorUserDefinedMinChanged( const float &colorUserDefinedMin );
    void onColorUserDefinedMaxChanged( const float &colorUserDefinedMax );
    void onColorServerSideMinMaxClicked();
    void onEditColorMap();

    void onOpacitySynthesizerChanged( const QString &opacitySynthesizer );
    void onOpacityFunctionChanged();
    void onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable );
    void onEditOpacityVariable();
    void onOpacityUserDefinedMinMaxClicked();
    void onOpacityUserDefinedMinChanged( const float &opacityUserDefinedMin );
    void onOpacityUserDefinedMaxChanged( const float &opacityUserDefinedMax );
    void onOpacityServerSideMinMaxClicked();
    void onEditOpacityMap();

    void onExport();
    void onImport();
    void onApply();

public slots:    
    void updateOperatorState( bool operatorState ); // true:権限あり
    void reset();
    void saveParameter();
};
#endif // TRANSFERFUNCTIONEDITOR_H

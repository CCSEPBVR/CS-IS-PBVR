#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include <QWebSocket>
#include <QFileDialog>

#include "WebSocketPair.h"
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
    explicit TransferFunctionEditor( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~TransferFunctionEditor();

private:
    // メンバ変数群
    Ui::TransferFunctionEditor *ui;

    // 通信関連
    WebSocketPair* m_web_sockets = nullptr;
    QStandardItemModel *m_model = nullptr;

    QString m_color_synthesizer;
    QString m_opacity_synthesizer;

    // ウィジェット群
    VariableEditor m_variable_editor;
    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

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
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );
};
#endif // TRANSFERFUNCTIONEDITOR_H

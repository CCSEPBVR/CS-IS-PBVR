#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QVector>
#include <vector>

#include "WebSocketPair.h"

#include "ClientUtils.h"
#include "VariableEditor.h"
#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"

#include "../../Shared/JsonKeys.h"
#include "../../Shared/TransferFunction.h"

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
    void reset();
    void emitLegendTransferFunctionUpdate();

signals:
    void transferFunctionUpdate();
    void updateLastSentTransferFunction( TransferFunction* lastSentTransferFunction );

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し

    // NOTE:テキストソケット用
    void onReceiveTransferFunctionParameter( const QJsonObject& payload );
    void onReceiveRequestDataAtTransferFunctionParameter( const QJsonObject& payload );

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    enum class UpdateTarget { Color, Opacity, Both };

    Ui::TransferFunctionEditor *ui;

    WebSocketPair* m_web_sockets          = nullptr;

    bool m_is_operator                    = true;

    TransferFunction* m_transfer_function = nullptr;

    VariableEditor   m_variable_editor;
    ColorMapEditor   m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    TransferFunction m_last_sent_tf;
    bool m_has_last_sent = false;
    TransferFunction m_legend_transfer_function;
    bool m_has_legend_transfer_function = false;

    void updateUIFromUserInput( UpdateTarget target );
    void updateUIFromServer( UpdateTarget target );

    template <class F>
    void updateSelectedColorTransferFunction( QComboBox* combo, F&& func )
    {
        if( !m_transfer_function ) return;

        const int idx = combo->currentIndex();
        const int tfCount = static_cast<int>( m_transfer_function->count() );
        if( idx < 0 || idx >= tfCount ) return;

        auto& color = m_transfer_function->at( static_cast<size_t>( idx ) ).color;
        std::forward<F>(func)( color );
    }

    template <class F>
    void updateSelectedOpacityTransferFunction( QComboBox* combo, F&& func )
    {
        if( !m_transfer_function ) return;

        const int idx = combo->currentIndex();
        const int tfCount = static_cast<int>( m_transfer_function->count() );
        if( idx < 0 || idx >= tfCount ) return;

        auto& opacity = m_transfer_function->at( static_cast<size_t>( idx ) ).opacity;
        std::forward<F>(func)( opacity );
    }

private slots:
    void onNumberOfTransferFunctionValueChanged( const int numberOfTransferFunction );
    // Color
    void onColorSynthesizerChanged( const QString &colorSynthesizer );
    void onColorComboBoxChanged();
    void onColorFunctionVariableChanged( const QString &colorFunctionVariable );
    void onColorFunctionVariableEditorClicked();
    void onColorRangeModeRadioButtonClicked();
    void onColorUserDefinedMinChanged( const double colorUserDefinedMin );
    void onColorUserDefinedMaxChanged( const double colorUserDefinedMax );
    void onColorMapEditorClicked();

    // Opacity
    void onOpacitySynthesizerChanged( const QString &opacitySynthesizer );
    void onOpacityComboBoxChanged();
    void onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable );
    void onOpacityRangeModeRadioButtonClicked();
    void onOpacityFunctionVariableEditorClicked();
    void onOpacityUserDefinedMinChanged( const double opacityUserDefinedMin );
    void onOpacityUserDefinedMaxChanged( const double opacityUserDefinedMax );
    void onOpacityMapEditorClicked();

    void onExport();
    void onImport();
    void onApply();
};

#endif // TRANSFERFUNCTIONEDITOR_H

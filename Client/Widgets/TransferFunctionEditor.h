#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>
#include <QStandardItem>
#include <QFileDialog>

#include "TransferFunctionItem.h"
#include "VariableEditor.h"
#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"

#include "ParticleTransferProtocol.h"
#include "ParticleTransferClient.h"
#include "ReceivedMessage.h"

namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor( QWidget *parent = nullptr );
    ~TransferFunctionEditor();

private:
    Ui::TransferFunctionEditor *ui;
    QStandardItemModel *m_model = nullptr;
    QString m_color_synthesizer;
    QString m_opacity_synthesizer;

private:
    void initialize();
    void updateUIFromCurrentItem();
    void disableUI();
    void enableUI();

signals:
    void failedTransferFunctionImport();
    void successTransferFunctionImport( const QString transferFunctionFile );
    void updateColorMapBar( QStandardItemModel* model );
    void updateTransferFunctionClientMessage( QString colorSynthesizer, QString opacitySynthesizer, QStandardItemModel* model );
    void requestReplaceServerPointObject();

public slots:
    void importTransferFunctionFromFile( const QString transferFunctionFile );
    void importTransferFunctionFromServer( const jpv::ParticleTransferServerMessage& serverMessage, kvs::visclient::ReceivedMessage receivedMessage, const bool isImportFlag );
    void applyTransferFunction();
    void updateTransferFunctionRangeAndView( const jpv::ParticleTransferServerMessage* serverMessage, kvs::visclient::ReceivedMessage receivedMessage );

private slots:
    void onNumberOfTransferFunctionValueChanged( int numberOfTransferFunction );

    void onColorSynthesizerChanged( const QString &colorSynthesizer );    
    void onColorFunctionVariableChanged( const QString &colorFunctionVariable );
    void onColorUserDefinedMinMaxClicked();
    void onColorServerSideMinMaxClicked();
    void onColorUserDefinedMinChanged( const float &colorUserDefinedMin );
    void onColorUserDefinedMaxChanged( const float &colorUserDefinedMax );
    void onEditColorVariable();
    void onEditColorMap();

    void onOpacitySynthesizerChanged( const QString &colorSynthesizer );
    void onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable );
    void onOpacityUserDefinedMinMaxClicked();
    void onOpacityServerSideMinMaxClicked();
    void onOpacityUserDefinedMinChanged( const float &opacityUserDefinedMin );
    void onOpacityUserDefinedMaxChanged( const float &opacityUserDefinedMax );
    void onEditOpacityVariable();
    void onEditOpacityMap();

    void onExport();
    void onImport();
    void onApply();
};

// #include <QDialog>
// #include <QFileDialog>

// #include <kvs/ValueArray>
// #include <kvs/ColorMap>
// #include <kvs/OpacityMap>

// #include "ParticleTransferProtocol.h"
// #include "ParticleTransferClient.h"
// #include "ReceivedMessage.h"
// #include "TransferFunctionItem.h"
// #include "VariableEditor.h"
// #include "ColorMapEditor.h"
// #include "OpacityMapEditor.h"

// #ifdef WIN32
// #undef min
// #undef max
// #endif

// namespace Ui {
// class TransferFunctionEditor;
// }

// class TransferFunctionEditor : public QDialog
// {
//     Q_OBJECT

// public:
//     explicit TransferFunctionEditor( QWidget *parent = nullptr );
//     ~TransferFunctionEditor();

// private:
//     Ui::TransferFunctionEditor *ui;
//     QStandardItemModel *m_model = nullptr;
//     QString m_color_synthesizer;
//     QString m_opacity_synthesizer;
//     void initialize();
//     void updateColorOpacityComboBox();

// private:
//     void debug();

// signals:
//     void failedTransferFunctionImport();
//     void successTransferFunctionImport( const QString transferFunctionFile );
//     void updateTransferFunctionClientMessage( QString colorSynthesizer, QString opacitySynthesizer, QStandardItemModel* model );
//     void needSameTimeStepReplaceServerPointObject();

// public slots:
//     void importTransferFunctionFromFile( const QString transferFunctionFile );
//     void importTransferFunctionFromServer( const jpv::ParticleTransferServerMessage& serverMessage, kvs::visclient::ReceivedMessage receivedMessage );
//     void applyTransferFunction();
//     void updateTransferFunctionRangeAndView( const jpv::ParticleTransferServerMessage* serverMessage, kvs::visclient::ReceivedMessage receivedMessage );

// private slots:
//     void onNumberOfTransferFunctionValueChanged( int numberOfTransferFunction );
//     void onColorFunctionChanged();
//     void onColorSynthesizerChanged( const QString &colorSynthesizer );
//     void onColorFunctionVariableChanged( const QString &colorFunctionVariable );
//     void onEditColorVariable();
//     void onColorUserDefinedMinMaxClicked();
//     void onColorServerSideMinMaxClicked();
//     void onColorUserDefinedMinChanged( const float &colorUserDefinedMin );
//     void onColorUserDefinedMaxChanged( const float &colorUserDefinedMax );
//     void onEditColorMap();

//     void onOpacityFunctionChanged();
//     void onOpacitySynthesizerChanged( const QString &opacitySynthesizer );
//     void onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable );
//     void onEditOpacityVariable();
//     void onOpacityUserDefinedMinMaxClicked();
//     void onOpacityServerSideMinMaxClicked();
//     void onOpacityUserDefinedMinChanged( const float &opacityUserDefinedMin );
//     void onOpacityUserDefinedMaxChanged( const float &opacityUserDefinedMax );
//     void onEditOpacityMap();

//     void onExport();
//     void onImport();
//     void onApply();
// };

#endif // TRANSFERFUNCTIONEDITOR_H

#ifndef TRANSFERFUNCEDITOR_CS_H
#define TRANSFERFUNCEDITOR_CS_H

#include <QDialog>
#include "Panels/coloreditor.h"
#include "Panels/opacityeditor.h"
#include "Panels/functionlisteditor.h"

#include <QGlue/extCommand.h>

namespace QGlue{
class OpacityMapPalette;
class ColorMapPalette;
class Histogram;
}

namespace Ui {
class TransferFuncEditor;
}

class TransferFuncEditor : public QDialog
{
    //    Q_OBJECT

public:
    explicit TransferFuncEditor(QWidget *parent = nullptr);

    kvs::visclient::ExtendedTransferFunctionMessage& doc();
    const kvs::visclient::ExtendedTransferFunctionMessage& doc() const;

    virtual void setCommandInstance(ExtCommand*);

    void exportFile( const std::string& fname, const bool addition);
    void importFile( const std::string& fname );

    void updateRangeView();
    void updateRangeEdit();

    bool selectTransferFunctionEditorColorFunction(int i);
    bool selectTransferFunctionEditorOpacityFunction(int num);

    virtual void apply();

    ~TransferFuncEditor();

public:
    ColorEditor colorEditor;
    OpacityEditor opacityEditor;

    FunctionListEditor cFuncEditor;
    FunctionListEditor oFuncEditor;

private:

    kvs::visclient::ExtendedTransferFunctionMessage m_doc;
    kvs::visclient::ExtendedTransferFunctionMessage m_doc_initial;

    QGlue::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    QGlue::OpacityMapPalette* m_opacity_map_palette; ///< opacity map palette
    QGlue::Histogram*    m_color_histogram;     ///< color histogram
    QGlue::Histogram*    m_opacity_histogram;   ///< opacity histogram


    double requested_opacity_min=0;
    double requested_opacity_max=1;
    double requested_color_min=0;
    double requested_color_max=1;
    //ADD BY)T.Osaki 2020.02.05
    size_t m_resolution = 256;

    /**
       * 伝達関数の表示を行う。
       */
    void displayTransferFunction();
    void clearTransferFunction();
    void populateOpacityFunctionLists(int n);
    void populateColorFunctionLists(int n);



    void connectSignalsToSlots();
private://Event Handlers

    void onLockOpacityRange(bool changed);
    void onLockColorRange(bool changed);
    void onResolutionChanged(int val);
    void onColorButtonClicked();
    void onOpacityButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
    void onNumTransferChanged(int val);
    void onColorMapFunctionChanged(int index);
    void onOpacityMapFunctionChanged(int index);
    void onColorFunctionSynthEdited(const QString &arg1);
    void onOpacityFunctionSynthEdited(const QString &arg1);
    void onCfuncClicked();
    void onOfuncClicked();
    void onTransferFunctionVarColorEdited(const QString &arg1);
    void onTransferFunctionVarOpacityEdited(const QString &arg1);
    void onTFminColorChanged(double arg1);
    void onTFminOpacityChanged(double arg1);
    void onTFmaxColorChanged(double arg1);
    void onTFmaxOpacityChanged(double arg1);
    void onResetButtonClicked();
    void onApplyButtonClicked();
    void onResolutionChangeButtonClicked();

private:
    Ui::TransferFuncEditor *ui;

};

#endif // TRANSFERFUNCEDITOR_CS_H

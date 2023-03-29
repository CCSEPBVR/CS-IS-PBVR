#ifndef TRANSFERFUNCEDITOR_IS_H
#define TRANSFERFUNCEDITOR_IS_H

#include <QDialog>
#include "Panels/coloreditor.h"
#include "Panels/opacityeditor.h"
#include "Panels/functionlisteditor.h"
#include "Client/ParamExTransFunc_IS.h".h"
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
    explicit TransferFuncEditor(QWidget *parent = 0);

    kvs::visclient::ParamExTransFunc& doc();
    const kvs::visclient::ParamExTransFunc& doc() const;

    virtual void setCommandInstance(ExtCommand*);

    void exportFile( const std::string& fname, const bool addition);
    void importFile( const std::string& fname );
    void importExtendTransferFunction(kvs::visclient::ParamExTransFunc doc);
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

    kvs::visclient::ParamExTransFunc m_doc;
    kvs::visclient::ParamExTransFunc m_doc_initial;

    QGlue::ColorMapPalette*   m_color_map_palette;   ///< color map palette
    QGlue::OpacityMapPalette* m_opacity_map_palette; ///< opacity map palette
    QGlue::Histogram*    m_color_histogram;     ///< color histogram
    QGlue::Histogram*    m_opacity_histogram;   ///< opacity histogram

    /**
       * 伝達関数の表示を行う。
       */
    void displayTransferFunction();
    void clearTransferFunction();
    void populateTransferFuncList(int n);
    void updateParamExTransFuncDoc();


    void connectSignalsToSlots();
private://Event Handlers
    void onResolutionChanged(int val);
    void onColorButtonClicked();
    void onOpacityButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
    void onNumTransferChanged(int val);
    void onTransferFunctionSelected(int index);
//    void onOpacityMapFunctionChanged(int index);
    void onFunctionSynthEdited(const QString &arg1);
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

private:
    Ui::TransferFuncEditor *ui;

};

#endif // TRANSFERFUNCEDITOR_IS_H

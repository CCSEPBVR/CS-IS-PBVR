#ifndef FUNCLIST_EDITOR_H
#define FUNCLIST_EDITOR_H

#include <QDialog>
#include <ExtendedTransferFunctionParameter.h>
#include "Client/ParamExTransFunc.h"
namespace Ui {
class FunctionListEditor;
}

class TransferFuncEditor;
class FunctionListEditor : public QDialog
{
public:
    enum FUNCTION_DIALOG_TYPE
    {
        UNKNOWN_DIALOG          = 1,
        COLOR_FUNCTION_DIALOG   = 1,
        OPACITY_FUNCTION_DIALOG = 2,
    };

public:
    explicit FunctionListEditor(QWidget *parent = 0);

    void    initalize(FUNCTION_DIALOG_TYPE dialogtype, kvs::visclient::ExtendedTransferFunctionMessage& m_doc, int i);
    void    setTransferFunctionParameterList(std::vector<NamedTransferFunctionParameter> *original_transfer_function);
    int     getSelectedFunctionIndex();

    ~FunctionListEditor();

private:
    int  save();
    void setMessage(QString message);

    bool validateFunction(const char *left, const char *right);
    void onFunctionListRowChanged(int currentRow);
    void onSetButtonClicked();
    void onSelectButtonClicked();
    void onSaveButtonClicked();
    void onCancelButtonClicked();

private:

    FUNCTION_DIALOG_TYPE m_dialogtype= COLOR_FUNCTION_DIALOG;
    TransferFuncEditor* m_parent;

    std::vector<NamedTransferFunctionParameter> *m_original_transfer_function;
    std::vector<NamedTransferFunctionParameter> m_edit_transfer_function;

private:

    Ui::FunctionListEditor *ui;
};

#endif // FUNCLIST_EDITOR_H

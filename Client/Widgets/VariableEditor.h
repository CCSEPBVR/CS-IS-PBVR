#ifndef VARIABLEEDITOR_H
#define VARIABLEEDITOR_H

#include <QDialog>
#include "TransferFunctionParameter.h"

namespace Ui {
class VariableEditor;
}

class VariableEditor : public QDialog
{
    Q_OBJECT

public:
    enum TYPE
    {
        COLOR   = 0,
        OPACITY = 1,
    };

public:
    explicit VariableEditor(QWidget *parent = nullptr);
    ~VariableEditor();
    void initalize( TYPE function_type, TransferFunctionParameter& transfer_function_parameter );
    TransferFunctionParameter& getEditTransferFunctionParameter() { return m_edit_transfer_function_parameter; }
private:
    Ui::VariableEditor *ui;
    TYPE m_type;
    TransferFunctionParameter m_edit_transfer_function_parameter;

    void setItems();

private slots:
    void onCellChanged( int row, int column );
    void onCancelButtonClicked();
    void onOKButtonClicked();
};

#endif // VARIABLEEDITOR_H

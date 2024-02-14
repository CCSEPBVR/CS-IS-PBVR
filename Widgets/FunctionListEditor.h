#ifndef FUNCTIONLISTEDITOR_H
#define FUNCTIONLISTEDITOR_H

#include <QDialog>
#include "ExtendedTransferFunctionMessage.h"

namespace Ui {
class FunctionListEditor;
}

class FunctionListEditor : public QDialog
{
    Q_OBJECT

public:
    enum FUNCTION_TYPE
    {
        COLOR_FUNCTION   = 1,
        OPACITY_FUNCTION = 2,
    };

public:
    explicit FunctionListEditor(QWidget *parent = nullptr);
    ~FunctionListEditor();
    void initalize(FUNCTION_TYPE function_type, ExtendedTransferFunctionMessage& extended_transfer_function, int i);

private:
    Ui::FunctionListEditor *ui;
    FUNCTION_TYPE m_function_type;
};

#endif // FUNCTIONLISTEDITOR_H

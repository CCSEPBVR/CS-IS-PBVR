#ifndef VARIABLEEDITORWIP_H
#define VARIABLEEDITORWIP_H

#include <QDialog>
#include <QStandardItemModel>

#include "../../Shared/TransferFunction.h"

namespace Ui {
class VariableEditorWIP;
}

class VariableEditorWIP : public QDialog
{
    Q_OBJECT

public:
    enum TYPE
    {
        COLOR   = 0,
        OPACITY = 1,
    };

    explicit VariableEditorWIP( QWidget *parent = nullptr );
    ~VariableEditorWIP();

    QStandardItemModel* model() const { return m_model; }
    void setup( TYPE type, TransferFunction* transferFunction );
    void initialize();

private:
    Ui::VariableEditorWIP *ui;

    QStandardItemModel* m_model = nullptr;
    TYPE m_type;
    TransferFunction* m_transfer_function;
};

#endif // VARIABLEEDITORWIP_H

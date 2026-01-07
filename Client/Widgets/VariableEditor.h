#ifndef VARIABLEEDITOR_H
#define VARIABLEEDITOR_H

#include <QDialog>
#include <QStandardItemModel>

#include "../../Shared/TransferFunction.h"

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

    explicit VariableEditor( QWidget *parent = nullptr );
    ~VariableEditor();

    QStandardItemModel* model() const { return m_model; }
    void setup( TYPE type, TransferFunction* transferFunction );
    void initialize();

private:
    Ui::VariableEditor *ui;

    QStandardItemModel* m_model = nullptr;
    TYPE m_type;
    TransferFunction* m_transfer_function;
};

#endif // VARIABLEEDITOR_H

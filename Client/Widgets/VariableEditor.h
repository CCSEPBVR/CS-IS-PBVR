#ifndef VARIABLEEDITOR_H
#define VARIABLEEDITOR_H

#include <QDialog>

#include "TransferFunctionItem.h"

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
    explicit VariableEditor( TYPE type, const QStandardItemModel& model, QWidget* parent = nullptr );
    ~VariableEditor();

    const QStandardItemModel& editedModel() const { return m_editable_mode; }

private:
    Ui::VariableEditor *ui;
    TYPE m_type;
    const QStandardItemModel& m_original_model;
    QStandardItemModel m_editable_mode;

    void initialize();
};

#endif // VARIABLEEDITOR_H

#ifndef VARIABLEEDITOR_H
#define VARIABLEEDITOR_H

#include <QDialog>
#include <QStandardItemModel>

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
    explicit VariableEditor(QWidget* parent = nullptr);
    ~VariableEditor();

    void setType(TYPE type) { m_type = type; }
    void setModel(const QStandardItemModel& model) { m_original_model = &model; }
    void initialize(); // TYPE とモデルをセット後に呼ぶ

    const QStandardItemModel& editedModel() const { return m_editable_mode; }

private:
    Ui::VariableEditor* ui;
    TYPE m_type;
    const QStandardItemModel* m_original_model = nullptr; // 後付け設定
    QStandardItemModel m_editable_mode;
};

#endif // VARIABLEEDITOR_H

#include "VariableEditor.h"
#include "ui_VariableEditor.h"

VariableEditor::VariableEditor(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::VariableEditor)
    , m_type(COLOR)
    , m_original_model(nullptr)
{
    ui->setupUi(this);

    ui->treeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    connect(ui->cancelPushButton, &QPushButton::clicked, this, &VariableEditor::close);
    connect(ui->OKPushButton, &QPushButton::clicked, this, &VariableEditor::accept);
}

VariableEditor::~VariableEditor()
{
    delete ui;
}

void VariableEditor::initialize()
{
    if (!m_original_model) return;

    // 既存のデータをクリア
    m_editable_mode.clear();

    m_editable_mode.setColumnCount(2);
    m_editable_mode.setHorizontalHeaderLabels(QStringList() << "Function" << "f（algebraic formula）");

    for (int row = 0; row < m_original_model->rowCount(); ++row)
    {
        const QStandardItem* item = m_original_model->item(row);
        QString funcStr, varStr;

        switch (m_type)
        {
        case TYPE::COLOR:
            funcStr = item->data(TransferFunctionItem::ColorFunction).toString();
            varStr = item->data(TransferFunctionItem::ColorVariable).toString();
            break;
        case TYPE::OPACITY:
            funcStr = item->data(TransferFunctionItem::OpacityFunction).toString();
            varStr = item->data(TransferFunctionItem::OpacityVariable).toString();
            break;
        }

        QStandardItem* funcItem = new QStandardItem(funcStr);
        funcItem->setFlags(funcItem->flags() & ~Qt::ItemIsEditable);

        QStandardItem* varItem = new QStandardItem(varStr);

        QList<QStandardItem*> newRow;
        newRow << funcItem << varItem;
        m_editable_mode.appendRow(newRow);
    }

    ui->treeView->setModel(&m_editable_mode);
}


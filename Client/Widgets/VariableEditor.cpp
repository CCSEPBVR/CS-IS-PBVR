#include "VariableEditor.h"
#include "ui_VariableEditor.h"
#include <QTableWidgetItem>

VariableEditor::VariableEditor(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VariableEditor)
{
    ui->setupUi(this);
    connect( ui->tableWidget, &QTableWidget::cellChanged, this, &VariableEditor::onCellChanged );
    connect( ui->CancelPushButton, &QPushButton::clicked, this, &VariableEditor::onCancelButtonClicked );
    connect( ui->OKPushButton, &QPushButton::clicked, this, &VariableEditor::onOKButtonClicked );
}

VariableEditor::~VariableEditor()
{
    delete ui;
}

void VariableEditor::initalize( TYPE type, TransferFunctionParameter& transfer_function_parameter )
{
    m_edit_transfer_function_parameter = transfer_function_parameter;
    m_type = type;

    if( m_type == TYPE::COLOR )
    {
        setWindowTitle( "Color Variable Editor" );
        ui->label->setText("Color Variable List [ ex : C1-99 = q1+q2*X ]");
    }
    else if( m_type == TYPE::OPACITY )
    {
        setWindowTitle( "Opacity Variable Editor" );
        ui->label->setText("Opacity Variable List [ ex : C1-99 = q1+q2*X ]");
    }

    setItems();
}

void VariableEditor::setItems()
{
    ui->tableWidget->blockSignals( true );
    ui->tableWidget->clearSelection();
    ui->tableWidget->clear();

    ui->tableWidget->setRowCount( m_edit_transfer_function_parameter.m_number_of_transfer_functions );
    ui->tableWidget->setColumnCount( 2 );

    ui->tableWidget->verticalHeader()->setVisible( false );

    QStringList headerLabels;
    headerLabels << "Function" << "f(algebraic formula)";
    ui->tableWidget->setHorizontalHeaderLabels( headerLabels );

    for( int i = 0; i < m_edit_transfer_function_parameter.m_number_of_transfer_functions; i++ )
    {
        if( m_type == TYPE::COLOR )
        {
            ui->tableWidget->setItem( i, 0, new QTableWidgetItem( QString::fromStdString( m_edit_transfer_function_parameter.m_transfer_function[i].m_color_function_name ) ) );
            ui->tableWidget->item( i, 0 )->setFlags( ui->tableWidget->item( i, 0 )->flags() & ~Qt::ItemIsEditable );
            ui->tableWidget->setItem( i, 1, new QTableWidgetItem( QString::fromStdString( m_edit_transfer_function_parameter.m_transfer_function[i].m_color_variable ) ) );
        }
        else if( m_type == TYPE::OPACITY )
        {
            ui->tableWidget->setItem( i, 0, new QTableWidgetItem( QString::fromStdString( m_edit_transfer_function_parameter.m_transfer_function[i].m_opacity_function_name ) ) );
            ui->tableWidget->item( i, 0 )->setFlags( ui->tableWidget->item( i, 0 )->flags() & ~Qt::ItemIsEditable );
            ui->tableWidget->setItem( i, 1, new QTableWidgetItem( QString::fromStdString( m_edit_transfer_function_parameter.m_transfer_function[i].m_opacity_variable ) ) );
        }
    }
    ui->tableWidget->blockSignals( false );
}

void VariableEditor::onCellChanged( int row, int column )
{
    if( m_type == TYPE::COLOR )
    {
        m_edit_transfer_function_parameter.m_transfer_function[row].m_color_variable = ui->tableWidget->item( row, column )->text().toStdString();
    }
    else if( m_type == TYPE::OPACITY )
    {
        m_edit_transfer_function_parameter.m_transfer_function[row].m_opacity_variable = ui->tableWidget->item( row, column )->text().toStdString();
    }
}

void VariableEditor::onCancelButtonClicked()
{
    close();
}

void VariableEditor::onOKButtonClicked()
{
    this->accept();
}

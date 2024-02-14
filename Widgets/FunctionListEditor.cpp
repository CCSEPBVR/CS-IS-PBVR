#include "FunctionListEditor.h"
#include "ui_FunctionListEditor.h"

FunctionListEditor::FunctionListEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FunctionListEditor),
    m_function_type( COLOR_FUNCTION )
{
    ui->setupUi(this);
}

FunctionListEditor::~FunctionListEditor()
{
    delete ui;
}

void FunctionListEditor::initalize( FUNCTION_TYPE function_type, ExtendedTransferFunctionMessage& extended_transfer_function, int i )
{
    this->m_function_type=function_type;
    if (function_type == COLOR_FUNCTION) {
        setWindowTitle("Color Function Editor");
        ui->exampleLbl->setText("Color Function List [ ex : C1-99 = q1+q2*X ]");
//        setTransferFunctionParameterList(&extended_transfer_function.m_color_transfer_function);
    }
    else {
        setWindowTitle("Opacity Function Editor");
        ui->exampleLbl->setText("Opacity Function List [ ex : O1-99 = q1+q2*X ]");
//        setTransferFunctionParameterList(&m_doc.m_opacity_transfer_function);
    }
    ui->functionLWidget->setCurrentRow(i);
}

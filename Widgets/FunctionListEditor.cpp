#include "FunctionListEditor.h"
#include "ui_FunctionListEditor.h"

FunctionListEditor::FunctionListEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FunctionListEditor),
    m_function_type( COLOR_FUNCTION )
{
    ui->setupUi(this);
    connect( ui->functionLWidget, &QListWidget::currentRowChanged, this, &FunctionListEditor::onFunctionListRowChanged );
    connect( ui->selectedFunction, &QLineEdit::returnPressed, this, &FunctionListEditor::onSetButtonClicked);
//    connect( ui->selectedFunction, &QLineEdit::cursorPositionChanged,this,&FunctionListEditor::onCursorPositionChanged );
//    connect( ui->setPBtn, &QPushButton::clicked, this, &FunctionListEditor::onSetButtonClicked );
    connect( ui->cancelPBtn, &QPushButton::clicked, this, &FunctionListEditor::onCancelButtonClicked );
    connect( ui->okPBtn, &QPushButton::clicked, this, &FunctionListEditor::onOkButtonClicked );
}

FunctionListEditor::~FunctionListEditor()
{
    delete ui;
}

void FunctionListEditor::initalize( FUNCTION_TYPE function_type, ExtendedTransferFunctionMessage& extended_transfer_function, int i )
{
    this->m_function_type=function_type;
    if (function_type == COLOR_FUNCTION)
    {
        setWindowTitle("Color Function Editor");
        ui->exampleLbl->setText("Color Function List [ ex : C1-99 = q1+q2*X ]");
        setTransferFunctionParameterList(&extended_transfer_function.m_color_transfer_function);
    }
    else
    {
        setWindowTitle("Opacity Function Editor");
        ui->exampleLbl->setText("Opacity Function List [ ex : O1-99 = q1+q2*X ]");
        setTransferFunctionParameterList(&extended_transfer_function.m_opacity_transfer_function);
    }
    ui->functionLWidget->setCurrentRow(i);
}

void FunctionListEditor::setTransferFunctionParameterList( std::vector<NamedTransferFunctionParameter> *original_extended_transfer_function)
{
    m_original_extended_transfer_function = original_extended_transfer_function;
    if( original_extended_transfer_function == NULL ) return;
    ui->functionLWidget->blockSignals( true );
    ui->functionLWidget->clearSelection();
    ui->functionLWidget->clear();
    m_edit_extended_transfer_function.clear();

    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = original_extended_transfer_function->begin(); itr != original_extended_transfer_function->end(); itr++)
    {
        NamedTransferFunctionParameter trans = (*itr);
        NamedTransferFunctionParameter edit_trans;
        edit_trans.m_name = trans.m_name;
        const char *left = trans.m_name.c_str();
        const char *right = NULL;
        if (this->m_function_type == COLOR_FUNCTION) {
            if (trans.m_name.substr(0, 1) != "C") continue;
            right = trans.m_color_variable.c_str();
            edit_trans.m_color_variable = trans.m_color_variable;
        }
        else {
            if (trans.m_name.substr(0, 1) != "O") continue;
            right = trans.m_opacity_variable.c_str();
            edit_trans.m_opacity_variable = trans.m_opacity_variable;
        }
        char function[256] = {0x00};
        sprintf(function, "%s = f(%s)", left , right);
        ui->functionLWidget->addItem(QString(function));
        ui->functionLWidget->blockSignals( false );
        //        m_funclist->add_item(num_lines, function);

        this->m_edit_extended_transfer_function.push_back(edit_trans);
    }
}

void FunctionListEditor::save()
{
    NamedTransferFunctionParameter edit_ntfp;
    for (size_t i=0; i< m_edit_extended_transfer_function.size(); i++)
    {
        edit_ntfp=m_edit_extended_transfer_function[i];
        if (edit_ntfp.m_name.substr(0, 1) == "O" && this->m_function_type == OPACITY_FUNCTION)
        {
            m_original_extended_transfer_function->at(i).m_opacity_variable = edit_ntfp.m_opacity_variable;
        }
        if (edit_ntfp.m_name.substr(0, 1) == "C" && this->m_function_type == COLOR_FUNCTION)
        {
            m_original_extended_transfer_function->at(i).m_color_variable = edit_ntfp.m_color_variable;
        }
    }
}

void FunctionListEditor::onFunctionListRowChanged( int current_row )
{
    NamedTransferFunctionParameter edit_trans = this->m_edit_extended_transfer_function[current_row];
    if( m_function_type == COLOR_FUNCTION )
    {
        ui->selectedName->setText( edit_trans.m_name.c_str() );
        ui->selectedFunction->setText( edit_trans.m_color_variable.c_str() );
    }
    else
    {
        ui->selectedName->setText( edit_trans.m_name.c_str() );
        ui->selectedFunction->setText( edit_trans.m_opacity_variable.c_str() );
    }
}

void FunctionListEditor::onSetButtonClicked()
{
    std::string selected_left=ui->selectedName->text().toStdString();
    std::string selected_right=ui->selectedFunction->text().toStdString();

    const char *left = selected_left.c_str();
    const char *right =selected_right.c_str();
    char function[256] = {0x00};
    sprintf(function, "%s = f(%s)", left , right);

    int current_row =ui->functionLWidget->currentRow();
    NamedTransferFunctionParameter *edit_trans = &(m_edit_extended_transfer_function[current_row]);

    if (m_function_type == COLOR_FUNCTION)
    {
        edit_trans->m_color_variable = std::string(right);
    }
    else
    {
        edit_trans->m_opacity_variable = std::string(right);
    }
    edit_trans->m_name = std::string(left);
    ui->functionLWidget->currentItem()->setText(QString(function));
}

void FunctionListEditor::onCancelButtonClicked()
{
    close();
}

void FunctionListEditor::onOkButtonClicked()
{
    save();
    close();
}

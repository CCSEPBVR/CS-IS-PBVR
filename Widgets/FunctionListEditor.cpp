#include "FunctionListEditor.h"
#include "ui_FunctionListEditor.h"

FunctionListEditor::FunctionListEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FunctionListEditor),
    m_function_type( COLOR_FUNCTION )
{
    ui->setupUi(this);
    connect( ui->functionTWidget, &QTableWidget::cellChanged, this, &FunctionListEditor::onCellChanged );
    connect( ui->cancelPBtn, &QPushButton::clicked, this, &FunctionListEditor::onCancelButtonClicked );
    connect( ui->okPBtn, &QPushButton::clicked, this, &FunctionListEditor::onOkButtonClicked );
}

FunctionListEditor::~FunctionListEditor()
{
    delete ui;
}

void FunctionListEditor::initalize( FUNCTION_TYPE function_type, ExtendedTransferFunctionMessage& extended_transfer_function )
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
}

void FunctionListEditor::setTransferFunctionParameterList( std::vector<NamedTransferFunctionParameter> *original_extended_transfer_function)
{
    m_original_extended_transfer_function = original_extended_transfer_function;
    if( original_extended_transfer_function == NULL ) return;

    ui->functionTWidget->blockSignals( true );
    ui->functionTWidget->clearSelection();
    ui->functionTWidget->clear();
    m_edit_extended_transfer_function.clear();

    //行と列数を設定
    ui->functionTWidget->setRowCount( original_extended_transfer_function->size() );
    ui->functionTWidget->setColumnCount( 2 );

    //行ヘッダーの非表示
    ui->functionTWidget->verticalHeader()->setVisible( false );

    //列ヘッダーの設定
    QStringList headerLabels;
    headerLabels << "Function" << "f(algebraic formula)";
    ui->functionTWidget->setHorizontalHeaderLabels( headerLabels );

    int counter = 0;
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = original_extended_transfer_function->begin(); itr != original_extended_transfer_function->end(); itr++)
    {
        NamedTransferFunctionParameter trans = (*itr);
        NamedTransferFunctionParameter edit_trans;
        edit_trans.m_name = trans.m_name;
        const char *left = trans.m_name.c_str();
        const char *right = NULL;

        if (this->m_function_type == COLOR_FUNCTION)
        {
            if (trans.m_name.substr(0, 1) != "C") continue;
            right = trans.m_color_variable.c_str();
            edit_trans.m_color_variable = trans.m_color_variable;
        }
        else
        {
            if (trans.m_name.substr(0, 1) != "O") continue;
            right = trans.m_opacity_variable.c_str();
            edit_trans.m_opacity_variable = trans.m_opacity_variable;
        }

        //Functionセル
        ui->functionTWidget->setItem( counter, 0, new QTableWidgetItem( left ) );
        //Functionのセルを編集不可にする。
        ui->functionTWidget->item( counter, 0 )->setFlags( ui->functionTWidget->item( counter, 0 )->flags() & ~Qt::ItemIsEditable );

        //algebraic formulaセル
        ui->functionTWidget->setItem( counter, 1, new QTableWidgetItem( right ) );

        this->m_edit_extended_transfer_function.push_back( edit_trans );
        counter++;
    }
    ui->functionTWidget->blockSignals( false );
}

void FunctionListEditor::save()
{
    NamedTransferFunctionParameter edit_ntfp;
    for (size_t i=0; i< m_edit_extended_transfer_function.size(); i++)
    {
        edit_ntfp=m_edit_extended_transfer_function[i];
        if (edit_ntfp.m_name.substr(0, 1) == "C" && this->m_function_type == COLOR_FUNCTION)
        {
            m_original_extended_transfer_function->at(i).m_color_variable = edit_ntfp.m_color_variable;
        }
        if (edit_ntfp.m_name.substr(0, 1) == "O" && this->m_function_type == OPACITY_FUNCTION)
        {
            m_original_extended_transfer_function->at(i).m_opacity_variable = edit_ntfp.m_opacity_variable;
        }
    }
}

void FunctionListEditor::onCellChanged( int row, int column )
{
    qInfo() << "CHANGED" << row << "," << column;
#ifdef Q_OS_WIN
    std::string selected_left = ui->functionTWidget->item( row, 0 )->text().toLocal8Bit().constData();
    std::string selected_right = ui->functionTWidget->item( row, 1 )->text().toLocal8Bit().constData();
#else
    std::string selected_left = ui->functionTWidget->item( row, 0 )->text().toStdString();
    std::string selected_right = ui->functionTWidget->item( row, 1 )->text().toStdString();
#endif

    const char *left = selected_left.c_str();
    const char *right = selected_right.c_str();
    char function[256] = {0x00};
    sprintf(function, "%s = f(%s)", left , right);

    NamedTransferFunctionParameter *edit_named_transfer_function = &( m_edit_extended_transfer_function[row] );

    if (m_function_type == COLOR_FUNCTION)
    {
        edit_named_transfer_function->m_color_variable = std::string(right);
    }
    else
    {
        edit_named_transfer_function->m_opacity_variable = std::string(right);
    }
    edit_named_transfer_function->m_name = std::string(left);
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

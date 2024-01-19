#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"
#include <kvs/ColorMapBar>
#include <kvs/OpacityMapBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "Widgets/FrequencyTable.h"

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferFunctionEditor)
{
    ui->setupUi(this);
    connect( ui->numberOfTransferFunctionSBox, &QSpinBox::valueChanged, this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );
    connect( ui->colorFunctionCBox, &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onColorFunctionChanged );
    connect( ui->opacityFunctionCBox, &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onOpacityFunctionChanged );

    connect( ui->color_function_synth, &QLineEdit::textEdited, this, &TransferFunctionEditor::onColorFunctionSynthEdited );
    connect( ui->opacity_function_synth, &QLineEdit::textEdited, this, &TransferFunctionEditor::onOpacityFunctionSynthEdited );


    connect( ui->transfer_function_var_color, &QLineEdit::textEdited, this, &TransferFunctionEditor::onTransferFunctionVarColorEdited );
    connect( ui->transfer_function_var_opacity, &QLineEdit::textEdited, this, &TransferFunctionEditor::onTransferFunctionVarOpacityEdited );

    connect( ui->editColorMapPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onColorMapEditorButtonClicked );
    connect( ui->editOpacityMapPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onOpacityMapEditorButtonClicked );

    connect( ui->transfer_function_min_color, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeColorChanged );
    connect( ui->transfer_function_max_color, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeColorChanged );

    connect( ui->transfer_function_min_opacity, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeOpacityChanged );
    connect( ui->transfer_function_max_opacity, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeOpacityChanged );

    populateColorFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );
    populateOpacityFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( int value )
{
    int num_transfer_function = value;
    m_extended_transfer_function_message.m_transfer_function_number = num_transfer_function;
    this->populateColorFunctionLists( value );
    this->populateOpacityFunctionLists( value );

    int n;
    int current_size;
    char name[8];

    current_size = m_extended_transfer_function_message.m_color_transfer_function.size();
    if (current_size <= num_transfer_function) {
        // 追加を行う
        for (n=current_size+1; n<=num_transfer_function; n++) {
            sprintf(name, "C%d", n);
            m_extended_transfer_function_message.addColorTransferFunction(name, "q1");
        }
    }
    else {
        // 削除を行う
        for (n=num_transfer_function+1; n<=current_size; n++) {
            sprintf(name, "C%d", n);
            m_extended_transfer_function_message.removeColorTransferFunction(name);
        }
    }

    // 不透明度関数を追加、削除する。
    current_size = this->m_extended_transfer_function_message.m_opacity_transfer_function.size();
    if (current_size <= num_transfer_function) {
        // 追加を行う
        for (n=current_size+1; n<=num_transfer_function; n++) {
            sprintf(name, "O%d", n);
            this->m_extended_transfer_function_message.addOpacityTransferFunction(name, "q1");
        }
    }
    else {
        // 削除を行う
        for (n=num_transfer_function+1; n<=current_size; n++) {
            sprintf(name, "O%d", n);
            this->m_extended_transfer_function_message.removeOpacityTransferFunction(name);
        }
    }
}

void TransferFunctionEditor::onColorFunctionChanged( int index )
{
    const NamedTransferFunctionParameter *transfer_function_color = m_extended_transfer_function_message.getColorTransferFunction( index + 1 );
    if( transfer_function_color != NULL )
    {
        kvs::ColorMap color_map = transfer_function_color->colorMap();
        ui->colorMapBar->setColorMap( color_map );
        //        const kvs::visclient::FrequencyTable* freq_table = extCommand->m_result.findColorFrequencyTable(trans_color->m_name);
        ui->transfer_function_var_color->setText(  transfer_function_color->m_color_variable.c_str() );
        ui->transfer_function_min_color->setValue( transfer_function_color->m_color_variable_min );
        ui->transfer_function_max_color->setValue( transfer_function_color->m_color_variable_max );
    }
}

void TransferFunctionEditor::onOpacityFunctionChanged( int index )
{
    const NamedTransferFunctionParameter *transfer_function_opacity = m_extended_transfer_function_message.getOpacityTransferFunction( index + 1 );
    if( transfer_function_opacity != NULL )
    {
        kvs::OpacityMap opacity_map = transfer_function_opacity->opacityMap();
        ui->opacityMapPalette->setOpacityMap( opacity_map );
        //        const kvs::visclient::FrequencyTable* freq_table = extCommand->m_result.findColorFrequencyTable(trans_color->m_name);
        ui->transfer_function_var_opacity->setText(  transfer_function_opacity->m_opacity_variable.c_str() );
        ui->transfer_function_min_opacity->setValue( transfer_function_opacity->m_opacity_variable_min );
        ui->transfer_function_max_opacity->setValue( transfer_function_opacity->m_opacity_variable_max );
    }
}

void TransferFunctionEditor::onColorFunctionSynthEdited( const QString &arg1 )
{
    m_extended_transfer_function_message.m_color_transfer_function_synthesis = arg1.toStdString();
}

void TransferFunctionEditor::onOpacityFunctionSynthEdited( const QString &arg1 )
{
    m_extended_transfer_function_message.m_color_transfer_function_synthesis = arg1.toStdString();
}

void TransferFunctionEditor::onTransferFunctionVarColorEdited( const QString &arg1 )
{
    std::string fName = ui->colorFunctionCBox->currentText().toStdString();
    std::string variable = ui->transfer_function_var_color->text().toStdString();

    bool exists = m_extended_transfer_function_message.getColorTransferFunction( fName ) != NULL;
    m_extended_transfer_function_message.setColorTransferFunction(fName, variable);

    if( exists )
    {
        m_extended_transfer_function_message.setColorTransferRange( fName, ui->transfer_function_min_color->value(), ui->transfer_function_max_color->value() );
    }
}

void TransferFunctionEditor::onTransferFunctionVarOpacityEdited( const QString &arg1 )
{
    std::string fName = ui->opacityFunctionCBox->currentText().toStdString();
    std::string variable = ui->transfer_function_var_opacity->text().toStdString();

    bool exists = m_extended_transfer_function_message.getOpacityTransferFunction( fName ) != NULL;
    m_extended_transfer_function_message.setOpacityTransferFunction(fName, variable);

    if( exists )
    {
        m_extended_transfer_function_message.setOpacityTransferRange( fName, ui->transfer_function_min_opacity->value(), ui->transfer_function_max_opacity->value() );
    }
}

void TransferFunctionEditor::populateColorFunctionLists(int n){
    QStringList itemList;
    for (int i=1;i <=n;i++ )
    {
        itemList.append(QString("C%1").arg(i));
    }
    ui->colorFunctionCBox->clear();
    ui->colorFunctionCBox->addItems(itemList);
}

void TransferFunctionEditor::populateOpacityFunctionLists(int n){
    QStringList itemList;
    for (int i=1;i <=n;i++ )
    {
        itemList.append(QString("O%1").arg(i));
    }
    ui->opacityFunctionCBox->clear();
    ui->opacityFunctionCBox->addItems(itemList);
}

void TransferFunctionEditor::onColorMapEditorButtonClicked()
{
    if( m_color_map_editor.exec() == QDialog::Accepted )
    {
        ui->colorMapBar->setColorMap( m_color_map_editor.getColorMap() );
    }
}

void TransferFunctionEditor::onOpacityMapEditorButtonClicked()
{
    if( m_opacity_map_editor.exec() == QDialog::Accepted )
    {
        ui->opacityMapPalette->setOpacityMap( m_opacity_map_editor.getOpacityMap() );
    }
}

void TransferFunctionEditor::onTransferFunctionRangeColorChanged()
{
    m_extended_transfer_function_message.setColorTransferRange( ui->colorFunctionCBox->currentText().toStdString(), ui->transfer_function_min_color->value(), ui->transfer_function_max_color->value() );
}

void TransferFunctionEditor::onTransferFunctionRangeOpacityChanged()
{
    m_extended_transfer_function_message.setOpacityTransferRange( ui->opacityFunctionCBox->currentText().toStdString(), ui->transfer_function_min_opacity->value(), ui->transfer_function_max_opacity->value() );
}

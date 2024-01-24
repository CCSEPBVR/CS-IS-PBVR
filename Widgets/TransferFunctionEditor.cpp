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

    connect( ui->applyPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onApplyButtonClicked );

    populateColorFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );
    populateOpacityFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

jpv::ParticleTransferClientMessage::EquationToken TransferFunctionEditor::convertToken( std::string expression )
{
    FuncParser::ExpressionTokenizer tokenizer;
    FuncParser::ExpressionConverter exprconv;

    jpv::ParticleTransferClientMessage::EquationToken eq_token;

    tokenizer.tokenizeString( expression );
    exprconv.convertExpToken( tokenizer.m_exp_token );
    int size = exprconv.token_array.size();
    if( size > 128 ){ printf("Equation length too long\n");}

    for( int i = 0; i < 128; i++ )
    {
        if( i < size )
        {
            eq_token.exp_token[i]   = exprconv.token_array[i];
            eq_token.var_name[i]    = exprconv.var_array[i];
            eq_token.value_array[i] = exprconv.value_array[i];
        }
        else
        {
            eq_token.exp_token[i]   = 0;
            eq_token.var_name[i]    = 0;
            eq_token.value_array[i] = 0;
        }
    }

    std::cout << "exp" << std::endl;
    for(int i = 0; i < 128; i++)
    {
        std::cout << eq_token.exp_token[i] << ",";
    }
    std::cout << std::endl;
    std::cout << "var_name" << std::endl;
    for(int i = 0; i < 128; i++)
    {
        std::cout << eq_token.var_name[i] << ",";
    }
    std::cout << std::endl;
    std::cout << "value_array" << std::endl;
    for(int i = 0; i < 128; i++)
    {
        std::cout << eq_token.value_array[i] << ",";
    }
    std::cout << std::endl;

    return eq_token;
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
        ui->colorMapBar->update();
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
        ui->opacityMapPalette->update();
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
    m_extended_transfer_function_message.m_opacity_transfer_function_synthesis = arg1.toStdString();
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
    int n_select_color = ui->colorFunctionCBox->currentIndex() + 1;
    const NamedTransferFunctionParameter *trans_color = m_extended_transfer_function_message.getColorTransferFunction( n_select_color );
    kvs::ColorMap color_map = trans_color->colorMap();

    if( m_color_map_editor.exec() == QDialog::Accepted )
    {
//        ui->colorMapBar->setColorMap( m_color_map_editor.getColorMap() );
        const kvs::ColorMap cmap = m_color_map_editor.getColorMap();
        ui->colorMapBar->setColorMap( cmap );
        int n_select_color = ui->colorFunctionCBox->currentIndex()+1;
        NamedTransferFunctionParameter *tc = m_extended_transfer_function_message.getColorTransferFunction(n_select_color);
        tc->setColorMap( cmap );
    }
}

void TransferFunctionEditor::onOpacityMapEditorButtonClicked()
{
    int n_select_opacity = ui->opacityFunctionCBox->currentIndex() + 1;
    const NamedTransferFunctionParameter *trans_opacity = m_extended_transfer_function_message.getOpacityTransferFunction( n_select_opacity );
    kvs::OpacityMap opacity_map = trans_opacity->opacityMap();

    if( m_opacity_map_editor.exec() == QDialog::Accepted )
    {
//        ui->opacityMapPalette->setOpacityMap( m_opacity_map_editor.getOpacityMap() );
        const kvs::OpacityMap omap = m_opacity_map_editor.getOpacityMap();
        ui->opacityMapPalette->setOpacityMap( omap );
        int n_select_opacity = ui->opacityFunctionCBox->currentIndex() + 1;
        NamedTransferFunctionParameter *tc = m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity);
        tc->setOpacityMap( omap );
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

void TransferFunctionEditor::onApplyButtonClicked()
{
    //applyToClientMessage
    m_client_message->m_transfer_function.clear();
    m_client_message->m_volume_equation.clear();
    m_client_message->color_var.clear();
    m_client_message->opacity_var.clear();

    m_client_message->m_color_transfer_function_synthesis = m_extended_transfer_function_message.m_color_transfer_function_synthesis;
    m_client_message->m_opacity_transfer_function_synthesis = m_extended_transfer_function_message.m_opacity_transfer_function_synthesis;

//    for ( size_t i = 0; i < this->m_color_transfer_function.size(); i++ )
    for ( size_t i = 0; i < m_extended_transfer_function_message.m_color_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq;

        const NamedTransferFunctionParameter& tf = m_extended_transfer_function_message.m_color_transfer_function[i];
        etf = tf;
        int func_num = etf.getNameNumber();
        std::stringstream ss;
        ss << "_F" << func_num;
        etf.m_color_variable   = ss.str() + "_VAR_C";
        veq.m_name     = etf.m_color_variable;
        veq.m_equation = m_extended_transfer_function_message.m_color_transfer_function[i].m_color_variable;
        m_client_message->m_transfer_function.push_back( etf );
        m_client_message->m_volume_equation.push_back( veq );
    }

    for ( size_t i = 0; i < m_extended_transfer_function_message.m_opacity_transfer_function.size(); i++ )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq;

        const NamedTransferFunctionParameter& tf = m_extended_transfer_function_message.m_opacity_transfer_function[i];
        etf = tf;
        int func_num = etf.getNameNumber();
        std::stringstream ss;
        ss << "_F" << func_num;
        etf.m_opacity_variable   = ss.str() + "_VAR_O";
        veq.m_name     = etf.m_opacity_variable;
        veq.m_equation = m_extended_transfer_function_message.m_opacity_transfer_function[i].m_opacity_variable;
        m_client_message->m_transfer_function.push_back( etf );
        m_client_message->m_volume_equation.push_back( veq );
    }

    std::string colorSynthBuf = m_extended_transfer_function_message.m_color_transfer_function_synthesis;
    std::replace(colorSynthBuf.begin(), colorSynthBuf.end(), 'C', 'c');
    m_client_message->color_func = this->convertToken( colorSynthBuf );
    std::string opacitySynthBuf = m_extended_transfer_function_message.m_opacity_transfer_function_synthesis;
    std::replace(opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a');
    m_client_message->opacity_func = this->convertToken( opacitySynthBuf );

    for ( size_t i = 0; i < m_extended_transfer_function_message.m_color_transfer_function.size(); i++ )
    {
        m_client_message->color_var.push_back( this->convertToken( m_extended_transfer_function_message.m_color_transfer_function[i].m_color_variable ) );
        m_client_message->opacity_var.push_back( this->convertToken( m_extended_transfer_function_message.m_opacity_transfer_function[i].m_opacity_variable ) );
    }

    //2023 shimomura
    std::cout <<" message->m_x_synthesis ="  << m_client_message->m_x_synthesis << std::endl;
    if( !m_client_message->m_x_synthesis.empty() ) m_client_message->x_synthesis_token = this->convertToken( m_client_message->m_x_synthesis );
    if( !m_client_message->m_y_synthesis.empty() ) m_client_message->y_synthesis_token = this->convertToken( m_client_message->m_y_synthesis );
    if( !m_client_message->m_z_synthesis.empty() ) m_client_message->z_synthesis_token = this->convertToken( m_client_message->m_z_synthesis );
}

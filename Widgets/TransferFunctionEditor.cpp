#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"
#include "Widgets/MergePanel.h"
#include <kvs/ColorMapBar>
#include <kvs/OpacityMapBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "Widgets/FrequencyTable.h"
#include <QFileDialog>
#include "ParameterFile.h"
#include <fstream>

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferFunctionEditor),    
    m_client_message( nullptr ),
    m_server_message( nullptr ),
    m_received_message( nullptr ),
    m_is_import_transfer_function_parameter( false ),
    m_color_function_list_editor( this ),
    m_opacity_function_list_editor( this ),
    m_color_map_editor( this ),
    m_opacity_map_editor( this ),
    m_extended_transfer_function_message(),
    m_extended_transfer_function_message_initial()
{
    ui->setupUi(this);

    populateColorFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );
    populateOpacityFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );

    connect( ui->numberOfTransferFunctionSBox, &QSpinBox::valueChanged, this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );
    connect( ui->colorFunctionCBox, &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onColorFunctionChanged );
    connect( ui->opacityFunctionCBox, &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onOpacityFunctionChanged );

    connect( ui->color_function_synth, &QLineEdit::textEdited, this, &TransferFunctionEditor::onColorFunctionSynthEdited );
    connect( ui->opacity_function_synth, &QLineEdit::textEdited, this, &TransferFunctionEditor::onOpacityFunctionSynthEdited );

    connect( ui->transfer_function_var_color, &QLineEdit::textEdited, this, &TransferFunctionEditor::onTransferFunctionVarColorEdited );
    connect( ui->transfer_function_var_opacity, &QLineEdit::textEdited, this, &TransferFunctionEditor::onTransferFunctionVarOpacityEdited );


    connect( ui->colorFunctionListEditorPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onColorFunctionListEditorButtonClicked );
    connect( ui->opacityFunctionListEditorPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onOpacityFunctionListEditorButtonClicked );

    connect( ui->editColorMapPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onColorMapEditorButtonClicked );
    connect( ui->editOpacityMapPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onOpacityMapEditorButtonClicked );

    connect( ui->transfer_function_min_color, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeColorChanged );
    connect( ui->transfer_function_max_color, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeColorChanged );

    connect( ui->transfer_function_min_opacity, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeOpacityChanged );
    connect( ui->transfer_function_max_opacity, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onTransferFunctionRangeOpacityChanged );

    connect( ui->applyPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onApplyButtonClicked );
    connect( ui->importPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onImportButtonClicked );
    connect( ui->exportPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onExportButtonClicked );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::applyVariableRange( const VariableRange& range )
{
    if( m_is_import_transfer_function_parameter == false )
    {
        bool isRangeInitialized = false;
        for( size_t i = 0; i < m_extended_transfer_function_message.m_color_transfer_function.size(); i++ )
        {
            if( !m_extended_transfer_function_message.m_color_transfer_function[i].m_range_initialized )
            {
                std::stringstream ss;
                ss << "t" << (i + 1);
                const std::string tag_c = ss.str() + "_var_c";
                m_extended_transfer_function_message.m_color_transfer_function[i].m_color_variable_min   = range.min( tag_c );
                m_extended_transfer_function_message.m_color_transfer_function[i].m_color_variable_max   = range.max( tag_c );
                m_extended_transfer_function_message.m_color_transfer_function[i].m_range_initialized = true;
                isRangeInitialized = true;
            }
        }

        for( size_t i = 0; i < m_extended_transfer_function_message.m_opacity_transfer_function.size(); i++ )
        {
            if( !m_extended_transfer_function_message.m_opacity_transfer_function[i].m_range_initialized )
            {
                std::stringstream ss;
                ss << "t" << (i + 1);
                const std::string tag_c = ss.str() + "_var_o";
                m_extended_transfer_function_message.m_opacity_transfer_function[i].m_opacity_variable_min   = range.min( tag_c );
                m_extended_transfer_function_message.m_opacity_transfer_function[i].m_opacity_variable_max   = range.max( tag_c );
                m_extended_transfer_function_message.m_opacity_transfer_function[i].m_range_initialized = true;
                isRangeInitialized = true;
            }
        }

        if( isRangeInitialized )
        {
            updateRangeEdit();
        }
    }
}

void TransferFunctionEditor::updateRangeEdit()
{
    onColorFunctionChanged( ui->colorFunctionCBox->currentIndex() );
    onOpacityFunctionChanged( ui->opacityFunctionCBox->currentIndex() );
}

//void TransferFunctionEditor::updateRangeView( const VariableRange& range )
//void TransferFunctionEditor::updateRangeView( const jpv::ParticleTransferServerMessage& reply )
void TransferFunctionEditor::updateRangeView()
{
    int n_select_color = ui->colorFunctionCBox->currentIndex() + 1;
    int n_select_opacity = ui->opacityFunctionCBox->currentIndex() + 1;
    char color_function_name[8] = {0x00};
    char opacity_function_name[8] = {0x00};
    sprintf(color_function_name, "C%d", n_select_color);
    sprintf(opacity_function_name, "O%d", n_select_opacity);
    char tag_c[16] = {0x00};
    char tag_o[16] = {0x00};
    // Temporary string just to prevent crash in Windows debug mode
    std::string tmp_cmf=ui->colorFunctionCBox->currentText().toStdString();
    std::string tmp_omf=ui->opacityFunctionCBox->currentText().toStdString();

    //2023/07/20 changed by shimomura
    sprintf(tag_c, "t%d_var_c", n_select_color);
    sprintf(tag_o, "t%d_var_o", n_select_opacity);

    const NamedTransferFunctionParameter *trans_color = this->m_extended_transfer_function_message.getColorTransferFunction(n_select_color);
    const NamedTransferFunctionParameter *trans_opacity = this->m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity);

    if( trans_color != nullptr )
    {
//        std::vector<size_t> color_table( 1024, 0 );
//        ui->colorHistogramBar->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, color_table.size(), color_table.data() ) );
//        kvs::visclient::FrequencyTable color_freq_table = kvs::visclient::FrequencyTable(  0.0,  1.0, reply.m_color_nbins[ui->colorFunctionCBox->currentIndex()], (size_t *)reply.m_color_bins[ui->colorFunctionCBox->currentIndex()], color_function_name );
//        ui->colorHistogramBar->setTable( color_freq_table );
//        ui->colorHistogramBar->setRange( m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_min, m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_max );
//        ui->colorMinRangeLbl->setNum( m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_min );
//        ui->colorMaxRangeLbl->setNum( m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_max );

        const kvs::visclient::FrequencyTable* freq_table = m_received_message->findColorFrequencyTable(trans_color->m_name);
        if ( freq_table != NULL )
        {
            QMetaObject::invokeMethod(this, [this, freq_table, n_select_color]() {
                    // ここに UI の更新などを行う
                    ui->colorHistogramBar->setTable(*freq_table);
                    ui->colorHistogramBar->setRange(m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_min, m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_max);
                    ui->colorHistogramBar->update();
                    ui->colorMinRangeLbl->setNum(m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_min);
                    ui->colorMaxRangeLbl->setNum(m_extended_transfer_function_message.getColorTransferFunction(n_select_color)->m_color_variable_max);
                }, Qt::QueuedConnection);
        }
    }

    if( trans_opacity != nullptr )
    {
//        std::vector<size_t> opacity_table( 1024, 0 );
//        ui->opacityHistogramBar->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, opacity_table.size(), opacity_table.data() ) );
//        kvs::visclient::FrequencyTable opacity_freq_table = kvs::visclient::FrequencyTable( 0.0, 1.0, reply.m_opacity_nbins[ui->opacityFunctionCBox->currentIndex()], (size_t *)reply.m_opacity_bins[ui->opacityFunctionCBox->currentIndex()], opacity_function_name );
//        ui->opacityHistogramBar->setTable( opacity_freq_table );
//        ui->opacityHistogramBar->setRange( m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_min, m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_max );
//        ui->opacityMinRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_min );
//        ui->opacityMaxRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_max );
        const kvs::visclient::FrequencyTable* freq_table = m_received_message->findOpacityFrequencyTable(trans_opacity->m_name);
        if ( freq_table != NULL )
        {
            QMetaObject::invokeMethod(this, [this, freq_table, n_select_opacity]() {
                    ui->opacityHistogramBar->setTable( *freq_table );
                    ui->opacityHistogramBar->setRange( m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_min, m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_max );
                    ui->opacityHistogramBar->update();
                    ui->opacityMinRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_min );
                    ui->opacityMaxRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(n_select_opacity)->m_opacity_variable_max );
                }, Qt::QueuedConnection);
        }
    }
    QMetaObject::invokeMethod( this, [this, tag_c, tag_o]()
        {
            ui->range_min_color->setText( QString::number( m_server_message->m_variable_range.min( tag_c ) ) );
            ui->range_max_color->setText( QString::number( m_server_message->m_variable_range.max( tag_c ) ) );
            ui->range_min_opacity->setText( QString::number( m_server_message->m_variable_range.min( tag_o ) ) );
            ui->range_max_opacity->setText( QString::number( m_server_message->m_variable_range.max( tag_o ) ) );
        }, Qt::QueuedConnection);

//    ui->colorHistogramBar->update();
//    ui->opacityHistogramBar->update();
}

//jpv::ParticleTransferClientMessage::EquationToken TransferFunctionEditor::convertToken( std::string expression )
//{
//    FuncParser::ExpressionTokenizer tokenizer;
//    FuncParser::ExpressionConverter exprconv;

//    jpv::ParticleTransferClientMessage::EquationToken eq_token;

//    tokenizer.tokenizeString( expression );
//    exprconv.convertExpToken( tokenizer.m_exp_token );
//    int size = exprconv.token_array.size();
//    if( size > 128 ){ printf("Equation length too long\n");}

//    for( int i = 0; i < 128; i++ )
//    {
//        if( i < size )
//        {
//            eq_token.exp_token[i]   = exprconv.token_array[i];
//            eq_token.var_name[i]    = exprconv.var_array[i];
//            eq_token.value_array[i] = exprconv.value_array[i];
//        }
//        else
//        {
//            eq_token.exp_token[i]   = 0;
//            eq_token.var_name[i]    = 0;
//            eq_token.value_array[i] = 0;
//        }
//    }

//    std::cout << "exp" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.exp_token[i] << ",";
//    }
//    std::cout << std::endl;
//    std::cout << "var_name" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.var_name[i] << ",";
//    }
//    std::cout << std::endl;
//    std::cout << "value_array" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.value_array[i] << ",";
//    }
//    std::cout << std::endl;

//    return eq_token;
//}

void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( int value )
{
    int num_transfer_function = value;
    m_extended_transfer_function_message.m_transfer_function_number = num_transfer_function;
    this->populateColorFunctionLists( value );
    this->populateOpacityFunctionLists( value );
    m_color_function_selector->populateColorFunctionLists( value );

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

        const kvs::visclient::FrequencyTable* freq_table = m_received_message->findColorFrequencyTable(transfer_function_color->m_name);
        if ( freq_table != NULL )
        {
            ui->colorHistogramBar->setTable( *freq_table );
            ui->colorHistogramBar->setRange( m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_min, m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_max );
            ui->colorHistogramBar->update();
            ui->colorMinRangeLbl->setNum( m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_min );
            ui->colorMaxRangeLbl->setNum( m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_max );

            char tag_c[16] = {0x00};
            sprintf(tag_c, "t%d_var_c", index + 1);
            ui->range_min_color->setText( QString::number( m_server_message->m_variable_range.min( tag_c ) ) );
            ui->range_max_color->setText( QString::number( m_server_message->m_variable_range.max( tag_c ) ) );
        }

        ui->transfer_function_var_color->blockSignals(true);
        ui->transfer_function_min_color->blockSignals(true);
        ui->transfer_function_max_color->blockSignals(true);

        ui->transfer_function_var_color->setText(  transfer_function_color->m_color_variable.c_str() );
        ui->transfer_function_min_color->setValue( transfer_function_color->m_color_variable_min );
        ui->transfer_function_max_color->setValue( transfer_function_color->m_color_variable_max );

        ui->transfer_function_var_color->blockSignals(false);
        ui->transfer_function_min_color->blockSignals(false);
        ui->transfer_function_max_color->blockSignals(false);
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

        const kvs::visclient::FrequencyTable* freq_table = m_received_message->findOpacityFrequencyTable(transfer_function_opacity->m_name);
        if ( freq_table != NULL )
        {
            ui->opacityHistogramBar->setTable( *freq_table );
            ui->opacityHistogramBar->setRange( m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_min, m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_max );
            ui->opacityHistogramBar->update();
            ui->opacityMinRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_min );
            ui->opacityMaxRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_max );

            char tag_o[16] = {0x00};
            sprintf(tag_o, "t%d_var_o", index + 1);
            ui->range_min_opacity->setText( QString::number( m_server_message->m_variable_range.min( tag_o ) ) );
            ui->range_max_opacity->setText( QString::number( m_server_message->m_variable_range.max( tag_o ) ) );
        }

        ui->transfer_function_var_opacity->blockSignals(true);
        ui->transfer_function_min_opacity->blockSignals(true);
        ui->transfer_function_max_opacity->blockSignals(true);

        ui->transfer_function_var_opacity->setText(  transfer_function_opacity->m_opacity_variable.c_str() );
        ui->transfer_function_min_opacity->setValue( transfer_function_opacity->m_opacity_variable_min );
        ui->transfer_function_max_opacity->setValue( transfer_function_opacity->m_opacity_variable_max );

        ui->transfer_function_var_opacity->blockSignals(false);
        ui->transfer_function_min_opacity->blockSignals(false);
        ui->transfer_function_max_opacity->blockSignals(false);
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

void TransferFunctionEditor::onColorFunctionListEditorButtonClicked()
{
    m_color_function_list_editor.initalize( FunctionListEditor::COLOR_FUNCTION, m_extended_transfer_function_message );
    m_color_function_list_editor.exec();
    onColorFunctionChanged( ui->colorFunctionCBox->currentIndex() );
}

void TransferFunctionEditor::onOpacityFunctionListEditorButtonClicked()
{
    m_opacity_function_list_editor.initalize( FunctionListEditor::OPACITY_FUNCTION, m_extended_transfer_function_message );
    m_opacity_function_list_editor.exec();
    onOpacityFunctionChanged( ui->opacityFunctionCBox->currentIndex() );
}

void TransferFunctionEditor::onColorMapEditorButtonClicked()
{
    int n_select_color = ui->colorFunctionCBox->currentIndex() + 1;
    const NamedTransferFunctionParameter *trans_color = m_extended_transfer_function_message.getColorTransferFunction( n_select_color );
    kvs::ColorMap color_map = trans_color->colorMap();

    m_color_map_editor.setColorMap( ui->colorMapBar->getColor() );
    m_color_map_editor.setInitialColorMap( ui->colorMapBar->getColor() );
    m_color_map_editor.clearUndoStack();

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

    m_opacity_map_editor.setOpacityMap( ui->opacityMapPalette->getOpacity() );
    m_opacity_map_editor.setInitialOpacityMap( ui->opacityMapPalette->getOpacity() );
    m_opacity_map_editor.clearUndoStack();

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
//    m_client_message->color_func = this->convertToken( colorSynthBuf );
//    m_client_message->color_func = colorSynthBuf;
    std::string opacitySynthBuf = m_extended_transfer_function_message.m_opacity_transfer_function_synthesis;
    std::replace(opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a');
//    m_client_message->opacity_func = this->convertToken( opacitySynthBuf );

//    for ( size_t i = 0; i < m_extended_transfer_function_message.m_color_transfer_function.size(); i++ )
//    {
//        m_client_message->color_var.push_back( this->convertToken( m_extended_transfer_function_message.m_color_transfer_function[i].m_color_variable ) );
//        m_client_message->opacity_var.push_back( this->convertToken( m_extended_transfer_function_message.m_opacity_transfer_function[i].m_opacity_variable ) );
//    }

    //2023 shimomura
//    std::cout <<" message->m_x_synthesis ="  << m_client_message->m_x_synthesis << std::endl;
//    m_client_message->m_x_synthesis = "X*2";
//    if( !m_client_message->m_x_synthesis.empty() ) m_client_message->x_synthesis_token = this->convertToken( m_client_message->m_x_synthesis );
//    if( !m_client_message->m_y_synthesis.empty() ) m_client_message->y_synthesis_token = this->convertToken( m_client_message->m_y_synthesis );
//    if( !m_client_message->m_z_synthesis.empty() ) m_client_message->z_synthesis_token = this->convertToken( m_client_message->m_z_synthesis );
    m_merge->setIsParticleGenerationNeeded( true );
    m_color_function_selector->updateColorMap();
}

void TransferFunctionEditor::onImportButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr("Import Transfer Function File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );
    this->importFile(fileName.toStdString());
}

void TransferFunctionEditor::importFile( const std::string& fname )
{
    ParameterFile param;
    bool stat;

    stat = param.loadIN( fname );

    if ( !stat ) return;
    ExtendedTransferFunctionMessage importdoc;

    const size_t resolution = param.getInt( "tf_resolution" );
    importdoc.m_extend_transfer_function_resolution =  resolution;

    std::cout<<"TF_RESOLUTION;"<<resolution<<std::endl;
    if (!param.hasParam("TF_SYNTH_C") || !param.hasParam("TF_SYNTH_O"))
    {
//        std::cerr << "[Error] This import file is old format. file=" << fname << std::endl;
//        QMessageBox::warning(this, QString("Obsolete File Format"), QString("The selected file is using an old unsupported format"));
        return;
    }

    importdoc.m_transfer_function_number = param.getInt( "TF_NUMBER" );
    importdoc.m_color_transfer_function_synthesis = param.getString( "TF_SYNTH_C" );
    importdoc.m_opacity_transfer_function_synthesis = param.getString( "TF_SYNTH_O" );

    importdoc.m_color_transfer_function.clear();
    for ( size_t n = 0; n < importdoc.m_transfer_function_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "C";

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        if (!param.hasParam(name))
        {
            char c_name[8] = {0x00};
            sprintf(c_name, "C%zd", n + 1);
            trans.m_name = std::string(c_name);
            trans.m_color_variable      = "q1";
        }
        else
        {
            trans.m_name = param.getString( tag_base + "C" );
            trans.m_color_variable      = param.getString( tag_base + "VAR_C" );
            trans.m_color_variable_min   = param.getFloat( tag_base + "MIN_C" );
            trans.m_color_variable_max   = param.getFloat( tag_base + "MAX_C" );

            std::string s_color = param.getString( tag_base + "TABLE_C" );
            std::replace( s_color.begin(), s_color.end(), ',', ' ' );
            std::stringstream ss_color( s_color );
            kvs::ColorMap::Table color_table( resolution * 3 );
            for ( size_t i = 0; i < resolution; i++ )
            {
                for ( size_t c = 0; c < 3; c++ )
                {
                    int color_e;
                    ss_color >> color_e;
                    color_table.at( i * 3 + c ) = color_e;
                }
            }
            kvs::ColorMap color_map( color_table );
            trans.setColorMap(color_map);
        }
        if (importdoc.getColorTransferFunction(trans.m_name) == NULL)
        {
            importdoc.m_color_transfer_function.push_back(trans);
        }
    }

    importdoc.m_opacity_transfer_function.clear();
    for ( size_t n = 0; n < importdoc.m_transfer_function_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "O";

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        if (!param.hasParam(name)) {
            char o_name[8] = {0x00};
            sprintf(o_name, "O%zd", n + 1);
            trans.m_name = std::string(o_name);
            trans.m_opacity_variable      = "q1";
        }
        else {
            trans.m_name = param.getString( tag_base + "O" );
            trans.m_opacity_variable    = param.getString( tag_base + "VAR_O" );
            trans.m_opacity_variable_min = param.getFloat( tag_base + "MIN_O" );
            trans.m_opacity_variable_max = param.getFloat( tag_base + "MAX_O" );

            std::string s_opacity = param.getString( tag_base + "TABLE_O" );
            std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );
            std::stringstream ss_opacity( s_opacity );
            kvs::OpacityMap::Table opacity_table( resolution );
            for ( size_t i = 0; i < resolution; i++ )
            {
                float opacity;
                ss_opacity >> opacity;
                opacity_table.at( i ) = opacity;
            }
            kvs::OpacityMap opacity_map( opacity_table );
            trans.setOpacityMap(opacity_map);
        }

        if (importdoc.getOpacityTransferFunction(trans.m_name) == NULL) {
            importdoc.m_opacity_transfer_function.push_back(trans);
        }
    }

//    this->m_doc=importdoc;
    m_extended_transfer_function_message = importdoc;
//    this->m_doc_initial=importdoc;

    //DEL BY)T0603 2020.05.25
    //ui->resolution->setValue(resolution);
    ui->numberOfTransferFunctionSBox->setValue( importdoc.m_transfer_function_number );
    ui->color_function_synth->setText(QString::fromStdString(importdoc.m_color_transfer_function_synthesis) );
    ui->opacity_function_synth->setText(QString::fromStdString(importdoc.m_opacity_transfer_function_synthesis));
    onColorFunctionChanged( ui->colorFunctionCBox->currentIndex() );
    onOpacityFunctionChanged( ui->opacityFunctionCBox->currentIndex() );

    m_is_import_transfer_function_parameter = true;
    onApplyButtonClicked();
}

void TransferFunctionEditor::onExportButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr("Export Current Settings to Parameter File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );
    if( fileName.right(4) != ".tfe" && fileName.right(4) != ".TFE" && fileName.right(3) != ".tf" && fileName.right(3) != ".TF")
    {
        fileName += ".tfe";
    }
    this->exportFile(fileName.toStdString(), false);
}

void TransferFunctionEditor::exportFile( const std::string& fname, const bool append)
{
    std::ofstream ofs;
    if ( append )
    {
        ofs.open( fname.c_str(), std::ofstream::app );
    }
    else
    {
        ofs.open( fname.c_str(), std::ofstream::out );
    }
    if ( ofs.fail() )
    {
        std::cerr << "Error: open file " << fname << std::endl;
        return;
    }

    ofs << "TF_RESOLUTION=" << m_extended_transfer_function_message.m_extend_transfer_function_resolution << std::endl;
    ofs << "TF_NUMBER=" << m_extended_transfer_function_message.m_transfer_function_number << std::endl;
    ofs << "TF_SYNTH_C=" << m_extended_transfer_function_message.m_color_transfer_function_synthesis << std::endl;
    ofs << "TF_SYNTH_O=" << m_extended_transfer_function_message.m_opacity_transfer_function_synthesis << std::endl;

    for ( size_t n = 0; n < m_extended_transfer_function_message.m_color_transfer_function.size(); n++ )
    {
        std::stringstream ss;
        int name_number =0;
        name_number = m_extended_transfer_function_message.m_color_transfer_function[n].getNameNumber();
        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "C=" << m_extended_transfer_function_message.m_color_transfer_function[n].m_name << std::endl;
        ofs << tag_base << "VAR_C=" << m_extended_transfer_function_message.m_color_transfer_function[n].m_color_variable << std::endl;
        ofs << tag_base << "MIN_C=" << m_extended_transfer_function_message.m_color_transfer_function[n].m_color_variable_min << std::endl;
        ofs << tag_base << "MAX_C=" << m_extended_transfer_function_message.m_color_transfer_function[n].m_color_variable_max << std::endl;
        kvs::ColorMap::Table color_table = m_extended_transfer_function_message.m_color_transfer_function[n].colorMap().table();

        ofs << tag_base << "TABLE_C=";
        for ( size_t i = 0; i < color_table.size(); i++ )
        {
            ofs << static_cast<int>( color_table.at( i ) ) << ",";
        }
        ofs << std::endl;
    }

    for ( size_t n = 0; n < m_extended_transfer_function_message.m_opacity_transfer_function.size(); n++ )
    {
        std::stringstream ss;
        int name_number =0;
        name_number = m_extended_transfer_function_message.m_opacity_transfer_function[n].getNameNumber();

        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "O=" << m_extended_transfer_function_message.m_opacity_transfer_function[n].m_name << std::endl;
        ofs << tag_base << "VAR_O=" << m_extended_transfer_function_message.m_opacity_transfer_function[n].m_opacity_variable << std::endl;
        ofs << tag_base << "MIN_O=" << m_extended_transfer_function_message.m_opacity_transfer_function[n].m_opacity_variable_min << std::endl;
        ofs << tag_base << "MAX_O=" << m_extended_transfer_function_message.m_opacity_transfer_function[n].m_opacity_variable_max << std::endl;

        kvs::OpacityMap::Table opacity_table = m_extended_transfer_function_message.m_opacity_transfer_function[n].opacityMap().table();
        ofs << tag_base << "TABLE_O=";
        for ( size_t i = 0; i < opacity_table.size(); i++ )
        {
            ofs << opacity_table.at( i ) << ",";
        }
        ofs << std::endl;
    }
    ofs.close();

    std::cerr << "TransferFunction parameters are exported to " << fname << std::endl;

}

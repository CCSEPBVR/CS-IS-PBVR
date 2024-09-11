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

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent, ColorMapBarSelector* color_map_bar_selector, MergePanel* merge, Connect* connect_panel) :
    QDialog(parent),
    ui(new Ui::TransferFunctionEditor),
    m_color_map_bar_selector( color_map_bar_selector ),
    m_merge( merge ),
    m_connect( connect_panel ),
    m_is_import_transfer_function_parameter( false ),
    m_color_function_list_editor( this ),
    m_opacity_function_list_editor( this ),
    m_color_map_editor( this ),
    m_opacity_map_editor( this ),
    m_extended_transfer_function_message(),
    m_extended_transfer_function_message_initial(),
    m_mode( TransferFunctionEditor::Mode::None )
{
    ui->setupUi(this);

    populateColorFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );
    populateOpacityFunctionLists( m_extended_transfer_function_message.m_transfer_function_number );

    m_is_color_range_sync.resize( m_extended_transfer_function_message.m_transfer_function_number );
    m_is_opacity_range_sync.resize( m_extended_transfer_function_message.m_transfer_function_number );
    for( int i = 0; i < m_extended_transfer_function_message.m_transfer_function_number; i++ )
    {
        m_is_color_range_sync[i] = false;
        m_is_opacity_range_sync[i] = false;
    }

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

    connect( ui->colorRangeSyncTBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onColorRangeSyncToolButtonClicked );
    connect( ui->opacityRangeSyncTBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onOpacityRangeSyncToolButtonClicked );
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

        const kvs::visclient::FrequencyTable* freq_table = m_connect->getReceivedMessage()->findColorFrequencyTable(trans_color->m_name);
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
        const kvs::visclient::FrequencyTable* freq_table = m_connect->getReceivedMessage()->findOpacityFrequencyTable(trans_opacity->m_name);
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
            ui->range_min_color->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.min( tag_c ) ) );
            ui->range_max_color->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.max( tag_c ) ) );
            ui->range_min_opacity->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.min( tag_o ) ) );
            ui->range_max_opacity->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.max( tag_o ) ) );

            bool applyFlag = false;
            for( int i = 0; i < ui->colorFunctionCBox->count(); i++ )
            {
                if( m_is_color_range_sync[i] )
                {
                    QString colorFunctionName = QString("C%1").arg( i + 1 );
                    QString colorTag = QString("t%1_var_c").arg( i + 1 );
                    m_extended_transfer_function_message.setColorTransferRange( colorFunctionName.toStdString(), m_connect->getServerMessage()->m_server_side_variable_range.min( colorTag.toStdString() ), m_connect->getServerMessage()->m_server_side_variable_range.max( colorTag.toStdString() ) );

                    if( i == ui->colorFunctionCBox->currentIndex() )
                    {
                        ui->transfer_function_min_color->blockSignals( true );
                        ui->transfer_function_max_color->blockSignals( true );
                        ui->transfer_function_min_color->setValue( m_connect->getServerMessage()->m_server_side_variable_range.min( colorTag.toStdString() ) );
                        ui->transfer_function_max_color->setValue( m_connect->getServerMessage()->m_server_side_variable_range.max( colorTag.toStdString() ) );
                        ui->transfer_function_min_color->blockSignals( false );
                        ui->transfer_function_max_color->blockSignals( false );
                    }
                    applyFlag = true;
                }
            }

            for( int i = 0; i < ui->opacityFunctionCBox->count(); i++ )
            {
                if( m_is_opacity_range_sync[i] )
                {
                    QString opacityFunctionName = QString("O%1").arg( i + 1 );
                    QString opacityTag = QString("t%1_var_o").arg( i + 1 );
                    m_extended_transfer_function_message.setOpacityTransferRange( opacityFunctionName.toStdString(), m_connect->getServerMessage()->m_server_side_variable_range.min( opacityTag.toStdString() ), m_connect->getServerMessage()->m_server_side_variable_range.max( opacityTag.toStdString() ) );

                    if( i == ui->colorFunctionCBox->currentIndex() )
                    {
                        ui->transfer_function_min_opacity->blockSignals( true );
                        ui->transfer_function_max_opacity->blockSignals( true );
                        ui->transfer_function_min_opacity->setValue( m_connect->getServerMessage()->m_server_side_variable_range.min( opacityTag.toStdString() ) );
                        ui->transfer_function_max_opacity->setValue( m_connect->getServerMessage()->m_server_side_variable_range.max( opacityTag.toStdString() ) );
                        ui->transfer_function_min_opacity->blockSignals( false );
                        ui->transfer_function_max_opacity->blockSignals( false );
                    }
                    applyFlag = true;
                }
            }

            if( applyFlag )
            {
                onApplyButtonClicked();
            }

        }, Qt::QueuedConnection);

//    ui->colorHistogramBar->update();
//    ui->opacityHistogramBar->update();
}

void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( int value )
{
    int num_transfer_function = value;
    m_extended_transfer_function_message.m_transfer_function_number = num_transfer_function;
    this->populateColorFunctionLists( value );
    this->populateOpacityFunctionLists( value );
    m_color_map_bar_selector->populateColorFunctionLists( value );

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

    int currentSize = m_is_color_range_sync.size();

    // 新しいサイズが現在のサイズより大きい場合、追加分を初期化
    if (value > currentSize) {
        m_is_color_range_sync.resize(value);
        m_is_opacity_range_sync.resize(value);
        for (int i = currentSize; i < value; ++i) {
            m_is_color_range_sync[i] = false; // 新しい要素のみ初期化
            m_is_opacity_range_sync[i] = false; // 新しい要素のみ初期化
        }
    } else if (value < currentSize) {
        // 新しいサイズが現在のサイズより小さい場合、要素を削除
        m_is_color_range_sync.resize(value);
        m_is_opacity_range_sync.resize(value);
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
        const kvs::visclient::FrequencyTable* freq_table = m_connect->getReceivedMessage()->findColorFrequencyTable(transfer_function_color->m_name);
        if ( freq_table != NULL )
        {
            ui->colorHistogramBar->setTable( *freq_table );
            ui->colorHistogramBar->setRange( m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_min, m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_max );
            ui->colorHistogramBar->update();
            ui->colorMinRangeLbl->setNum( m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_min );
            ui->colorMaxRangeLbl->setNum( m_extended_transfer_function_message.getColorTransferFunction(index + 1)->m_color_variable_max );
            char tag_c[16] = {0x00};
            sprintf(tag_c, "t%d_var_c", index + 1);
            ui->range_min_color->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.min( tag_c ) ) );
            ui->range_max_color->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.max( tag_c ) ) );
        }

        ui->transfer_function_var_color->blockSignals(true);
        ui->transfer_function_min_color->blockSignals(true);
        ui->transfer_function_max_color->blockSignals(true);
        ui->colorRangeSyncTBtn->blockSignals( true );
        ui->userSideColorRangeLbl->setText( QString( "C%1 Min:Max : " ).arg( index + 1 ) );
        ui->serverSideColorRangeLbl->setText( QString( "C%1 Server side Range Min:Max : " ).arg( index + 1 ) );
        ui->transfer_function_var_color->setText(  transfer_function_color->m_color_variable.c_str() );
        ui->transfer_function_min_color->setValue( transfer_function_color->m_color_variable_min );
        ui->transfer_function_max_color->setValue( transfer_function_color->m_color_variable_max );
        ui->colorRangeSyncTBtn->setChecked( m_is_color_range_sync[index] );

        ui->transfer_function_var_color->blockSignals(false);
        ui->transfer_function_min_color->blockSignals(false);
        ui->transfer_function_max_color->blockSignals(false);
        ui->colorRangeSyncTBtn->blockSignals( false );
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

        const kvs::visclient::FrequencyTable* freq_table = m_connect->getReceivedMessage()->findOpacityFrequencyTable(transfer_function_opacity->m_name);
        if ( freq_table != NULL )
        {
            ui->opacityHistogramBar->setTable( *freq_table );
            ui->opacityHistogramBar->setRange( m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_min, m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_max );
            ui->opacityHistogramBar->update();
            ui->opacityMinRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_min );
            ui->opacityMaxRangeLbl->setNum( m_extended_transfer_function_message.getOpacityTransferFunction(index + 1)->m_opacity_variable_max );

            char tag_o[16] = {0x00};
            sprintf(tag_o, "t%d_var_o", index + 1);
            ui->range_min_opacity->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.min( tag_o ) ) );
            ui->range_max_opacity->setText( QString::number( m_connect->getServerMessage()->m_server_side_variable_range.max( tag_o ) ) );
        }

        ui->transfer_function_var_opacity->blockSignals(true);
        ui->transfer_function_min_opacity->blockSignals(true);
        ui->transfer_function_max_opacity->blockSignals(true);
        ui->opacityRangeSyncTBtn->blockSignals( true );
        ui->userSideOpacityRangeLbl->setText( QString( "O%1 Min:Max : " ).arg( index + 1 ) );
        ui->serverSideOpacityRangeLbl->setText( QString( "O%1 Server side Range Min:Max : " ).arg( index + 1 ) );
        ui->transfer_function_var_opacity->setText(  transfer_function_opacity->m_opacity_variable.c_str() );
        ui->transfer_function_min_opacity->setValue( transfer_function_opacity->m_opacity_variable_min );
        ui->transfer_function_max_opacity->setValue( transfer_function_opacity->m_opacity_variable_max );
        ui->opacityRangeSyncTBtn->setChecked( m_is_opacity_range_sync[index] );

        ui->transfer_function_var_opacity->blockSignals(false);
        ui->transfer_function_min_opacity->blockSignals(false);
        ui->transfer_function_max_opacity->blockSignals(false);
        ui->opacityRangeSyncTBtn->blockSignals( false );
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
    m_connect->getClientMessage()->m_transfer_function.clear();
    m_connect->getClientMessage()->m_volume_equation.clear();
    m_connect->getClientMessage()->color_var.clear();
    m_connect->getClientMessage()->opacity_var.clear();

    m_connect->getClientMessage()->m_color_transfer_function_synthesis = m_extended_transfer_function_message.m_color_transfer_function_synthesis;
    m_connect->getClientMessage()->m_opacity_transfer_function_synthesis = m_extended_transfer_function_message.m_opacity_transfer_function_synthesis;

        for( size_t i = 0; i < m_extended_transfer_function_message.m_transfer_function_number; i++ )
        {
            NamedTransferFunctionParameter etf;
            jpv::ParticleTransferClientMessage::VolumeEquation veq_c, veq_o;

            const NamedTransferFunctionParameter& c_tf = m_extended_transfer_function_message.m_color_transfer_function[i];
            const NamedTransferFunctionParameter& o_tf = m_extended_transfer_function_message.m_opacity_transfer_function[i];

            etf = c_tf;

            std::stringstream ss;
            ss << "_F" << i;

            etf.m_color_variable   = ss.str() + "_VAR_C";
            etf.m_opacity_variable = ss.str() + "_VAR_O";
            veq_c.m_name     = etf.m_color_variable;
            veq_o.m_name     = etf.m_opacity_variable;
            veq_c.m_equation = m_extended_transfer_function_message.m_color_transfer_function[i].m_color_variable;
            veq_o.m_equation = m_extended_transfer_function_message.m_opacity_transfer_function[i].m_opacity_variable;

            std::replace(etf.m_name.begin(), etf.m_name.end(), 'C', 't');
            etf.setOpacityMap( o_tf.opacityMap() );
            etf.m_equation_opacity = o_tf.m_equation_opacity;
            etf.m_opacity_variable_min = o_tf.m_opacity_variable_min;
            etf.m_opacity_variable_max = o_tf.m_opacity_variable_max;

            m_connect->getClientMessage()->m_transfer_function.push_back( etf );
            m_connect->getClientMessage()->m_volume_equation.push_back( veq_c );
            m_connect->getClientMessage()->m_volume_equation.push_back( veq_o );
        }

        std::string colorSynthBuf = m_extended_transfer_function_message.m_color_transfer_function_synthesis;
        std::replace(colorSynthBuf.begin(), colorSynthBuf.end(), 'C', 'c');

        std::string opacitySynthBuf = m_extended_transfer_function_message.m_opacity_transfer_function_synthesis;
        std::replace(opacitySynthBuf.begin(), opacitySynthBuf.end(), 'O', 'a');

        m_merge->setIsParticleGenerationNeeded( true );
        m_color_map_bar_selector->updateColorMap();
}

void TransferFunctionEditor::onImportButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr("Import Transfer Function File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );

#ifdef Q_OS_WIN
    this->importFile( fileName.toLocal8Bit().constData() );
#else
    this->importFile( fileName.toStdString() );
#endif
}

bool TransferFunctionEditor::importFile( const std::string& fname )
{
    ParameterFile param;
    bool stat;

    stat = param.loadIN( fname );

    if ( !stat )
    {
        return false;
    }
    ExtendedTransferFunctionMessage importdoc;

    const size_t resolution = param.getInt( "tf_resolution" );
    importdoc.m_extend_transfer_function_resolution =  resolution;

    std::cout<<"TF_RESOLUTION;"<<resolution<<std::endl;
    if (!param.hasParam("TF_SYNTH_C") || !param.hasParam("TF_SYNTH_O"))
    {
//        std::cerr << "[Error] This import file is old format. file=" << fname << std::endl;
//        QMessageBox::warning(this, QString("Obsolete File Format"), QString("The selected file is using an old unsupported format"));
        return false;
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

#ifdef Q_OS_WIN
    ui->color_function_synth->setText( QString::fromUtf8(importdoc.m_color_transfer_function_synthesis.c_str() ) );
    ui->opacity_function_synth->setText( QString::fromUtf8(importdoc.m_opacity_transfer_function_synthesis.c_str() ) );
#else
    ui->color_function_synth->setText( QString::fromStdString(importdoc.m_color_transfer_function_synthesis ) );
    ui->opacity_function_synth->setText( QString::fromStdString(importdoc.m_opacity_transfer_function_synthesis ) );
#endif

    onColorFunctionChanged( ui->colorFunctionCBox->currentIndex() );
    onOpacityFunctionChanged( ui->opacityFunctionCBox->currentIndex() );

    m_is_import_transfer_function_parameter = true;
    onApplyButtonClicked();
    return true;
}

void TransferFunctionEditor::onExportButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr("Export Current Settings to Parameter File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );
    if( fileName.right(4) != ".tfe" && fileName.right(4) != ".TFE" && fileName.right(3) != ".tf" && fileName.right(3) != ".TF")
    {
        fileName += ".tfe";
    }
#ifdef Q_OS_WIN
    this->exportFile( fileName.toLocal8Bit().constData(), false );
#else
    this->exportFile( fileName.toStdString(), false );
#endif
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

void TransferFunctionEditor::importFromServerIS()
{
    m_extended_transfer_function_message.m_extend_transfer_function_resolution= 256;

    //m_extended_transfer_function_message->transferFunction.clear();
    m_extended_transfer_function_message.m_color_transfer_function.clear();
    m_extended_transfer_function_message.m_opacity_transfer_function.clear();

    // set tf_number
    m_extended_transfer_function_message.m_transfer_function_number = m_connect->getServerMessage()->m_transfer_function.size();
    qDebug() << m_extended_transfer_function_message.m_transfer_function_number;
    for ( size_t i = 0, j = 0; i < m_connect->getServerMessage()->m_transfer_function.size(); i++, j+=2 )
    {
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation veq_c, veq_o;

        const NamedTransferFunctionParameter& tf = m_connect->getServerMessage()->m_transfer_function[i];

        etf = tf;

        //m_extended_transfer_function_message->transferFunction.push_back( etf );
        m_extended_transfer_function_message.m_color_transfer_function.push_back(etf);
        m_extended_transfer_function_message.m_opacity_transfer_function.push_back(etf);

        std::string* nameBuf = &m_extended_transfer_function_message.m_color_transfer_function[i].m_name;
        std::replace(nameBuf->begin(), nameBuf->end(), 't', 'C');
        nameBuf = &m_extended_transfer_function_message.m_opacity_transfer_function[i].m_name;
        std::replace(nameBuf->begin(), nameBuf->end(), 't', 'O');
    }

    m_extended_transfer_function_message.m_color_transfer_function_synthesis = m_connect->getServerMessage()->m_color_transfer_function_synthesis;
    m_extended_transfer_function_message.m_opacity_transfer_function_synthesis = m_connect->getServerMessage()->m_opacity_transfer_function_synthesis;

    ui->numberOfTransferFunctionSBox->setValue( m_extended_transfer_function_message.m_transfer_function_number );

#ifdef Q_OS_WIN
    ui->color_function_synth->setText( QString::fromUtf8(importdoc.m_color_transfer_function_synthesis.c_str() ) );
    ui->opacity_function_synth->setText( QString::fromUtf8(importdoc.m_opacity_transfer_function_synthesis.c_str() ) );
#else
    ui->color_function_synth->setText( QString::fromStdString( m_extended_transfer_function_message.m_color_transfer_function_synthesis ) );
    ui->opacity_function_synth->setText( QString::fromStdString( m_extended_transfer_function_message.m_opacity_transfer_function_synthesis ) );
#endif

    qDebug() << QString::fromStdString( m_extended_transfer_function_message.m_color_transfer_function_synthesis );
    qDebug() << QString::fromStdString( m_extended_transfer_function_message.m_color_transfer_function[0].m_color_variable );
    qDebug() << m_extended_transfer_function_message.m_color_transfer_function[0].m_color_variable_min;
    qDebug() << m_extended_transfer_function_message.m_color_transfer_function[0].m_color_variable_max;

    qDebug() << QString::fromStdString( m_extended_transfer_function_message.m_opacity_transfer_function_synthesis );
    qDebug() << QString::fromStdString( m_extended_transfer_function_message.m_opacity_transfer_function[0].m_opacity_variable );
    qDebug() << m_extended_transfer_function_message.m_opacity_transfer_function[0].m_opacity_variable_min;
    qDebug() << m_extended_transfer_function_message.m_opacity_transfer_function[0].m_opacity_variable_max;

    onColorFunctionChanged( ui->colorFunctionCBox->currentIndex() );
    onOpacityFunctionChanged( ui->opacityFunctionCBox->currentIndex() );

    m_is_import_transfer_function_parameter = true;
    onApplyButtonClicked();
}

void TransferFunctionEditor::onColorRangeSyncToolButtonClicked()
{
    m_is_color_range_sync[ui->colorFunctionCBox->currentIndex()] = ui->colorRangeSyncTBtn->isChecked();
}

void TransferFunctionEditor::onOpacityRangeSyncToolButtonClicked()
{
    m_is_opacity_range_sync[ui->opacityFunctionCBox->currentIndex()] = ui->opacityRangeSyncTBtn->isChecked();
}

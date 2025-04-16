#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"
#include "Widgets/MergePanel.h"

#include <QFileDialog>

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent, MergePanel* merge, Connect* connect_panel, ColorMapBarSelector* color_map_bar_selector)
    : QDialog(parent),
    ui(new Ui::TransferFunctionEditor),
    m_merge( merge ),
    m_connect( connect_panel ),
    m_color_map_bar_selector( color_map_bar_selector )
{
    ui->setupUi(this);

    connect( ui->numberOfTransferFunctionSpinBox, &QSpinBox::valueChanged, this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );

    connect( ui->colorSynthesizerLineEdit, &QLineEdit::textEdited, this, &TransferFunctionEditor::onColorSynthesizerEdited );
    connect( ui->colorFunctionComboBox, &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onColorFunctionChanged );
    connect( ui->colorUserDefinedMinMaxRadioButton, &QRadioButton::clicked, this, &TransferFunctionEditor::onColorSelectedRangeClicked );
    connect( ui->colorServerSideMinMaxRadioButton, &QRadioButton::clicked, this, &TransferFunctionEditor::onColorSelectedRangeClicked );
    connect( ui->colorFunctionVariableLineEdit, &QLineEdit::textEdited, this, &TransferFunctionEditor::onColorFunctionVariableEdited );
    connect( ui->colorFunctionListEditorPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onColorFunctionListEditorPushButtonClicked );
    connect( ui->colorUserDefinedMinDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onColorUserDefinedChanged );
    connect( ui->colorUserDefinedMaxDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onColorUserDefinedChanged );
    connect( ui->editColorMapPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onEditColorMapPushButtonClicked );

    connect( ui->opacitySynthesizerLineEdit, &QLineEdit::textEdited, this, &TransferFunctionEditor::onOpacitySynthesizerEdited );
    connect( ui->opacityFunctionComboBox, &QComboBox::currentIndexChanged, this, &TransferFunctionEditor::onOpacityFunctionChanged );
    connect( ui->opacityUserDefinedMinMaxRadioButton, &QRadioButton::clicked, this, &TransferFunctionEditor::onOpacitySelectedRangeClicked );
    connect( ui->opacityServerSideMinMaxRadioButton, &QRadioButton::clicked, this, &TransferFunctionEditor::onOpacitySelectedRangeClicked );
    connect( ui->opacityFunctionVariableLineEdit, &QLineEdit::textEdited, this, &TransferFunctionEditor::onOpacityFunctionVariableEdited );
    connect( ui->opacityFunctionListEditorPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onOpacityFunctionListEditorPushButtonClicked );
    connect( ui->opacityUserDefinedMinDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onOpacityUserDefinedChanged );
    connect( ui->opacityUserDefinedMaxDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &TransferFunctionEditor::onOpacityUserDefinedChanged );
    connect( ui->editOpacityMapPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onEditOpacityMapPushButtonClicked );

    connect( ui->exportPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onExportButtonClicked );
    connect( ui->importPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onImportButtonClicked );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &TransferFunctionEditor::onApplyButtonClicked );

    ui->numberOfTransferFunctionSpinBox->setMinimum( 1 );  //Default
    ui->numberOfTransferFunctionSpinBox->setMaximum( 16 ); //Default

    ui->colorSynthesizerLineEdit->setText( "C1" ); //Default
    ui->opacitySynthesizerLineEdit->setText( "O1" ); //Default
    m_parameter.m_color_synthesizer = "C1"; //Default
    m_parameter.m_opacity_synthesizer = "O1"; //Default
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

//伝達関数の個数が変更された際に実行される。
void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( int numberOfTransferFunction )
{
    m_parameter.m_number_of_transfer_functions = numberOfTransferFunction;

    int currentSize = m_parameter.m_transfer_functions.size();
    char colorName[8];
    char opacityName[8];

    if( currentSize <= numberOfTransferFunction )
    {
        for( int i = currentSize + 1; i <= numberOfTransferFunction; i++ )
        {
            snprintf( colorName, sizeof( colorName ), "C%d", i );
            snprintf( opacityName, sizeof( opacityName ), "O%d", i );
            m_parameter.addTransferFunction( colorName, opacityName, "q1" );
        }
    }
    else
    {
        for( int i = numberOfTransferFunction + 1; i <= currentSize; i++ )
        {
            snprintf( colorName, sizeof( colorName ), "C%d", i );
            snprintf( opacityName, sizeof( opacityName ), "O%d", i );
            m_parameter.removeTransferFunction( colorName, opacityName );
        }
    }

    updateFunctionLists();
}

void TransferFunctionEditor::updateFunctionLists()
{
    QString colorCurrentText = ui->colorFunctionComboBox->currentText();  // 現在の選択項目を保存
    QStringList colorItemList;

    QString opacityCurrentText = ui->opacityFunctionComboBox->currentText();  // 現在の選択項目を保存
    QStringList opacityItemList;

    for(size_t i = 1; i <= m_parameter.m_transfer_functions.size(); i++)
    {
        colorItemList.append(QString("C%1").arg(i));
        opacityItemList.append(QString("O%1").arg(i));
    }

    ui->colorFunctionComboBox->clear();
    ui->colorFunctionComboBox->addItems(colorItemList);

    ui->opacityFunctionComboBox->clear();
    ui->opacityFunctionComboBox->addItems(opacityItemList);

    // 以前の選択項目を再度選択
    int colorIndex = ui->colorFunctionComboBox->findText( colorCurrentText );
    if( colorIndex != -1 )
    {
        ui->colorFunctionComboBox->setCurrentIndex( colorIndex );
    }
    else
    {
        // 見つからなかった場合は最後の項目を選択
        ui->colorFunctionComboBox->setCurrentIndex( ui->colorFunctionComboBox->count() - 1 );
    }

    // 以前の選択項目を再度選択
    int opacityIndex = ui->opacityFunctionComboBox->findText( opacityCurrentText );
    if( opacityIndex != -1 )
    {
        ui->opacityFunctionComboBox->setCurrentIndex( opacityIndex );
    }
    else
    {
        // 見つからなかった場合は最後の項目を選択
        ui->opacityFunctionComboBox->setCurrentIndex( ui->opacityFunctionComboBox->count() - 1 );
    }
}

void TransferFunctionEditor::onColorSynthesizerEdited( const QString &arg1 )
{
    m_parameter.m_color_synthesizer = arg1.toStdString();
}

void TransferFunctionEditor::onColorFunctionChanged( int index )
{
    const algebricTransferFunction* func = m_parameter.getTransferFunction( index );
    if( func != nullptr )
    {
        ui->colorFunctionVariableLineEdit->blockSignals( true );
        ui->colorFunctionVariableLineEdit->setText( QString::fromStdString( func->m_color_variable ) );
        ui->colorFunctionVariableLineEdit->blockSignals( false );

        switch ( func->m_color_stored_select_range ) //stored
        {
        case algebricTransferFunction::UserDefinedRange:
            ui->colorUserDefinedMinMaxRadioButton->blockSignals( true );
            ui->colorUserDefinedMinMaxRadioButton->setChecked( true );
            ui->colorUserDefinedMinMaxRadioButton->blockSignals( false );
            break;
        case algebricTransferFunction::ServerSideRange:
            ui->colorServerSideMinMaxRadioButton->blockSignals( true );
            ui->colorServerSideMinMaxRadioButton->setChecked( true );
            ui->colorServerSideMinMaxRadioButton->blockSignals( false );
            break;
        default:
            break;
        }

        ui->colorUserDefinedMinDoubleSpinBox->blockSignals( true );
        ui->colorUserDefinedMaxDoubleSpinBox->blockSignals( true );
        ui->colorUserDefinedMinDoubleSpinBox->setValue( func->m_color_user_defined_min );
        ui->colorUserDefinedMaxDoubleSpinBox->setValue( func->m_color_user_defined_max );
        ui->colorUserDefinedMinDoubleSpinBox->blockSignals( false );
        ui->colorUserDefinedMaxDoubleSpinBox->blockSignals( false );

        ui->colorServerSideMinLineEdit->blockSignals( true );
        ui->colorServerSideMaxLineEdit->blockSignals( true );
        ui->colorServerSideMinLineEdit->setText( QString::number( func->m_color_server_side_min ) );
        ui->colorServerSideMaxLineEdit->setText( QString::number( func->m_color_server_side_max ) );
        ui->colorServerSideMinLineEdit->blockSignals( false );
        ui->colorServerSideMaxLineEdit->blockSignals( false );

        const kvs::ColorMap::Table table = func->colorMap().table();
        QList<QColor> color_list;

        for (size_t i = 0; i + 2 < table.size(); i += 3)
        {
            QColor color(table[i], table[i + 1], table[i + 2]);
            color_list.append(color);
        }

        ui->colorMapBar->setColors(color_list);
        // ui->colorMapBar->startInitialization();
        ui->colorMapBar->update();

        std::vector<int> ch;
        ch.reserve( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_histogram.bin().size() );
        for( int i = 0; i< m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_histogram.bin().size(); i++ )
        {
            ch.push_back( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_histogram.bin().at(i) );
        }

        ui->colorHistogram->setDatas( ch );

        switch ( func->m_color_confirmed_select_range ) //confirmd
        {
        case algebricTransferFunction::UserDefinedRange:
            // ui->colorHistogram->setRange( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_user_defined_min, m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_user_defined_max );
            if (index < m_connect->getClientMessage()->m_transfer_function.size())
            {
                ui->colorMapMinLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->colorFunctionComboBox->currentIndex()].m_color_variable_min );
                ui->colorMapMaxLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->colorFunctionComboBox->currentIndex()].m_color_variable_max );
            }
            break;
        case algebricTransferFunction::ServerSideRange:
            // ui->colorHistogram->setRange( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_min, m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_max );
            ui->colorMapMinLabel->setNum( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_min );
            ui->colorMapMaxLabel->setNum( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_max );
            break;
        default:
            break;
        }
        ui->colorHistogram->update();
    }
}

void TransferFunctionEditor::onColorFunctionVariableEdited()
{
    std::string colorName = ui->colorFunctionComboBox->currentText().toStdString();
    std::string variable = ui->colorFunctionVariableLineEdit->text().toStdString();

    bool exists = m_parameter.getTransferFunction( colorName ) != nullptr;
    m_parameter.setColorFunctionVariable( colorName, variable );
}

void TransferFunctionEditor::onColorFunctionListEditorPushButtonClicked()
{
    m_variable_editor.initalize( VariableEditor::COLOR, m_parameter );
    if( m_variable_editor.exec() == QDialog::Accepted )
    {
        m_parameter = m_variable_editor.getEditTransferFunctionParameter();
        ui->colorFunctionVariableLineEdit->setText( QString::fromStdString( m_variable_editor.getEditTransferFunctionParameter().m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_variable ) );
    }
}

void TransferFunctionEditor::onColorSelectedRangeClicked()
{
    std::string colorName = ui->colorFunctionComboBox->currentText().toStdString();
    if( ui->colorUserDefinedMinMaxRadioButton->isChecked() )
    {
        m_parameter.getTransferFunction( colorName )->m_color_stored_select_range = algebricTransferFunction::UserDefinedRange; //stored
    }
    else if( ui->colorServerSideMinMaxRadioButton->isChecked() )
    {
        m_parameter.getTransferFunction( colorName )->m_color_stored_select_range = algebricTransferFunction::ServerSideRange; //stored
    }
}

void TransferFunctionEditor::onColorUserDefinedChanged()
{
    std::string colorName = ui->colorFunctionComboBox->currentText().toStdString();
    m_parameter.setColorFunctionRange( colorName, ui->colorUserDefinedMinDoubleSpinBox->value(), ui->colorUserDefinedMaxDoubleSpinBox->value() );
}

void TransferFunctionEditor::onEditColorMapPushButtonClicked()
{
    std::string colorName = ui->colorFunctionComboBox->currentText().toStdString();
    ColorMapEditor colorMapEditor;
    colorMapEditor.adjustSize();
    colorMapEditor.setDefaultColorMap( ui->colorMapBar->getColors() );

    if( colorMapEditor.exec() == QDialog::Accepted )
    {
        // カラーパレットの色を取得してウィジェットに反映
        ui->colorMapBar->setColors( colorMapEditor.getColorMap() );

        // QList<QColor> → kvs::ColorMap に変換
        QVector<QColor> qcolors = ui->colorMapBar->getColors();
        kvs::ValueArray<kvs::UInt8> table;
        table.allocate( qcolors.size() * 3 );

        for ( int i = 0; i < qcolors.size(); ++i )
        {
            const QColor& c = qcolors[i];
            table[3*i]     = static_cast<kvs::UInt8>( c.red() );
            table[3*i + 1] = static_cast<kvs::UInt8>( c.green() );
            table[3*i + 2] = static_cast<kvs::UInt8>( c.blue() );
        }

        kvs::ColorMap cmap( table );
        m_parameter.getTransferFunction( colorName )->setColorMap( cmap );
    }

    // std::string colorName = ui->colorFunctionComboBox->currentText().toStdString();

    // m_color_map_editor.setColorMap( m_parameter.getTransferFunction( colorName )->colorMap() );
    // m_color_map_editor.setInitialColorMap( m_parameter.getTransferFunction( colorName )->colorMap() );
    // m_color_map_editor.clearUndoStack();

    // if( m_color_map_editor.exec() == QDialog::Accepted )
    // {
    //     const kvs::ColorMap cmap = m_color_map_editor.getColorMap();
    //     ui->colorMapBar->setColorMap( cmap );
    //     m_parameter.getTransferFunction( colorName )->setColorMap( cmap );
    // }
}

void TransferFunctionEditor::onOpacitySynthesizerEdited( const QString &arg1 )
{
    m_parameter.m_opacity_synthesizer = arg1.toStdString();
}

void TransferFunctionEditor::onOpacityFunctionChanged( int index )
{
    const algebricTransferFunction* func = m_parameter.getTransferFunction( index );
    if( func != nullptr )
    {
        ui->opacityFunctionVariableLineEdit->blockSignals( true );
        ui->opacityFunctionVariableLineEdit->setText( QString::fromStdString( func->m_opacity_variable ) );
        ui->opacityFunctionVariableLineEdit->blockSignals( false );

        switch ( func->m_opacity_stored_select_range ) //stored
        {
        case algebricTransferFunction::UserDefinedRange:
            ui->opacityUserDefinedMinMaxRadioButton->blockSignals( true );
            ui->opacityUserDefinedMinMaxRadioButton->setChecked( true );
            ui->opacityUserDefinedMinMaxRadioButton->blockSignals( false );
            break;
        case algebricTransferFunction::ServerSideRange:
            ui->opacityServerSideMinMaxRadioButton->blockSignals( true );
            ui->opacityServerSideMinMaxRadioButton->setChecked( true );
            ui->opacityServerSideMinMaxRadioButton->blockSignals( false );
            break;
        default:
            break;
        }

        ui->opacityUserDefinedMinDoubleSpinBox->blockSignals( true );
        ui->opacityUserDefinedMaxDoubleSpinBox->blockSignals( true );
        ui->opacityUserDefinedMinDoubleSpinBox->setValue( func->m_opacity_user_defined_min );
        ui->opacityUserDefinedMaxDoubleSpinBox->setValue( func->m_opacity_user_defined_max );
        ui->opacityUserDefinedMinDoubleSpinBox->blockSignals( false );
        ui->opacityUserDefinedMaxDoubleSpinBox->blockSignals( false );

        ui->opacityServerSideMinLineEdit->blockSignals( true );
        ui->opacityServerSideMaxLineEdit->blockSignals( true );
        ui->opacityServerSideMinLineEdit->setText( QString::number( func->m_opacity_server_side_min ) );
        ui->opacityServerSideMaxLineEdit->setText( QString::number( func->m_opacity_server_side_max ) );
        ui->opacityServerSideMinLineEdit->blockSignals( false );
        ui->opacityServerSideMaxLineEdit->blockSignals( false );

        const kvs::OpacityMap& omap = func->opacityMap();
        const kvs::ValueArray<float>& values = omap.table(); // もしくは omap.getTable() に読み替え

        QVector<float> opacities;
        opacities.reserve(values.size());
        for (size_t i = 0; i < values.size(); ++i)
        {
            opacities.append(values[i]);
        }

        ui->opacityMapBar->setOpacities(opacities);
        ui->opacityMapBar->update();

        // ui->opacityHistogram->setTable( func->m_opacity_histogram );

        std::vector<int> oh;
        oh.reserve( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram.bin().size() );
        for( int i = 0; i< m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram.bin().size(); i++ )
        {
            oh.push_back( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram.bin().at(i) );
        }

        ui->opacityHistogram->setDatas( oh );


        switch ( func->m_opacity_confirmed_select_range ) //confirmd
        {
        case algebricTransferFunction::UserDefinedRange:
            // ui->opacityHistogram->setRange( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_user_defined_min, m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_user_defined_max );
            if (index < m_connect->getClientMessage()->m_transfer_function.size())
            {
                ui->opacityMapMinLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->opacityFunctionComboBox->currentIndex()].m_opacity_variable_min );
                ui->opacityMapMaxLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->opacityFunctionComboBox->currentIndex()].m_opacity_variable_max );
            }
            break;
        case algebricTransferFunction::ServerSideRange:
            // ui->opacityHistogram->setRange( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_min, m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_max );
            ui->opacityMapMinLabel->setNum( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_min );
            ui->opacityMapMaxLabel->setNum( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_max );
            break;
        default:
            break;
        }
        ui->opacityHistogram->update();
    }
}

void TransferFunctionEditor::onOpacityFunctionVariableEdited()
{
    std::string opacityName = ui->opacityFunctionComboBox->currentText().toStdString();
    std::string variable = ui->opacityFunctionVariableLineEdit->text().toStdString();

    bool exists = m_parameter.getTransferFunction( opacityName ) != nullptr;
    m_parameter.setOpacityFunctionVariable( opacityName, variable );
}

void TransferFunctionEditor::onOpacityFunctionListEditorPushButtonClicked()
{
    m_variable_editor.initalize( VariableEditor::OPACITY, m_parameter );
    if( m_variable_editor.exec() == QDialog::Accepted )
    {
        m_parameter = m_variable_editor.getEditTransferFunctionParameter();
        ui->opacityFunctionVariableLineEdit->setText( QString::fromStdString( m_variable_editor.getEditTransferFunctionParameter().m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_variable ) );
    }
}

void TransferFunctionEditor::onOpacitySelectedRangeClicked()
{
    std::string opacityName = ui->opacityFunctionComboBox->currentText().toStdString();
    if( ui->opacityUserDefinedMinMaxRadioButton->isChecked() )
    {
        m_parameter.getTransferFunction( opacityName )->m_opacity_stored_select_range = algebricTransferFunction::UserDefinedRange; //stored
    }
    else if( ui->opacityServerSideMinMaxRadioButton->isChecked() )
    {
        m_parameter.getTransferFunction( opacityName )->m_opacity_stored_select_range = algebricTransferFunction::ServerSideRange; //stored
    }
}

void TransferFunctionEditor::onOpacityUserDefinedChanged()
{
    std::string opacityName = ui->opacityFunctionComboBox->currentText().toStdString();
    m_parameter.setOpacityFunctionRange( opacityName, ui->opacityUserDefinedMinDoubleSpinBox->value(), ui->opacityUserDefinedMaxDoubleSpinBox->value() );
}

void TransferFunctionEditor::onEditOpacityMapPushButtonClicked()
{
    std::string opacityName = ui->opacityFunctionComboBox->currentText().toStdString();
    OpacityMapEditor opacityMapEditor;
    opacityMapEditor.adjustSize();
    opacityMapEditor.setDefaultOpacityMap( ui->opacityMapBar->getOpacities() );

    if( opacityMapEditor.exec() == QDialog::Accepted )
    {
        // UI側に反映
        ui->opacityMapBar->setOpacities( opacityMapEditor.getOpacityMap() );

        // QVector<float> → kvs::OpacityMap に変換
        QVector<float> opacities = ui->opacityMapBar->getOpacities();
        kvs::ValueArray<float> values( opacities.size() );

        for (int i = 0; i < opacities.size(); ++i)
        {
            values[i] = opacities[i];
        }

        kvs::OpacityMap omap( values );
        m_parameter.getTransferFunction( opacityName )->setOpacityMap( omap );
    }
}

void TransferFunctionEditor::onExportButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr("Export Current Settings to Parameter File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );
    if( fileName.right(4) != ".tfe" && fileName.right(4) != ".TFE" && fileName.right(3) != ".tf" && fileName.right(3) != ".TF")
    {
        fileName += ".tfe";
    }
#ifdef Q_OS_WIN
    this->exportTransferFunctionFile( fileName.toLocal8Bit().constData(), false );
#else
    this->exportTransferFunctionFile( fileName.toStdString(), false );
#endif

}

void TransferFunctionEditor::onImportButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr("Import Transfer Function File"), ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );

#ifdef Q_OS_WIN
    importTransferFunctionFromFile( fileName.toLocal8Bit().constData() );
#else
    importTransferFunctionFromFile( fileName.toStdString() );
#endif
}

void TransferFunctionEditor::onApplyButtonClicked()
{
    apply();
}

void TransferFunctionEditor::exportTransferFunctionFile( const std::string& transferFunctionFile, const bool append )
{
    QFile file( QString::fromStdString( transferFunctionFile ) );
    QIODevice::OpenMode mode = append ? QIODevice::Append : QIODevice::WriteOnly;

    if (!file.open(mode | QIODevice::Text))
    {
        qWarning() << "Error: open file" << QString::fromStdString( transferFunctionFile );
        return;
    }

    QTextStream out(&file);
    out << "TF_RESOLUTION=" << 256 << "\n";
    out << "TF_NUMBER=" << m_parameter.m_number_of_transfer_functions << "\n";
    out << "TF_SYNTH_C=" << QString::fromStdString( m_parameter.m_color_synthesizer ) << "\n";
    out << "TF_SYNTH_O=" << QString::fromStdString( m_parameter.m_opacity_synthesizer ) << "\n";

    for( int i = 0; i < m_parameter.m_number_of_transfer_functions; i++ )
    {
        QString tagBase = QString( "TF_NAME%1_" ).arg( i + 1 );

        out << tagBase << "C=" << QString::fromStdString( m_parameter.m_transfer_functions[i].m_color_function_name ) << "\n";
        out << tagBase << "VAR_C=" << QString::fromStdString( m_parameter.m_transfer_functions[i].m_color_variable ) << "\n";
        out << tagBase << "MIN_C=" << m_parameter.m_transfer_functions[i].m_color_user_defined_min  << "\n";
        out << tagBase << "MAX_C=" << m_parameter.m_transfer_functions[i].m_color_user_defined_max  << "\n";

        kvs::ColorMap::Table colorTable = m_parameter.m_transfer_functions[i].colorMap().table();

        out << tagBase << "TABLE_C=";
        for( size_t i = 0; i < colorTable.size(); i++ )
        {
            out << static_cast<int>( colorTable.at(i) );
            if( i < colorTable.size() - 1 )
            {
                out << ",";
            }
        }
        out << "\n";
    }

    for( int i = 0; i < m_parameter.m_number_of_transfer_functions; i++ )
    {
        QString tagBase = QString( "TF_NAME%1_" ).arg( i + 1 );

        out << tagBase << "O=" << QString::fromStdString( m_parameter.m_transfer_functions[i].m_opacity_function_name ) << "\n";
        out << tagBase << "VAR_O=" << QString::fromStdString( m_parameter.m_transfer_functions[i].m_opacity_variable ) << "\n";
        out << tagBase << "MIN_O=" << m_parameter.m_transfer_functions[i].m_opacity_user_defined_min  << "\n";
        out << tagBase << "MAX_O=" << m_parameter.m_transfer_functions[i].m_opacity_user_defined_max  << "\n";

        kvs::OpacityMap::Table opacityTable = m_parameter.m_transfer_functions[i].opacityMap().table();

        out << tagBase << "TABLE_O=";
        for( size_t i = 0; i < opacityTable.size(); i++ )
        {
            std::cout << opacityTable.at(i) << std::endl;
            out << opacityTable.at(i) ;
            if( i < opacityTable.size() - 1 )
            {
                out << ",";
            }
        }
        out << "\n";
    }

    file.close();
    qWarning() << "TransferFunction parameters are exported to" << QString::fromStdString( transferFunctionFile );
}

bool TransferFunctionEditor::importTransferFunctionFromFile( const std::string& transferFunctionFile )
{
    QFile file( QString::fromStdString( transferFunctionFile ) );

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Error: open file" << QString::fromStdString( transferFunctionFile );
        return false;
    }

    QTextStream in(&file);
    QString line;

    while (!in.atEnd())
    {
        line = in.readLine();

        if (line.startsWith("TF_RESOLUTION="))
        {
            int resolution = line.mid(14).toInt();
            // 解像度の読み込み。必要ならm_parameterに格納
            qDebug() << resolution;
        }
        else if (line.startsWith("TF_NUMBER="))
        {
            ui->numberOfTransferFunctionSpinBox->setValue( line.mid(10).toInt() );
        }
        else if (line.startsWith("TF_SYNTH_C="))
        {
            ui->colorSynthesizerLineEdit->setText( line.mid(11) );
            m_parameter.m_color_synthesizer = line.mid(11).toStdString();
        }
        else if (line.startsWith("TF_SYNTH_O="))
        {
            ui->opacitySynthesizerLineEdit->setText( line.mid(11) );
            m_parameter.m_opacity_synthesizer = line.mid(11).toStdString();
        }
        else
        {
            for( int i = 0; i < m_parameter.m_number_of_transfer_functions; i++ )
            {
                QString tagBase = QString( "TF_NAME%1_" ).arg( i + 1 );

                if (line.startsWith(tagBase + "C="))
                {
                    m_parameter.m_transfer_functions[i].m_color_function_name = line.mid(tagBase.length() + 2).toStdString();
                }
                else if (line.startsWith(tagBase + "VAR_C="))
                {
                    m_parameter.m_transfer_functions[i].m_color_variable = line.mid(tagBase.length() + 6).toStdString();
                }
                else if (line.startsWith(tagBase + "MIN_C="))
                {
                    m_parameter.m_transfer_functions[i].m_color_user_defined_min = line.mid(tagBase.length() + 6).toDouble();
                }
                else if (line.startsWith(tagBase + "MAX_C="))
                {
                    m_parameter.m_transfer_functions[i].m_color_user_defined_max = line.mid(tagBase.length() + 6).toDouble();
                }
                else if (line.startsWith(tagBase + "TABLE_C="))
                {
                    QString tableData = line.mid( tagBase.length() + 8 );
                    if( tableData.endsWith(",") )
                    {
                        tableData.chop(1);
                    }
                    QStringList colorValues = tableData.split(",");
                    int resolution = colorValues.size() / 3 ; // RGBの3つの値で1つのポイントを表す。
                    kvs::ColorMap::Table colorTable( resolution * 3 );
                    for( size_t i = 0; i < resolution; i++ )
                    {
                        for( size_t c = 0; c < 3; c++ )
                        {
                            colorTable[i * 3 + c] = static_cast<unsigned char>( colorValues[i * 3 + c].toInt() );
                        }
                    }
                    kvs::ColorMap colorMap( colorTable );
                    m_parameter.m_transfer_functions[i].setColorMap( colorMap );
                }
                else if (line.startsWith(tagBase + "O="))
                {
                    m_parameter.m_transfer_functions[i].m_opacity_function_name = line.mid(tagBase.length() + 2).toStdString();
                }
                else if (line.startsWith(tagBase + "VAR_O="))
                {
                    m_parameter.m_transfer_functions[i].m_opacity_variable = line.mid(tagBase.length() + 6).toStdString();
                }
                else if (line.startsWith(tagBase + "MIN_O="))
                {
                    m_parameter.m_transfer_functions[i].m_opacity_user_defined_min = line.mid(tagBase.length() + 6).toDouble();
                }
                else if (line.startsWith(tagBase + "MAX_O="))
                {
                    m_parameter.m_transfer_functions[i].m_opacity_user_defined_max = line.mid(tagBase.length() + 6).toDouble();
                }
                else if (line.startsWith(tagBase + "TABLE_O="))
                {
                    QString tableData = line.mid( tagBase.length() + 8 );
                    if( tableData.endsWith(",") )
                    {
                        tableData.chop(1);
                    }
                    QStringList opacityValues = tableData.split(",");

                    kvs::OpacityMap::Table opacityTable( opacityValues.size() );
                    for( size_t i = 0; i < opacityValues.size(); i++ )
                    {
                        opacityTable[i] = opacityValues[i].toFloat();
                    }
                    kvs::OpacityMap opacity_map( opacityTable );
                    m_parameter.m_transfer_functions[i].setOpacityMap( opacity_map );
                }
                m_parameter.m_transfer_functions[i].m_color_stored_select_range = algebricTransferFunction::UserDefinedRange; //stored
                m_parameter.m_transfer_functions[i].m_opacity_stored_select_range = algebricTransferFunction::UserDefinedRange; //stored
            }
        }
    }

    //読み込みが終わったらC1 O1を選択した状態にする。-> 読み込んでない場合は事前に入力された値が再表示されるだけ。
    {
        ui->colorFunctionComboBox->blockSignals( true );
        ui->colorFunctionVariableLineEdit->blockSignals( true );
        ui->colorUserDefinedMinMaxRadioButton->blockSignals( true );
        ui->colorUserDefinedMinDoubleSpinBox->blockSignals( true );
        ui->colorUserDefinedMaxDoubleSpinBox->blockSignals( true );
        ui->colorMapBar->blockSignals( true );

        ui->colorFunctionComboBox->setCurrentIndex( 0 );
        ui->colorFunctionVariableLineEdit->setText( QString::fromStdString( m_parameter.m_transfer_functions[0].m_color_variable ) );
        ui->colorUserDefinedMinMaxRadioButton->setChecked( true );
        ui->colorUserDefinedMinDoubleSpinBox->setValue( m_parameter.m_transfer_functions[0].m_color_user_defined_min );
        ui->colorUserDefinedMaxDoubleSpinBox->setValue( m_parameter.m_transfer_functions[0].m_color_user_defined_max );

        const kvs::ValueArray<kvs::UInt8>& table = m_parameter.m_transfer_functions[0].colorMap().table();
        QVector<QColor> colors;
        for ( size_t i = 0; i < table.size(); i += 3 )
        {
            if ( i + 2 >= table.size() ) break; // 範囲チェック
            QColor color( table[i], table[i + 1], table[i + 2] );
            colors.push_back( color );
        }
        ui->colorMapBar->setColors( colors );

        ui->colorFunctionComboBox->blockSignals( false );
        ui->colorFunctionVariableLineEdit->blockSignals( false );
        ui->colorUserDefinedMinMaxRadioButton->blockSignals( false );
        ui->colorUserDefinedMinDoubleSpinBox->blockSignals( false );
        ui->colorUserDefinedMaxDoubleSpinBox->blockSignals( false );
        ui->colorMapBar->blockSignals( false );
    }

    {
        ui->opacityFunctionComboBox->blockSignals( true );
        ui->opacityFunctionVariableLineEdit->blockSignals( true );
        ui->opacityUserDefinedMinDoubleSpinBox->blockSignals( true );
        ui->opacityUserDefinedMaxDoubleSpinBox->blockSignals( true );
        ui->opacityMapBar->blockSignals( true );

        ui->opacityFunctionComboBox->setCurrentIndex( 0 );
        ui->opacityFunctionVariableLineEdit->setText( QString::fromStdString( m_parameter.m_transfer_functions[0].m_opacity_variable ) );
        ui->opacityUserDefinedMinMaxRadioButton->setChecked( true );
        ui->opacityUserDefinedMinDoubleSpinBox->setValue( m_parameter.m_transfer_functions[0].m_opacity_user_defined_min );
        ui->opacityUserDefinedMaxDoubleSpinBox->setValue( m_parameter.m_transfer_functions[0].m_opacity_user_defined_max );
        const kvs::OpacityMap& omap = m_parameter.m_transfer_functions[0].opacityMap();
        const kvs::ValueArray<float>& values = omap.table(); // または omap.getTable()

        QVector<float> opacities;
        opacities.reserve(values.size());
        for (size_t i = 0; i < values.size(); ++i)
        {
            opacities.append(values[i]);
        }

        ui->opacityMapBar->setOpacities(opacities);

        ui->opacityFunctionComboBox->blockSignals( false );
        ui->opacityFunctionVariableLineEdit->blockSignals( false );
        ui->opacityUserDefinedMinDoubleSpinBox->blockSignals( false );
        ui->opacityUserDefinedMaxDoubleSpinBox->blockSignals( false );
        ui->opacityMapBar->blockSignals( true );
    }

    apply();
    return true;
}

void TransferFunctionEditor::importTransferFunctionFromServer()
{
    int resolution = 256;
    ui->numberOfTransferFunctionSpinBox->setValue( m_connect->getServerMessage()->m_transfer_function.size() );

    m_parameter.m_color_synthesizer = m_connect->getServerMessage()->m_color_transfer_function_synthesis;
    m_parameter.m_opacity_synthesizer = m_connect->getServerMessage()->m_opacity_transfer_function_synthesis;

    for( size_t i = 0; i < m_connect->getServerMessage()->m_transfer_function.size(); i++ )
    {
        char tag_c[16] = {0x00};
        sprintf(tag_c, "t%d_var_c", i + 1);
        m_parameter.m_transfer_functions[i].m_color_variable = m_connect->getServerMessage()->m_transfer_function[i].m_color_variable;
        m_parameter.m_transfer_functions[i].m_color_stored_select_range = algebricTransferFunction::ServerSideRange; //stored
        m_parameter.m_transfer_functions[i].m_color_server_side_min = m_connect->getServerMessage()->m_server_side_variable_range.min( tag_c );
        m_parameter.m_transfer_functions[i].m_color_server_side_max = m_connect->getServerMessage()->m_server_side_variable_range.max( tag_c );
        m_parameter.m_transfer_functions[i].setColorMap( m_connect->getServerMessage()->m_transfer_function[i].colorMap() );
        //m_parameter.m_transfer_functions[i].m_color_histogram = *m_connect->getReceivedMessage()->findColorFrequencyTable( m_parameter.m_transfer_functions[i].m_color_function_name );
        char tag_o[16] = {0x00};
        sprintf(tag_o, "t%d_var_o", i + 1);
        m_parameter.m_transfer_functions[i].m_opacity_variable = m_connect->getServerMessage()->m_transfer_function[i].m_opacity_variable;
        m_parameter.m_transfer_functions[i].m_opacity_stored_select_range = algebricTransferFunction::ServerSideRange; //stored
        m_parameter.m_transfer_functions[i].m_opacity_server_side_min = m_connect->getServerMessage()->m_server_side_variable_range.min( tag_o );
        m_parameter.m_transfer_functions[i].m_opacity_server_side_max = m_connect->getServerMessage()->m_server_side_variable_range.max( tag_o );
        m_parameter.m_transfer_functions[i].setOpacityMap( m_connect->getServerMessage()->m_transfer_function[i].opacityMap() );
        //m_parameter.m_transfer_functions[i].m_opacity_histogram = *m_connect->getReceivedMessage()->findOpacityFrequencyTable( m_parameter.m_transfer_functions[i].m_opacity_function_name );
    }

    for( size_t i = 0; i < m_connect->getServerMessage()->m_transfer_function_count; i++ )
    {
        m_parameter.m_transfer_functions[i].m_color_histogram = *m_connect->getReceivedMessage()->findColorFrequencyTable( m_parameter.m_transfer_functions[i].m_color_function_name );
        m_parameter.m_transfer_functions[i].m_opacity_histogram = *m_connect->getReceivedMessage()->findOpacityFrequencyTable( m_parameter.m_transfer_functions[i].m_opacity_function_name );
    }

    {
        ui->colorFunctionComboBox->blockSignals( true );
        ui->colorFunctionVariableLineEdit->blockSignals( true );
        ui->colorServerSideMinMaxRadioButton->blockSignals( true );
        ui->colorServerSideMinLineEdit->blockSignals( true );
        ui->colorServerSideMaxLineEdit->blockSignals( true );
        ui->colorMapBar->blockSignals( true );

        ui->colorFunctionComboBox->setCurrentIndex( 0 );
        ui->colorFunctionVariableLineEdit->setText( QString::fromStdString( m_parameter.m_transfer_functions[0].m_color_variable ) );
        ui->colorServerSideMinMaxRadioButton->setChecked( true );
        ui->colorServerSideMinLineEdit->setText( QString::number( m_parameter.m_transfer_functions[0].m_color_server_side_min ) );
        ui->colorServerSideMaxLineEdit->setText( QString::number( m_parameter.m_transfer_functions[0].m_color_server_side_max ) );
        ui->colorMapMinLabel->setNum( m_parameter.m_transfer_functions[0].m_color_server_side_min );
        ui->colorMapMaxLabel->setNum( m_parameter.m_transfer_functions[0].m_color_server_side_max );

        const kvs::ValueArray<kvs::UInt8>& table = m_parameter.m_transfer_functions[0].colorMap().table();
        QVector<QColor> colors;
        for ( size_t i = 0; i < table.size(); i += 3 )
        {
            if ( i + 2 >= table.size() ) break; // 範囲チェック
            QColor color( table[i], table[i + 1], table[i + 2] );
            colors.push_back( color );
        }
        ui->colorMapBar->setColors( colors );

        std::vector<int> ch;
        ch.reserve( m_parameter.m_transfer_functions[0].m_color_histogram.bin().size() );
        for( int i = 0; i< m_parameter.m_transfer_functions[0].m_color_histogram.bin().size(); i++ )
        {
            ch.push_back( m_parameter.m_transfer_functions[0].m_color_histogram.bin().at(i) );
        }

        ui->colorHistogram->setDatas( ch );

        ui->colorFunctionComboBox->blockSignals( false );
        ui->colorFunctionVariableLineEdit->blockSignals( false );
        ui->colorServerSideMinMaxRadioButton->blockSignals( false );
        ui->colorServerSideMinLineEdit->blockSignals( false );
        ui->colorServerSideMaxLineEdit->blockSignals( false );
        ui->colorMapBar->blockSignals( false );
    }

    {
        ui->opacityFunctionComboBox->blockSignals( true );
        ui->opacityFunctionVariableLineEdit->blockSignals( true );
        ui->opacityServerSideMinMaxRadioButton->blockSignals( true );
        ui->opacityServerSideMinLineEdit->blockSignals( true );
        ui->opacityServerSideMaxLineEdit->blockSignals( true );
        ui->opacityMapBar->blockSignals( true );

        ui->opacityFunctionComboBox->setCurrentIndex( 0 );
        ui->opacityFunctionVariableLineEdit->setText( QString::fromStdString( m_parameter.m_transfer_functions[0].m_opacity_variable ) );
        ui->opacityServerSideMinMaxRadioButton->setChecked( true );
        ui->opacityServerSideMinLineEdit->setText( QString::number( m_parameter.m_transfer_functions[0].m_opacity_server_side_min ) );
        ui->opacityServerSideMaxLineEdit->setText( QString::number( m_parameter.m_transfer_functions[0].m_opacity_server_side_max ) );
        ui->opacityMapMinLabel->setNum( m_parameter.m_transfer_functions[0].m_opacity_server_side_min );
        ui->opacityMapMaxLabel->setNum( m_parameter.m_transfer_functions[0].m_opacity_server_side_max );

        const kvs::OpacityMap& omap = m_parameter.m_transfer_functions[0].opacityMap();
        const kvs::ValueArray<float>& values = omap.table(); // または omap.getTable()

        QVector<float> opacities;
        opacities.reserve(values.size());
        for (size_t i = 0; i < values.size(); ++i)
        {
            opacities.append(values[i]);
        }

        ui->opacityMapBar->setOpacities(opacities);

        // ui->opacityHistogram->setTable( m_parameter.m_transfer_functions[0].m_opacity_histogram );
        std::vector<int> oh;
        oh.reserve( m_parameter.m_transfer_functions[0].m_opacity_histogram.bin().size() );
        for( int i = 0; i< m_parameter.m_transfer_functions[0].m_opacity_histogram.bin().size(); i++ )
        {
            oh.push_back( m_parameter.m_transfer_functions[0].m_opacity_histogram.bin().at(i) );
        }

        ui->opacityHistogram->setDatas( oh );

        // ui->opacityHistogram->setRange( m_parameter.m_transfer_functions[0].m_opacity_server_side_min, m_parameter.m_transfer_functions[0].m_opacity_server_side_max );

        ui->opacityFunctionComboBox->blockSignals( false );
        ui->opacityFunctionVariableLineEdit->blockSignals( false );
        ui->opacityServerSideMinMaxRadioButton->blockSignals( false );
        ui->opacityServerSideMinLineEdit->blockSignals( false );
        ui->opacityServerSideMaxLineEdit->blockSignals( false );
        ui->opacityMapBar->blockSignals( false );
    }
    apply();
}

void TransferFunctionEditor::apply()
{
    m_connect->getClientMessage()->m_transfer_function.clear();
    m_connect->getClientMessage()->m_volume_equation.clear();

    m_connect->getClientMessage()->m_color_transfer_function_synthesis = m_parameter.m_color_synthesizer;
    m_connect->getClientMessage()->m_opacity_transfer_function_synthesis = m_parameter.m_opacity_synthesizer;

    for( int i = 0; i < m_parameter.m_number_of_transfer_functions; i++ )
    {
        // 警告 クライアントメッセージがNamedTransferFunctionParameterとかいうのを使っているので詰め直す。 可能であれば修正した方が良い。
        NamedTransferFunctionParameter etf;
        jpv::ParticleTransferClientMessage::VolumeEquation volumeEquationColor, volumeEquationOpacity;

        std::stringstream ss;
        ss << "_F" << i;

        etf.m_color_variable   = ss.str() + "_VAR_C";
        etf.setColorMap( m_parameter.getTransferFunction(i)->colorMap() );
        m_parameter.getTransferFunction(i)->m_color_confirmed_select_range = m_parameter.getTransferFunction(i)->m_color_stored_select_range;
        if( m_parameter.getTransferFunction(i)->m_color_confirmed_select_range == algebricTransferFunction::UserDefinedRange ) //confirmd
        {
            etf.m_color_variable_min = m_parameter.getTransferFunction(i)->m_color_user_defined_min;
            etf.m_color_variable_max = m_parameter.getTransferFunction(i)->m_color_user_defined_max;
        }
        else if( m_parameter.getTransferFunction(i)->m_color_confirmed_select_range == algebricTransferFunction::ServerSideRange ) //confirmd
        {
            etf.m_color_variable_min = m_parameter.getTransferFunction(i)->m_color_server_side_min;
            etf.m_color_variable_max = m_parameter.getTransferFunction(i)->m_color_server_side_max;
        }
        volumeEquationColor.m_name = etf.m_color_variable;
        volumeEquationColor.m_equation = m_parameter.m_transfer_functions[i].m_color_variable;

        etf.m_opacity_variable   = ss.str() + "_VAR_O";
        etf.setOpacityMap( m_parameter.getTransferFunction(i)->opacityMap() );
        m_parameter.getTransferFunction(i)->m_opacity_confirmed_select_range = m_parameter.getTransferFunction(i)->m_opacity_stored_select_range;
        if( m_parameter.getTransferFunction(i)->m_opacity_confirmed_select_range == algebricTransferFunction::UserDefinedRange ) //confirmd
        {
            etf.m_opacity_variable_min = m_parameter.getTransferFunction(i)->m_opacity_user_defined_min;
            etf.m_opacity_variable_max = m_parameter.getTransferFunction(i)->m_opacity_user_defined_max;
        }
        else if( m_parameter.getTransferFunction(i)->m_opacity_confirmed_select_range == algebricTransferFunction::ServerSideRange ) //confirmd
        {
            etf.m_opacity_variable_min = m_parameter.getTransferFunction(i)->m_opacity_server_side_min;
            etf.m_opacity_variable_max = m_parameter.getTransferFunction(i)->m_opacity_server_side_max;
        }
        volumeEquationOpacity.m_name = etf.m_opacity_variable;
        volumeEquationOpacity.m_equation = m_parameter.m_transfer_functions[i].m_opacity_variable;

        m_connect->getClientMessage()->m_transfer_function.push_back( etf );
        m_connect->getClientMessage()->m_volume_equation.push_back( volumeEquationColor );
        m_connect->getClientMessage()->m_volume_equation.push_back( volumeEquationOpacity );
    }
    m_merge->setIsParticleGenerationNeeded( true );
    m_color_map_bar_selector->updateFunctionLists();
    onColorFunctionChanged( ui->colorFunctionComboBox->currentIndex() );
    onOpacityFunctionChanged( ui->opacityFunctionComboBox->currentIndex() );
}

void TransferFunctionEditor::updateRangeView()
{
    for( size_t i = 0; i < m_parameter.m_transfer_functions.size(); i++ )
    {
        m_parameter.m_transfer_functions[i].m_color_generated_server_side_min = m_parameter.m_transfer_functions[i].m_color_server_side_min;
        m_parameter.m_transfer_functions[i].m_color_generated_server_side_max = m_parameter.m_transfer_functions[i].m_color_server_side_max;
        char tag_c[16] = {0x00};
        sprintf(tag_c, "t%d_var_c", i + 1);
        m_parameter.m_transfer_functions[i].m_color_server_side_min = m_connect->getServerMessage()->m_server_side_variable_range.min( tag_c );
        m_parameter.m_transfer_functions[i].m_color_server_side_max = m_connect->getServerMessage()->m_server_side_variable_range.max( tag_c );
        const auto* color_histogram = m_connect->getReceivedMessage()->findColorFrequencyTable( m_parameter.m_transfer_functions[i].m_color_function_name );
        if (color_histogram != nullptr) {
            m_parameter.m_transfer_functions[i].m_color_histogram = *color_histogram;
        }
        m_parameter.m_transfer_functions[i].m_opacity_generated_server_side_min = m_parameter.m_transfer_functions[i].m_opacity_server_side_min;
        m_parameter.m_transfer_functions[i].m_opacity_generated_server_side_max = m_parameter.m_transfer_functions[i].m_opacity_server_side_max;
        char tag_o[16] = {0x00};
        sprintf(tag_o, "t%d_var_o", i + 1);
        m_parameter.m_transfer_functions[i].m_opacity_server_side_min = m_connect->getServerMessage()->m_server_side_variable_range.min( tag_o );
        m_parameter.m_transfer_functions[i].m_opacity_server_side_max = m_connect->getServerMessage()->m_server_side_variable_range.max( tag_o );
        const auto* opacity_histogram = m_connect->getReceivedMessage()->findOpacityFrequencyTable( m_parameter.m_transfer_functions[i].m_opacity_function_name );
        if (opacity_histogram != nullptr) {
            m_parameter.m_transfer_functions[i].m_opacity_histogram = *opacity_histogram;
        }
    }

    QMetaObject::invokeMethod(this, [this]() {
        {
            ui->colorFunctionComboBox->blockSignals( true );
            ui->colorFunctionVariableLineEdit->blockSignals( true );
            ui->colorServerSideMinLineEdit->blockSignals( true );
            ui->colorServerSideMaxLineEdit->blockSignals( true );
            ui->colorMapBar->blockSignals( true );

            ui->colorFunctionComboBox->setCurrentIndex( ui->colorFunctionComboBox->currentIndex() );
            ui->colorFunctionVariableLineEdit->setText( QString::fromStdString( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_variable ) );
            ui->colorServerSideMinLineEdit->setText( QString::number( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_min ) );
            ui->colorServerSideMaxLineEdit->setText( QString::number( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_max ) );
            // ui->colorMapBar->setColorMap( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].colorMap() );
            const kvs::ValueArray<kvs::UInt8>& table = m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].colorMap().table();
            QVector<QColor> colors;
            for ( size_t i = 0; i < table.size(); i += 3 )
            {
                if ( i + 2 >= table.size() ) break; // 範囲チェック
                QColor color( table[i], table[i + 1], table[i + 2] );
                colors.push_back( color );
            }
            ui->colorMapBar->setColors( colors );

            std::vector<int> ch;
            ch.reserve( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_histogram.bin().size() );
            for( int i = 0; i< m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_histogram.bin().size(); i++ )
            {
                ch.push_back( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_histogram.bin().at(i) );
            }

            ui->colorHistogram->setDatas( ch );

            switch ( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_confirmed_select_range ) //confirmd
            {
            case algebricTransferFunction::UserDefinedRange:
                // ui->colorHistogram->setRange( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_user_defined_min, m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_user_defined_max );
                ui->colorHistogram->update();
                ui->colorMapMinLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->colorFunctionComboBox->currentIndex()].m_color_variable_min );
                ui->colorMapMaxLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->colorFunctionComboBox->currentIndex()].m_color_variable_max );
                break;
            case algebricTransferFunction::ServerSideRange:
                // ui->colorHistogram->setRange( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_min, m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_max );
                ui->colorHistogram->update();
                ui->colorMapMinLabel->setNum( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_min );
                ui->colorMapMaxLabel->setNum( m_parameter.m_transfer_functions[ui->colorFunctionComboBox->currentIndex()].m_color_server_side_max );
                break;
            default:
                break;
            }

            ui->colorFunctionComboBox->blockSignals( false );
            ui->colorFunctionVariableLineEdit->blockSignals( false );
            ui->colorServerSideMinLineEdit->blockSignals( false );
            ui->colorServerSideMaxLineEdit->blockSignals( false );
            ui->colorMapBar->blockSignals( false );
        }

        {
            ui->opacityFunctionComboBox->blockSignals( true );
            ui->opacityFunctionVariableLineEdit->blockSignals( true );
            ui->opacityServerSideMinLineEdit->blockSignals( true );
            ui->opacityServerSideMaxLineEdit->blockSignals( true );
            ui->opacityMapBar->blockSignals( true );

            ui->opacityFunctionComboBox->setCurrentIndex( ui->opacityFunctionComboBox->currentIndex() );
            ui->opacityFunctionVariableLineEdit->setText( QString::fromStdString( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_variable ) );
            ui->opacityServerSideMinLineEdit->setText( QString::number( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_min ) );
            ui->opacityServerSideMaxLineEdit->setText( QString::number( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_max ) );

            const kvs::OpacityMap& omap = m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].opacityMap();
            const kvs::ValueArray<float>& values = omap.table(); // または omap.getTable()

            QVector<float> opacities;
            opacities.reserve(values.size());
            for (size_t i = 0; i < values.size(); ++i)
            {
                opacities.append(values[i]);
            }

            ui->opacityMapBar->setOpacities(opacities);

            // ui->opacityHistogram->setTable( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram );
            std::vector<int> oh;
            oh.reserve( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram.bin().size() );
            for( int i = 0; i< m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram.bin().size(); i++ )
            {
                oh.push_back( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_histogram.bin().at(i) );
            }

            ui->opacityHistogram->setDatas( oh );

            switch ( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_confirmed_select_range ) //confirmd
            {
            case algebricTransferFunction::UserDefinedRange:
                // ui->opacityHistogram->setRange( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_user_defined_min, m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_user_defined_max );
                ui->opacityHistogram->update();
                ui->opacityMapMinLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->opacityFunctionComboBox->currentIndex()].m_opacity_variable_min );
                ui->opacityMapMaxLabel->setNum( m_connect->getClientMessage()->m_transfer_function[ui->opacityFunctionComboBox->currentIndex()].m_opacity_variable_max );
                break;
            case algebricTransferFunction::ServerSideRange:
                // ui->opacityHistogram->setRange( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_min, m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_max );
                ui->opacityHistogram->update();
                ui->opacityMapMinLabel->setNum( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_min );
                ui->opacityMapMaxLabel->setNum( m_parameter.m_transfer_functions[ui->opacityFunctionComboBox->currentIndex()].m_opacity_server_side_max );
                break;
            default:
                break;
            }

            ui->opacityFunctionComboBox->blockSignals( false );
            ui->opacityFunctionVariableLineEdit->blockSignals( false );
            ui->opacityServerSideMinLineEdit->blockSignals( false );
            ui->opacityServerSideMaxLineEdit->blockSignals( false );
            ui->opacityMapBar->blockSignals( false );
        }
    }, Qt::QueuedConnection);

    //作成された伝達関数のうちサーバレンジを使用するものが存在する場合、粒子再生成を行う。
    bool isParticleGenerationNeeded = false;
    for( int i = 0; i < m_parameter.m_number_of_transfer_functions; i++ )
    {
        switch ( m_parameter.m_transfer_functions[i].m_color_confirmed_select_range ) //confirmd
        {
        case algebricTransferFunction::UserDefinedRange:
            break;
        case algebricTransferFunction::ServerSideRange:
            m_connect->getClientMessage()->m_transfer_function[i].m_color_variable_min = m_parameter.m_transfer_functions[i].m_color_server_side_min;
            m_connect->getClientMessage()->m_transfer_function[i].m_color_variable_max = m_parameter.m_transfer_functions[i].m_color_server_side_max;
            isParticleGenerationNeeded = true;
            break;
        default:
            break;
        }

        switch ( m_parameter.m_transfer_functions[i].m_opacity_confirmed_select_range ) //confirmd
        {
        case algebricTransferFunction::UserDefinedRange:
            break;
        case algebricTransferFunction::ServerSideRange:
            m_connect->getClientMessage()->m_transfer_function[i].m_opacity_variable_min = m_parameter.m_transfer_functions[i].m_opacity_server_side_min;
            m_connect->getClientMessage()->m_transfer_function[i].m_opacity_variable_max = m_parameter.m_transfer_functions[i].m_opacity_server_side_max;
            isParticleGenerationNeeded = true;
            break;
        default:
            break;
        }
    }
    if( isParticleGenerationNeeded )
    {
        m_merge->setIsParticleGenerationNeeded( true );
    }
    m_color_map_bar_selector->updateRangeView();
}

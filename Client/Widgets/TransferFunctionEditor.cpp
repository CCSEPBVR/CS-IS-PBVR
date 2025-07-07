#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"

TransferFunctionEditor::TransferFunctionEditor( QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::TransferFunctionEditor )
    , m_model( new QStandardItemModel( this ) )
{
    initialize();
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::initialize()
{
    ui->setupUi( this );
    ui->numberOfTransferFunctionLabelSpinBox->setMinimum( 0 );
    ui->numberOfTransferFunctionLabelSpinBox->setMaximum( 16 );

    disableUI();

    connect( ui->numberOfTransferFunctionLabelSpinBox   , &QSpinBox::valueChanged           , this, &TransferFunctionEditor::onNumberOfTransferFunctionValueChanged );

    connect( ui->colorSynthesizerLineEdit               , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onColorSynthesizerChanged );
    connect( ui->colorFunctionComboBox                  , &QComboBox::currentIndexChanged   , this, &TransferFunctionEditor::updateUIFromCurrentItem );
    connect( ui->colorFunctionVariableLineEdit          , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onColorFunctionVariableChanged );
    connect( ui->colorUserDefinedMinMaxRadioButton      , &QRadioButton::clicked            , this, &TransferFunctionEditor::onColorUserDefinedMinMaxClicked );
    connect( ui->colorServerSideMinMaxRadioButton       , &QRadioButton::clicked            , this, &TransferFunctionEditor::onColorServerSideMinMaxClicked );
    connect( ui->colorUserDefinedMinSpinBox             , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onColorUserDefinedMinChanged );
    connect( ui->colorUserDefinedMaxSpinBox             , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onColorUserDefinedMaxChanged );
    connect( ui->colorFunctionPushButton                , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditColorVariable );
    connect( ui->editColorMapPushbutton                 , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditColorMap );

    connect( ui->opacitySynthesizerLineEdit             , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onOpacitySynthesizerChanged );
    connect( ui->opacityFunctionComboBox                , &QComboBox::currentIndexChanged   , this, &TransferFunctionEditor::updateUIFromCurrentItem );
    connect( ui->opacityFunctionVariableLineEdit        , &QLineEdit::textChanged           , this, &TransferFunctionEditor::onOpacityFunctionVariableChanged );
    connect( ui->opacityUserDefinedMinMaxRadioButton    , &QRadioButton::clicked            , this, &TransferFunctionEditor::onOpacityUserDefinedMinMaxClicked );
    connect( ui->opacityServerSideMinMaxRadioButton     , &QRadioButton::clicked            , this, &TransferFunctionEditor::onOpacityServerSideMinMaxClicked );
    connect( ui->opacityUserDefinedMinSpinBox           , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onOpacityUserDefinedMinChanged );
    connect( ui->opacityUserDefinedMaxSpinBox           , &QDoubleSpinBox::valueChanged     , this, &TransferFunctionEditor::onOpacityUserDefinedMaxChanged );
    connect( ui->opacityFunctionPushButton              , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditOpacityVariable );
    connect( ui->editOpacityMapPushbutton               , &QPushButton::clicked             , this, &TransferFunctionEditor::onEditOpacityMap );

    connect( ui->exportPushButton                       , &QPushButton::clicked             , this, &TransferFunctionEditor::onExport );
    connect( ui->importPushButton                       , &QPushButton::clicked             , this, &TransferFunctionEditor::onImport );
    connect( ui->applyPushButton                        , &QPushButton::clicked             , this, &TransferFunctionEditor::onApply );
}

void TransferFunctionEditor::disableUI()
{
    ui->colorSynthesizerLineEdit->setEnabled( false );
    ui->colorFunctionComboBox->setEnabled( false );
    ui->colorFunctionVariableLineEdit->setEnabled( false );
    ui->colorUserDefinedMinMaxRadioButton->setEnabled( false );
    ui->colorServerSideMinMaxRadioButton->setEnabled( false );
    ui->colorUserDefinedMinSpinBox->setEnabled( false );
    ui->colorUserDefinedMaxSpinBox->setEnabled( false );
    ui->colorServerSideMinSpinBox->setEnabled( false );
    ui->colorServerSideMaxSpinBox->setEnabled( false );
    ui->colorFunctionPushButton->setEnabled( false );
    ui->editColorMapPushbutton->setEnabled( false );

    ui->opacitySynthesizerLineEdit->setEnabled( false );
    ui->opacityFunctionComboBox->setEnabled( false );
    ui->opacityFunctionVariableLineEdit->setEnabled( false );
    ui->opacityUserDefinedMinMaxRadioButton->setEnabled( false );
    ui->opacityServerSideMinMaxRadioButton->setEnabled( false );
    ui->opacityUserDefinedMinSpinBox->setEnabled( false );
    ui->opacityUserDefinedMaxSpinBox->setEnabled( false );
    ui->opacityServerSideMinSpinBox->setEnabled( false );
    ui->opacityServerSideMaxSpinBox->setEnabled( false );
    ui->opacityFunctionPushButton->setEnabled( false );
    ui->editOpacityMapPushbutton->setEnabled( false );

    ui->exportPushButton->setEnabled( false );
    ui->applyPushButton->setEnabled( false );

    QVector<QColor> initialColors( 256, QColor( 0, 0, 0 ) );
    ui->colorMapBar->setColors( initialColors );
    QVector<float> initialOpacity( 256, 0.0f );
    ui->opacityMapBar->setOpacities( initialOpacity );
    std::vector<int> initialHistogram( 256, 0 );
    ui->colorHistogram->setDatas( initialHistogram );
    ui->opacityHistogram->setDatas( initialHistogram );
}

void TransferFunctionEditor::enableUI()
{
    ui->colorSynthesizerLineEdit->setEnabled( true );
    ui->colorFunctionComboBox->setEnabled( true );
    ui->colorFunctionVariableLineEdit->setEnabled( true );
    ui->colorUserDefinedMinMaxRadioButton->setEnabled( true );
    ui->colorServerSideMinMaxRadioButton->setEnabled( true );
    ui->colorUserDefinedMinSpinBox->setEnabled( true );
    ui->colorUserDefinedMaxSpinBox->setEnabled( true );
    ui->colorServerSideMinSpinBox->setEnabled( true );
    ui->colorServerSideMaxSpinBox->setEnabled( true );
    ui->colorFunctionPushButton->setEnabled( true );
    ui->editColorMapPushbutton->setEnabled( true );

    ui->opacitySynthesizerLineEdit->setEnabled( true );
    ui->opacityFunctionComboBox->setEnabled( true );
    ui->opacityFunctionVariableLineEdit->setEnabled( true );
    ui->opacityUserDefinedMinMaxRadioButton->setEnabled( true );
    ui->opacityServerSideMinMaxRadioButton->setEnabled( true );
    ui->opacityUserDefinedMinSpinBox->setEnabled( true );
    ui->opacityUserDefinedMaxSpinBox->setEnabled( true );
    ui->opacityServerSideMinSpinBox->setEnabled( true );
    ui->opacityServerSideMaxSpinBox->setEnabled( true );
    ui->opacityFunctionPushButton->setEnabled( true );
    ui->editOpacityMapPushbutton->setEnabled( true );

    ui->exportPushButton->setEnabled( true );
    ui->applyPushButton->setEnabled( true );
}

void TransferFunctionEditor::onNumberOfTransferFunctionValueChanged( int numberOfTransferFunction )
{
    TransferFunctionItem transferFunctionItemCreator;

    const int currentRowCount = m_model->rowCount();

    // モデルの行数を増やす場合
    if (numberOfTransferFunction > currentRowCount)
    {
        for (int i = currentRowCount; i < numberOfTransferFunction; i++)
        {
            QList<QStandardItem*> items = transferFunctionItemCreator.createItem(i + 1);
            if (!items.isEmpty())
            {
                m_model->appendRow(items);
            }
        }
    }
    // 減らす場合は末尾から削除
    else if (numberOfTransferFunction < currentRowCount)
    {
        while (m_model->rowCount() > numberOfTransferFunction)
        {
            m_model->removeRow(m_model->rowCount() - 1);
        }
    }

    // --- ComboBox の同期処理 ---
    // colorFunctionComboBox の更新
    while (ui->colorFunctionComboBox->count() > numberOfTransferFunction)
        ui->colorFunctionComboBox->removeItem(ui->colorFunctionComboBox->count() - 1);
    for (int i = ui->colorFunctionComboBox->count(); i < numberOfTransferFunction; ++i)
    {
        QStandardItem* item = m_model->item(i);
        if (!item) continue;
        QVariant data = item->data(TransferFunctionItem::ColorFunction);
        ui->colorFunctionComboBox->addItem(data.toString(), data);
    }

    // opacityFunctionComboBox の更新
    while (ui->opacityFunctionComboBox->count() > numberOfTransferFunction)
        ui->opacityFunctionComboBox->removeItem(ui->opacityFunctionComboBox->count() - 1);
    for (int i = ui->opacityFunctionComboBox->count(); i < numberOfTransferFunction; ++i)
    {
        QStandardItem* item = m_model->item(i);
        if (!item) continue;
        QVariant data = item->data(TransferFunctionItem::OpacityFunction);
        ui->opacityFunctionComboBox->addItem(data.toString(), data);
    }

    // --- 0個の場合はUIリセット ---
    if (numberOfTransferFunction == 0)
    {
        ui->colorFunctionVariableLineEdit->clear();
        ui->colorUserDefinedMinSpinBox->setValue(0.00);
        ui->colorUserDefinedMaxSpinBox->setValue(0.00);
        ui->colorServerSideMinSpinBox->setValue(0.00);
        ui->colorServerSideMaxSpinBox->setValue(0.00);
        ui->colorMinLabel->setText("Min : ");
        ui->colorMaxLabel->setText(" : Max");

        disableUI();
    }
    else
    {
        enableUI();
    }
}

void TransferFunctionEditor::onColorSynthesizerChanged( const QString &colorSynthesizer )
{
    m_color_synthesizer = colorSynthesizer;
}

void TransferFunctionEditor::onColorFunctionVariableChanged( const QString &colorFunctionVariable )
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( colorFunctionVariable, TransferFunctionItem::ColorVariable );
}

void TransferFunctionEditor::onColorUserDefinedMinMaxClicked()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->colorUserDefinedMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::UserRange, TransferFunctionItem::TemporaryColorRangeMode );
    }
}

void TransferFunctionEditor::onColorServerSideMinMaxClicked()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->colorServerSideMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::ServerRange, TransferFunctionItem::TemporaryColorRangeMode );
    }
}

void TransferFunctionEditor::onColorUserDefinedMinChanged( const float &colorUserDefinedMin )
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( colorUserDefinedMin, TransferFunctionItem::ColorUserRangeMin );
}

void TransferFunctionEditor::onColorUserDefinedMaxChanged( const float &colorUserDefinedMax )
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( colorUserDefinedMax, TransferFunctionItem::ColorUserRangeMax );
}

void TransferFunctionEditor::onEditColorVariable()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    VariableEditor colorVariableEditor( VariableEditor::TYPE::COLOR , *m_model );
    colorVariableEditor.adjustSize();

    if( colorVariableEditor.exec() == QDialog::Accepted )
    {
        const QStandardItemModel& editedModel = colorVariableEditor.editedModel();

        for( int row = 0; row < m_model->rowCount() && row < editedModel.rowCount(); row++ )
        {
            QStandardItem* item = m_model->item( row );
            const QStandardItem* editedVarItem = editedModel.item( row, 1 ); // 2列目が variable

            if ( editedVarItem )
            {
                item->setData( editedVarItem->text(), TransferFunctionItem::ColorVariable );
            }
        }
        updateUIFromCurrentItem();
    }
}

void TransferFunctionEditor::onEditColorMap()
{
    int index = ui->colorFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    ColorMapEditor colorMapEditor;
    colorMapEditor.adjustSize();
    colorMapEditor.setDefaultColorMap( ui->colorMapBar->getColors() );

    if( colorMapEditor.exec() == QDialog::Accepted )
    {
        QVector<QColor> qcolors = colorMapEditor.getColorMap();
        item->setData( QVariant::fromValue( qcolors ), TransferFunctionItem::ColorMap );
        updateUIFromCurrentItem();
    }
}
//
void TransferFunctionEditor::onOpacitySynthesizerChanged( const QString &opacitySynthesizer )
{
    m_opacity_synthesizer = opacitySynthesizer;
}

void TransferFunctionEditor::onOpacityFunctionVariableChanged( const QString &opacityFunctionVariable )
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( opacityFunctionVariable, TransferFunctionItem::OpacityVariable );
}

void TransferFunctionEditor::onOpacityUserDefinedMinMaxClicked()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->opacityUserDefinedMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::UserRange, TransferFunctionItem::TemporaryOpacityRangeMode );
    }
}

void TransferFunctionEditor::onOpacityServerSideMinMaxClicked()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    if( ui->opacityServerSideMinMaxRadioButton->isChecked() )
    {
        item->setData( TransferFunctionItem::RangeSelectionMode::ServerRange, TransferFunctionItem::TemporaryOpacityRangeMode );
    }
}

void TransferFunctionEditor::onOpacityUserDefinedMinChanged( const float &OpacityUserDefinedMin )
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( OpacityUserDefinedMin, TransferFunctionItem::OpacityUserRangeMin );
}

void TransferFunctionEditor::onOpacityUserDefinedMaxChanged( const float &OpacityUserDefinedMax )
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    item->setData( OpacityUserDefinedMax, TransferFunctionItem::OpacityUserRangeMax );
}

void TransferFunctionEditor::onEditOpacityVariable()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    VariableEditor OpacityVariableEditor( VariableEditor::TYPE::OPACITY , *m_model );
    OpacityVariableEditor.adjustSize();

    if( OpacityVariableEditor.exec() == QDialog::Accepted )
    {
        const QStandardItemModel& editedModel = OpacityVariableEditor.editedModel();

        for( int row = 0; row < m_model->rowCount() && row < editedModel.rowCount(); row++ )
        {
            QStandardItem* item = m_model->item( row );
            const QStandardItem* editedVarItem = editedModel.item( row, 1 ); // 2列目が variable

            if ( editedVarItem )
            {
                item->setData( editedVarItem->text(), TransferFunctionItem::OpacityVariable );
            }
        }
        updateUIFromCurrentItem();
    }
}

void TransferFunctionEditor::onEditOpacityMap()
{
    int index = ui->opacityFunctionComboBox->currentIndex();
    if( !m_model || index < 0 || index >= m_model->rowCount() ) return;

    QStandardItem* item = m_model->item( index );
    if( !item ) return;

    OpacityMapEditor OpacityMapEditor;
    OpacityMapEditor.adjustSize();
    OpacityMapEditor.setDefaultOpacityMap( ui->opacityMapBar->getOpacities() );

    if( OpacityMapEditor.exec() == QDialog::Accepted )
    {
        QVector<float> opacities = OpacityMapEditor.getOpacityMap();
        item->setData( QVariant::fromValue( opacities ), TransferFunctionItem::OpacityMap );
        updateUIFromCurrentItem();
    }
}

void TransferFunctionEditor::updateUIFromCurrentItem()
{
    // --- Color ---
    {
        int indexColor = ui->colorFunctionComboBox->currentIndex();
        if( !m_model || indexColor < 0 || indexColor >= m_model->rowCount() ) return;

        QStandardItem* itemColor = m_model->item( indexColor );
        if( !itemColor ) return;

        ui->colorFunctionVariableLineEdit->setText( itemColor->data( TransferFunctionItem::ColorVariable ).toString() );

        int temporaryColorStatus = itemColor->data( TransferFunctionItem::TemporaryColorRangeMode ).toInt();
        if( temporaryColorStatus == TransferFunctionItem::UserRange )
        {
            ui->colorUserDefinedMinMaxRadioButton->setChecked( true );
        }
        else if( temporaryColorStatus == TransferFunctionItem::ServerRange )
        {
            ui->colorServerSideMinMaxRadioButton->setChecked( true );
        }

        ui->colorUserDefinedMinSpinBox->setValue( itemColor->data( TransferFunctionItem::ColorUserRangeMin ).toDouble() );
        ui->colorUserDefinedMaxSpinBox->setValue( itemColor->data( TransferFunctionItem::ColorUserRangeMax ).toDouble() );
        ui->colorServerSideMinSpinBox->setValue( itemColor->data( TransferFunctionItem::ColorServerRangeMin ).toDouble() );
        ui->colorServerSideMaxSpinBox->setValue( itemColor->data( TransferFunctionItem::ColorServerRangeMax ).toDouble() );
        ui->colorMapBar->setColors( itemColor->data( TransferFunctionItem::ColorMap ).value<QVector<QColor>>() );

        std::vector<int> colorHistogram = itemColor->data( TransferFunctionItem::ColorHistogram ).value<std::vector<int>>();
        ui->colorHistogram->setDatas( colorHistogram );
        ui->colorHistogram->update();

        int resultColorRangeMode = itemColor->data( TransferFunctionItem::ResultColorRangeMode ).toInt();
        float colorMin = 0.0f;
        float colorMax = 0.0f;

        if( resultColorRangeMode == TransferFunctionItem::UserRange )
        {
            colorMin = itemColor->data( TransferFunctionItem::ColorUserRangeMin ).toDouble();
            colorMax = itemColor->data( TransferFunctionItem::ColorUserRangeMax ).toDouble();
            ui->colorMinLabel->setText( "Min : " + QString::number( colorMin ) );
            ui->colorMaxLabel->setText( QString::number( colorMax ) + " : Max" );
        }
        else if( resultColorRangeMode == TransferFunctionItem::ServerRange )
        {
            colorMin = itemColor->data( TransferFunctionItem::ColorServerRangeMin ).toDouble();
            colorMax = itemColor->data( TransferFunctionItem::ColorServerRangeMax ).toDouble();
            ui->colorMinLabel->setText( "Min : " + QString::number( colorMin ) );
            ui->colorMaxLabel->setText( QString::number( colorMax ) + " : Max" );
        }
        else
        {
            ui->colorMinLabel->setText( "Min : " );
            ui->colorMaxLabel->setText( " : Max" );
        }
    }

    // --- Opacity ---
    {
        int indexOpacity = ui->opacityFunctionComboBox->currentIndex();
        if( !m_model || indexOpacity < 0 || indexOpacity >= m_model->rowCount() ) return;

        QStandardItem* itemOpacity = m_model->item( indexOpacity );
        if( !itemOpacity ) return;

        ui->opacityFunctionVariableLineEdit->setText( itemOpacity->data( TransferFunctionItem::OpacityVariable ).toString() );

        int temporaryopacityStatus = itemOpacity->data( TransferFunctionItem::TemporaryOpacityRangeMode ).toInt();
        if( temporaryopacityStatus == TransferFunctionItem::UserRange )
        {
            ui->opacityUserDefinedMinMaxRadioButton->setChecked( true );
        }
        else if( temporaryopacityStatus == TransferFunctionItem::ServerRange )
        {
            ui->opacityServerSideMinMaxRadioButton->setChecked( true );
        }

        ui->opacityUserDefinedMinSpinBox->setValue( itemOpacity->data( TransferFunctionItem::OpacityUserRangeMin ).toDouble() );
        ui->opacityUserDefinedMaxSpinBox->setValue( itemOpacity->data( TransferFunctionItem::OpacityUserRangeMax ).toDouble() );
        ui->opacityServerSideMinSpinBox->setValue( itemOpacity->data( TransferFunctionItem::OpacityServerRangeMin ).toDouble() );
        ui->opacityServerSideMaxSpinBox->setValue( itemOpacity->data( TransferFunctionItem::OpacityServerRangeMax ).toDouble() );
        ui->opacityMapBar->setOpacities( itemOpacity->data( TransferFunctionItem::OpacityMap ).value<QVector<float>>() );

        std::vector<int> opacityHistogram = itemOpacity->data( TransferFunctionItem::OpacityHistogram ).value<std::vector<int>>();
        ui->opacityHistogram->setDatas( opacityHistogram );
        ui->opacityHistogram->update();

        int resultOpacityRangeMode = itemOpacity->data( TransferFunctionItem::ResultOpacityRangeMode ).toInt();
        float opacityMin = 0.0f;
        float opacityMax = 0.0f;

        if( resultOpacityRangeMode == TransferFunctionItem::UserRange )
        {
            opacityMin = itemOpacity->data( TransferFunctionItem::OpacityUserRangeMin ).toDouble();
            opacityMax = itemOpacity->data( TransferFunctionItem::OpacityUserRangeMax ).toDouble();
            ui->opacityMinLabel->setText( "Min : " + QString::number( opacityMin ) );
            ui->opacityMaxLabel->setText( QString::number( opacityMax ) + " : Max" );
        }
        else if( resultOpacityRangeMode == TransferFunctionItem::ServerRange )
        {
            opacityMin = itemOpacity->data( TransferFunctionItem::OpacityServerRangeMin ).toDouble();
            opacityMax = itemOpacity->data( TransferFunctionItem::OpacityServerRangeMax ).toDouble();
            ui->opacityMinLabel->setText( "Min : " + QString::number( opacityMin ) );
            ui->opacityMaxLabel->setText( QString::number( opacityMax ) + " : Max" );
        }
        else
        {
            ui->opacityMinLabel->setText( "Min : " );
            ui->opacityMaxLabel->setText( " : Max" );
        }
    }
}

void TransferFunctionEditor::onExport()
{
    if ( !m_model ) return;

    // ファイル名をユーザーに指定させる
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr( "Transfer Function をエクスポート" ),
        "",
        tr( "Transfer Function Export (*.tfe);;All Files (*)" )
        );
    if ( filePath.isEmpty() ) return;

    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        qWarning() << "ファイルを開けませんでした:" << filePath;
        return;
    }

    QTextStream out( &file );

    // 総数と合成式などヘッダー部分
    int tf_count = m_model->rowCount();
    out << "TF_NUMBER=" << tf_count << "\n";
    out << "TF_SYNTH_C=" << m_color_synthesizer << "\n";
    out << "TF_SYNTH_O=" << m_opacity_synthesizer << "\n";

    // 各 Transfer Function 行を出力
    for ( int i = 0; i < tf_count; ++i )
    {
        QStandardItem* item = m_model->item( i );
        if ( !item ) continue;

        QString tagBase = QString( "TF_NAME%1_" ).arg( i + 1 );

        auto writeIfValid = [&]( const QString& key, int role )
        {
            QVariant val = item->data( role );
            if ( val.isValid() )
            {
                out << tagBase << key << "=" << val.toString() << "\n";
            }
        };

        // --- Color 関連出力 ---
        writeIfValid( "C", TransferFunctionItem::ColorFunction );
        writeIfValid( "VAR_C", TransferFunctionItem::ColorVariable );
        writeIfValid( "MIN_C", TransferFunctionItem::ColorUserRangeMin );
        writeIfValid( "MAX_C", TransferFunctionItem::ColorUserRangeMax );

        QVariant colorMapVar = item->data( TransferFunctionItem::ColorMap );
        if ( colorMapVar.isValid() )
        {
            QVector<QColor> colors = colorMapVar.value<QVector<QColor>>();
            QStringList colorStrings;
            for ( const QColor& c : colors )
            {
                colorStrings << QString::number( c.red() )
                             << QString::number( c.green() )
                             << QString::number( c.blue() );
            }
            out << tagBase << "TABLE_C=" << colorStrings.join( "," ) << "\n";
        }

        // --- Opacity 関連出力 ---
        writeIfValid( "O", TransferFunctionItem::OpacityFunction );
        writeIfValid( "VAR_O", TransferFunctionItem::OpacityVariable );
        writeIfValid( "MIN_O", TransferFunctionItem::OpacityUserRangeMin );
        writeIfValid( "MAX_O", TransferFunctionItem::OpacityUserRangeMax );

        QVariant opacityMapVar = item->data( TransferFunctionItem::OpacityMap );
        if ( opacityMapVar.isValid() )
        {
            QVector<float> opacities = opacityMapVar.value<QVector<float>>();
            QStringList opacityStrings;
            for ( float v : opacities )
            {
                opacityStrings << QString::number( v );
            }
            out << tagBase << "TABLE_O=" << opacityStrings.join( "," ) << "\n";
        }
    }

    file.close();
    qDebug() << "Transfer Function を保存しました:" << filePath;
}



void TransferFunctionEditor::onImport()
{
    QFileDialog fileDialog( this, tr( "Import Transfer Function File" ), ".", tr( "Transfer Function Files (*.tfe *.TFE *.tf *.TF )" ) );
    fileDialog.setFileMode( QFileDialog::ExistingFile );

    if( fileDialog.exec() )
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        QString selectedFile = selectedFiles.first();

        importTransferFunctionFromFile( selectedFile );
    }
}

void TransferFunctionEditor::onApply()
{
    applyTransferFunction();
}

void TransferFunctionEditor::importTransferFunctionFromFile( const QString transferFunctionFile )
{
    QFile file( transferFunctionFile );
    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        emit failedTransferFunctionImport();
        return;
    }

    QTextStream in( &file );

    int tfCount = 0;

    // --- ここに keyHandlers を定義 ---
    QMap<QString, std::function<void( const QString& ) >> keyHandlers =
        {
            { "TF_NUMBER", [this, &tfCount]( const QString& value ) {
                 tfCount = value.toInt();
                 ui->numberOfTransferFunctionLabelSpinBox->setValue( tfCount );
             }},
            { "TF_SYNTH_C", [this]( const QString& value ) {
                 m_color_synthesizer = value;
                 ui->colorSynthesizerLineEdit->setText( value );
             }},
            { "TF_SYNTH_O", [this]( const QString& value ) {
                 m_opacity_synthesizer = value;
                 ui->opacitySynthesizerLineEdit->setText( value );
             }}
        };

    QString line;
    QRegularExpression tfRegex( R"(TF_NAME(\d+)_([A-Z]+)_([CO]))" );

    while( !in.atEnd() )
    {
        line = in.readLine().trimmed();
        if( !line.contains( '=' ) ) continue;

        QString key = line.section( '=', 0, 0 );
        QString value = line.section( '=', 1 );

        // --- 固定キー処理 ---
        if( keyHandlers.contains( key ) )
        {
            keyHandlers[key]( value );
            continue;
        }

        // --- TF_NAME{n}_KEY_{C|O} 処理 ---
        QRegularExpressionMatch match = tfRegex.match( key );
        if( match.hasMatch() )
        {
            int index = match.captured( 1 ).toInt() - 1;
            QString field = match.captured( 2 );
            QString type = match.captured( 3 ); // "C" または "O"

            if( index < 0 || index >= m_model->rowCount() ) continue;
            QStandardItem* item = m_model->item( index );
            if( !item ) continue;

            // --- カラー／不透明度の各種フィールドに対応 ---
            if( type == "C" )
            {
                if( field == "C" )
                {
                    item->setData( value, TransferFunctionItem::ColorFunction );
                }
                else if( field == "VAR" )
                {
                    item->setData( value, TransferFunctionItem::ColorVariable );
                }
                else if( field == "MIN" )
                {
                    item->setData( value.toDouble(), TransferFunctionItem::ColorUserRangeMin );
                }
                else if( field == "MAX" )
                {
                    item->setData( value.toDouble(), TransferFunctionItem::ColorUserRangeMax );
                }
                else if( field == "TABLE" )
                {
                    QStringList strValues = value.split( ',', Qt::SkipEmptyParts );
                    QVector<QColor> colors;
                    for( int j = 0; j + 2 < strValues.size(); j += 3 )
                    {
                        colors.append( QColor(strValues[j].toInt(), strValues[j+1].toInt(), strValues[j+2].toInt()) );
                    }
                    item->setData( QVariant::fromValue(colors), TransferFunctionItem::ColorMap );
                }
            }
            else if( type == "O" )
            {
                if( field == "O" )
                {
                    item->setData( value, TransferFunctionItem::OpacityFunction );
                }
                else if( field == "VAR" )
                {
                    item->setData( value, TransferFunctionItem::OpacityVariable );
                }
                else if( field == "MIN" )
                {
                    item->setData( value.toDouble(), TransferFunctionItem::OpacityUserRangeMin );
                }
                else if( field == "MAX" )
                {
                    item->setData( value.toDouble(), TransferFunctionItem::OpacityUserRangeMax );
                }
                else if( field == "TABLE" )
                {
                    QStringList strValues = value.split( ',', Qt::SkipEmptyParts );
                    QVector<float> opacities;
                    for( const auto& s : strValues )
                    {
                        opacities.append( s.toFloat() );
                    }
                    item->setData( QVariant::fromValue(opacities), TransferFunctionItem::OpacityMap );
                }
            }
            item->setData( QVariant::fromValue( TransferFunctionItem::UserRange ), TransferFunctionItem::TemporaryColorRangeMode );
            item->setData( QVariant::fromValue( TransferFunctionItem::UserRange ), TransferFunctionItem::TemporaryOpacityRangeMode );
        }
    }
    emit successTransferFunctionImport( transferFunctionFile );
    updateUIFromCurrentItem();
}

// void TransferFunctionEditor::importTransferFunctionFromServer( const jpv::ParticleTransferServerMessage& serverMessage, kvs::visclient::ReceivedMessage receivedMessage )
void TransferFunctionEditor::importTransferFunctionFromServer( const jpv::ParticleTransferServerMessage& serverMessage, kvs::visclient::ReceivedMessage receivedMessage, const bool isImportFlag )
{
    const int tfCount = serverMessage.m_transfer_function.size();
    ui->numberOfTransferFunctionLabelSpinBox->setValue( tfCount );

    if( !isImportFlag ) // 伝達関数ファイルがインポートされていない場合
    {
        ui->colorSynthesizerLineEdit->setText( QString::fromUtf8( serverMessage.m_color_transfer_function_synthesis ) );
        ui->opacitySynthesizerLineEdit->setText( QString::fromUtf8( serverMessage.m_opacity_transfer_function_synthesis ) );
    }

    for( int i = 0; i < tfCount; i++ )
    {
        QStandardItem* item = m_model->item( i );
        if( !item ) continue;

        // --- Color ---
        {
            std::string colorFunctionName = "C" + std::to_string(i + 1);
            QString tag_c = QString("t%1_var_c").arg(i + 1);
            if( !isImportFlag ) // 伝達関数ファイルがインポートされていない場合
            {
                item->setData( QString::fromUtf8( serverMessage.m_transfer_function[i].m_color_variable ), TransferFunctionItem::ColorVariable );
                item->setData( 0.0, TransferFunctionItem::ColorUserRangeMin );
                item->setData( 1.0, TransferFunctionItem::ColorUserRangeMax );


                const auto& raw = serverMessage.m_transfer_function[i].colorMap().table();
                QVector<QColor> colors;
                for( size_t j = 0; j + 2 < raw.size(); j += 3 )
                {
                    colors.append( QColor( raw[j], raw[j + 1], raw[j + 2] ) );
                }
                item->setData( QVariant::fromValue( colors ), TransferFunctionItem::ColorMap );

                item->setData( TransferFunctionItem::ServerRange, TransferFunctionItem::TemporaryColorRangeMode );
            }
            else
            {
                item->setData( QVariant::fromValue( TransferFunctionItem::UserRange ), TransferFunctionItem::TemporaryColorRangeMode );
            }

            item->setData( serverMessage.m_server_side_variable_range.min( tag_c.toUtf8().data() ), TransferFunctionItem::ColorServerRangeMin );
            item->setData( serverMessage.m_server_side_variable_range.max( tag_c.toUtf8().data() ), TransferFunctionItem::ColorServerRangeMax );

            if( const auto* hist = receivedMessage.findColorFrequencyTable( colorFunctionName ) )
            {
                std::vector<int> ch;
                ch.reserve(hist->bin().size());
                for( int j = 0; j < hist->bin().size(); j++ )
                {
                    ch.push_back( hist->bin().at( j ) );
                }
                item->setData( QVariant::fromValue( ch ), TransferFunctionItem::ColorHistogram );
            }
            item->setData( TransferFunctionItem::ServerRange, TransferFunctionItem::ResultColorRangeMode );
        }

        // --- Opacity ---
        {
            std::string opacityFunctionName = "O" + std::to_string( i + 1 );
            QString tag_o = QString( "t%1_var_o" ).arg( i + 1 );
            if( !isImportFlag ) // 伝達関数ファイルがインポートされていない場合
            {
                item->setData( QString::fromUtf8( serverMessage.m_transfer_function[i].m_opacity_variable ), TransferFunctionItem::OpacityVariable );
                item->setData( 0.0, TransferFunctionItem::OpacityUserRangeMin );
                item->setData( 1.0, TransferFunctionItem::OpacityUserRangeMax );

                const auto& raw = serverMessage.m_transfer_function[i].opacityMap().table();
                QVector<float> opacities( raw.begin(), raw.end() );
                item->setData( QVariant::fromValue( opacities ), TransferFunctionItem::OpacityMap );

                item->setData( TransferFunctionItem::ServerRange, TransferFunctionItem::TemporaryOpacityRangeMode );
            }
            else
            {
                item->setData( QVariant::fromValue( TransferFunctionItem::UserRange ), TransferFunctionItem::TemporaryOpacityRangeMode );
            }

            item->setData( serverMessage.m_server_side_variable_range.min( tag_o.toUtf8().data() ), TransferFunctionItem::OpacityServerRangeMin );
            item->setData( serverMessage.m_server_side_variable_range.max( tag_o.toUtf8().data() ), TransferFunctionItem::OpacityServerRangeMax );

            if( const auto* hist = receivedMessage.findOpacityFrequencyTable( opacityFunctionName ) )
            {
                std::vector<int> oh;
                oh.reserve( hist->bin().size() );
                for( int j = 0; j < hist->bin().size(); j++ )
                {
                    oh.push_back( hist->bin().at( j ) );
                }
                item->setData( QVariant::fromValue(oh), TransferFunctionItem::OpacityHistogram );
            }
            item->setData( TransferFunctionItem::ServerRange, TransferFunctionItem::ResultOpacityRangeMode );
        }
    }

    updateUIFromCurrentItem();
}

void TransferFunctionEditor::applyTransferFunction()
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* item = m_model->item( row );
        if( !item ) continue;

        QVariant tmpValue = item->data( TransferFunctionItem::TemporaryColorRangeMode );
        if( tmpValue.isValid() )
        {
            item->setData( tmpValue, TransferFunctionItem::CurrentColorRangeMode );
        }

        tmpValue = item->data( TransferFunctionItem::TemporaryOpacityRangeMode );
        if( tmpValue.isValid() )
        {
            item->setData( tmpValue, TransferFunctionItem::CurrentOpacityRangeMode );
        }
    }

    emit updateColorMapBar( m_model );
    emit updateTransferFunctionClientMessage( m_color_synthesizer, m_opacity_synthesizer, m_model );
    emit requestReplaceServerPointObject();
}

void TransferFunctionEditor::updateTransferFunctionRangeAndView( const jpv::ParticleTransferServerMessage* serverMessage, kvs::visclient::ReceivedMessage receivedMessage )
{
    const int tfCount = serverMessage->m_transfer_function_count;
    for( int i = 0; i < tfCount; i++ )
    {
        QStandardItem* item = m_model->item( i );
        if( !item ) continue;

        // --- Color ---
        std::string colorFunctionName = "C" + std::to_string(i + 1);
        QString tag_c = QString("t%1_var_c").arg(i + 1);

        item->setData( serverMessage->m_server_side_variable_range.min( tag_c.toUtf8().data() ), TransferFunctionItem::ColorServerRangeMin );
        item->setData( serverMessage->m_server_side_variable_range.max( tag_c.toUtf8().data() ), TransferFunctionItem::ColorServerRangeMax );

        if( const auto* hist = receivedMessage.findColorFrequencyTable( colorFunctionName ) )
        {
            std::vector<int> ch;
            ch.reserve(hist->bin().size());
            for( int j = 0; j < hist->bin().size(); j++ )
            {
                ch.push_back( hist->bin().at( j ) );
            }
            item->setData( QVariant::fromValue( ch ), TransferFunctionItem::ColorHistogram );
        }

        int colorRangeMode = item->data( TransferFunctionItem::CurrentColorRangeMode ).toInt();

        if( colorRangeMode == TransferFunctionItem::RangeSelectionMode::UserRange )
        {
            item->setData( TransferFunctionItem::UserRange, TransferFunctionItem::ResultColorRangeMode );
        }
        else if( colorRangeMode == TransferFunctionItem::RangeSelectionMode::ServerRange )
        {
            emit requestReplaceServerPointObject();
            item->setData( TransferFunctionItem::ServerRange, TransferFunctionItem::ResultColorRangeMode );
        }

        // --- Opacity ---
        std::string opacityFunctionName = "O" + std::to_string( i + 1 );
        QString tag_o = QString( "t%1_var_o" ).arg( i + 1 );

        item->setData( serverMessage->m_server_side_variable_range.min( tag_o.toUtf8().data() ), TransferFunctionItem::OpacityServerRangeMin );
        item->setData( serverMessage->m_server_side_variable_range.max( tag_o.toUtf8().data() ), TransferFunctionItem::OpacityServerRangeMax );

        if( const auto* hist = receivedMessage.findOpacityFrequencyTable( opacityFunctionName ) )
        {
            std::vector<int> oh;
            oh.reserve( hist->bin().size() );
            for( int j = 0; j < hist->bin().size(); j++ )
            {
                oh.push_back( hist->bin().at( j ) );
            }
            item->setData( QVariant::fromValue(oh), TransferFunctionItem::OpacityHistogram );
        }

        int opacityRangeMode = item->data( TransferFunctionItem::CurrentOpacityRangeMode ).toInt();

        if( opacityRangeMode == TransferFunctionItem::RangeSelectionMode::UserRange )
        {
            item->setData( TransferFunctionItem::UserRange, TransferFunctionItem::ResultOpacityRangeMode );
        }
        else if( opacityRangeMode == TransferFunctionItem::RangeSelectionMode::ServerRange )
        {
            emit requestReplaceServerPointObject();
            item->setData( TransferFunctionItem::ServerRange, TransferFunctionItem::ResultOpacityRangeMode );
        }
    }

    updateUIFromCurrentItem();
    emit updateColorMapBar( m_model );
}

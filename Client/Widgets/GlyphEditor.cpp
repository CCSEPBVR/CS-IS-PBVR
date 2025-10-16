#include "GlyphEditor.h"
#include "ui_GlyphEditor.h"

GlyphEditor::GlyphEditor( QWebSocket* textSocket, QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::GlyphEditor )
    , m_web_text_socket( textSocket )
    , m_model( new QStandardItemModel( this ) )
{
    initialize();
}

GlyphEditor::~GlyphEditor()
{
    delete ui;
}

void GlyphEditor::initialize()
{
    ui->setupUi( this );

    QStringList types;
    types << "Arrow" << "Diamond" << "Sphere";
    ui->glyphTypeComboBox->addItems( types );
    ui->scaleFactorDoubleSpinBox->setValue( 1.0 );

    ui->sizeConstantRadioButton->setChecked( true );
    ui->sizeNumberOfVariablesSpinBox->setMaximum( 0 );
    m_size_model = new QStandardItemModel( 0, 2, this );
    ui->sizeTreeView->header()->setVisible( false );
    ui->sizeTreeView->setModel( m_size_model );

    ui->uniformDistributionRadioButton->setChecked( true );
    ui->numberOfSamplePointsSpinBox->setValue( 1000 );
    ui->seedSpinBox->setValue( 1 );
    ui->strideSpinBox->setValue( 3 );

    ui->colorDataConstantRadioButton->setChecked( true );
    ui->colorDataNumberOfVariablesSpinBox->setMaximum( 0 );
    m_color_data_model = new QStandardItemModel( 0, 2, this );
    ui->colorDataTreeView->header()->setVisible( false );
    ui->colorDataTreeView->setModel( m_color_data_model );

    connect( ui->direction1ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );
    connect( ui->direction2ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );
    connect( ui->direction3ComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &GlyphEditor::onDirectionComboBoxIndexChanged );

    connect( ui->sizeConstantRadioButton, &QRadioButton::clicked, this, &GlyphEditor::onSizeConstantRadioButtonClicked );
    connect( ui->sizeVariableArrayRadioButton, &QRadioButton::clicked, this, &GlyphEditor::onSizeVariablesRadioButtonClicked );
    connect( ui->sizeNumberOfVariablesSpinBox, &QSpinBox::valueChanged, this, &GlyphEditor::onSizeNumberOfVariableValueChanged );

    connect( ui->editColorMapPushButton, &QPushButton::clicked, this, &GlyphEditor::onEditColorMap );

    connect( ui->colorDataConstantRadioButton, &QRadioButton::clicked, this, &GlyphEditor::onColorDataConstantRadioButtonClicked );
    connect( ui->colorDataVariableArrayRadioButton, &QRadioButton::clicked, this, &GlyphEditor::onColorDataVariablesRadioButtonClicked );
    connect( ui->colorDataNumberOfVariablesSpinBox, &QSpinBox::valueChanged, this, &GlyphEditor::onColorDataNumberOfVariableValueChanged );

    connect( ui->applyPushButton, &QPushButton::clicked, this, &GlyphEditor::onApply );
}

void GlyphEditor::onDirectionComboBoxIndexChanged(int /*index*/)
{
    QStringList selectedList;
    selectedList << ui->direction1ComboBox->currentText()
                 << ui->direction2ComboBox->currentText()
                 << ui->direction3ComboBox->currentText();

    QList<QComboBox*> comboBoxes =
        {
            ui->direction1ComboBox,
            ui->direction2ComboBox,
            ui->direction3ComboBox
        };

    QSet<QString> used;
    for( QComboBox* combo : comboBoxes )
    {
        QString value = combo->currentText();
        if( !value.isEmpty() && used.contains( value ) )
        {
            // 重複していたら別の未使用の値を探して設定
            for( const QString& candidate : m_vector_list )
            {
                if( !used.contains( candidate ) )
                {
                    combo->blockSignals( true );
                    int index = combo->findText( candidate);
                    if (index != -1)
                        combo->setCurrentIndex(index);
                    combo->blockSignals(false);
                    used.insert(candidate);
                    break;
                }
            }
        }
        else
        {
            if (!value.isEmpty())
                used.insert(value);
        }
    }
}

void GlyphEditor::onSizeConstantRadioButtonClicked()
{
    if( ui->sizeConstantRadioButton->isChecked() )
    {
        ui->sizeNumberOfVariablesSpinBox->setDisabled( true );
        ui->sizeTreeView->setDisabled( true );
    }
}

void GlyphEditor::onSizeVariablesRadioButtonClicked()
{
    if( ui->sizeVariableArrayRadioButton->isChecked() )
    {
        ui->sizeNumberOfVariablesSpinBox->setEnabled( true );
        ui->sizeTreeView->setEnabled( true );
    }
}

void GlyphEditor::onSizeNumberOfVariableValueChanged()
{
    int numberOfVariables = ui->sizeNumberOfVariablesSpinBox->value();

    // 現在の選択値を保存
    QStringList previousSelections;
    for( int row = 0; row < m_size_model->rowCount(); row++ )
    {
        QModelIndex index = m_size_model->index( row, 1 ); // "Scale"列
        previousSelections << index.data().toString();
    }

    m_size_model->removeRows( 0, m_size_model->rowCount() );

    ui->sizeTreeView->setItemDelegateForColumn( 1, new ComboBoxDelegate( m_vector_list, this ) );

    for( int i = 0; i < numberOfVariables; i++ )
    {
        QStandardItem* item1 = new QStandardItem( QString( "Variable %1" ).arg( i + 1 ) );
        item1->setFlags( item1->flags() & ~Qt::ItemIsEditable );  // 編集不可に設定

        QString previousValue;
        if( i < previousSelections.size() )
        {
            previousValue = previousSelections[i]; // 以前の値を復元
        }
        else
        {
            previousValue = m_vector_list.value( i ); // 初期値として m_vector_list から取得
        }

        QStandardItem* item2 = new QStandardItem( previousValue );
        m_size_model->appendRow( QList<QStandardItem*>() << item1 << item2 );
    }
}

void GlyphEditor::onColorDataConstantRadioButtonClicked()
{
    if( ui->colorDataConstantRadioButton->isChecked() )
    {
        ui->colorDataNumberOfVariablesSpinBox->setDisabled( true );
        ui->colorDataTreeView->setDisabled( true );
    }
}

void GlyphEditor::onColorDataVariablesRadioButtonClicked()
{
    if( ui->colorDataVariableArrayRadioButton->isChecked() )
    {
        ui->colorDataNumberOfVariablesSpinBox->setEnabled( true );
        ui->colorDataTreeView->setEnabled( true );
    }
}

void GlyphEditor::onEditColorMap()
{
    ColorMapEditor colorMapEditor;
    colorMapEditor.adjustSize();
    colorMapEditor.setDefaultColorMap( ui->colorMap->getColors() );

    if( colorMapEditor.exec() == QDialog::Accepted )
    {
        ui->colorMap->setColors( colorMapEditor.getColorMap() );
    }
}

void GlyphEditor::onColorDataNumberOfVariableValueChanged()
{
    int numberOfVariables = ui->colorDataNumberOfVariablesSpinBox->value();

    // 現在の選択値を保存
    QStringList previousSelections;
    for( int row = 0; row < m_color_data_model->rowCount(); row++ )
    {
        QModelIndex index = m_color_data_model->index( row, 1 ); // "Scale"列
        previousSelections << index.data().toString();
    }

    m_color_data_model->removeRows( 0, m_color_data_model->rowCount() );

    ui->colorDataTreeView->setItemDelegateForColumn( 1, new ComboBoxDelegate( m_vector_list, this ) );

    for( int i = 0; i < numberOfVariables; i++ )
    {
        QStandardItem* item1 = new QStandardItem( QString( "Variable %1" ).arg( i + 1 ) );
        item1->setFlags( item1->flags() & ~Qt::ItemIsEditable );  // 編集不可に設定

        QString previousValue;
        if( i < previousSelections.size() )
        {
            previousValue = previousSelections[i]; // 以前の値を復元
        }
        else
        {
            previousValue = m_vector_list.value( i ); // 初期値として m_vector_list から取得
        }

        QStandardItem* item2 = new QStandardItem( previousValue );
        m_color_data_model->appendRow( QList<QStandardItem*>() << item1 << item2 );
    }
}

void GlyphEditor::onApply()
{
    m_model->clear();

    auto* rootItem = new QStandardItem("GlyphParams");

    // 基本パラメータ設定
    rootItem->setData(static_cast<GlyphItem::GlyphType>(ui->glyphTypeComboBox->currentIndex()), GlyphItem::UserGlyphType);
    rootItem->setData(ui->scaleFactorDoubleSpinBox->value(), GlyphItem::ScaleFactor);
    rootItem->setData(QStringList{
                          ui->direction1ComboBox->currentText(),
                          ui->direction2ComboBox->currentText(),
                          ui->direction3ComboBox->currentText()
                      }, GlyphItem::Direction);
    rootItem->setData(ui->sizeConstantRadioButton->isChecked() ? "Constant" : "Variable", GlyphItem::SizeDataDefines);

    QStringList sizeVars;
    for (int i = 0; i < m_size_model->rowCount(); ++i)
        sizeVars << m_size_model->item(i, 1)->text();
    rootItem->setData(sizeVars, GlyphItem::SizeVariables);

    int distributionMode = -1;
    if (ui->uniformDistributionRadioButton->isChecked()) distributionMode = GlyphItem::UniformDistribution;
    else if (ui->allPointsRadioButton->isChecked()) distributionMode = GlyphItem::AllPoints;
    else if (ui->everyNthPointRadioButton->isChecked()) distributionMode = GlyphItem::EveryNthPoints;
    rootItem->setData(distributionMode, GlyphItem::DistributionMode);

    rootItem->setData(ui->numberOfSamplePointsSpinBox->value(), GlyphItem::NumberOfSamplePoints);
    rootItem->setData(ui->seedSpinBox->value(), GlyphItem::Seed);
    rootItem->setData(ui->strideSpinBox->value(), GlyphItem::Stride);

    QVector<QColor> colors = ui->colorMap->getColors();
    rootItem->setData(QVariant::fromValue(colors), GlyphItem::ColorMap);

    rootItem->setData(ui->colorDataConstantRadioButton->isChecked() ? "Constant" : "Variable", GlyphItem::ColorDataDefines);

    QStringList colorVars;
    for (int i = 0; i < m_color_data_model->rowCount(); ++i)
        colorVars << m_color_data_model->item(i, 1)->text();
    rootItem->setData(colorVars, GlyphItem::ColorVariables);

    m_model->appendRow(rootItem);

    // ====== JSON 生成 & 送信 ======
    QJsonObject root;
    root["event"] = "glyph";

    QJsonObject params;
    params["glyphType"] = rootItem->data(GlyphItem::UserGlyphType).toInt();
    params["scaleFactor"] = rootItem->data(GlyphItem::ScaleFactor).toDouble();

    QJsonArray directionArray;
    for (const QString& dir : rootItem->data(GlyphItem::Direction).toStringList())
        directionArray.append(dir);
    params["direction"] = directionArray;

    params["sizeDataDefines"] = rootItem->data(GlyphItem::SizeDataDefines).toString();

    QJsonArray sizeVarArray;
    for (const QString& s : rootItem->data(GlyphItem::SizeVariables).toStringList())
        sizeVarArray.append(s);
    params["sizeVariables"] = sizeVarArray;

    params["distributionMode"] = rootItem->data(GlyphItem::DistributionMode).toInt();
    params["numberOfSamplePoints"] = rootItem->data(GlyphItem::NumberOfSamplePoints).toInt();
    params["seed"] = rootItem->data(GlyphItem::Seed).toInt();
    params["stride"] = rootItem->data(GlyphItem::Stride).toInt();

    QJsonArray colorArray;
    for (const QColor& c : colors)
    {
        QJsonObject colorObj;
        colorObj["r"] = c.red();
        colorObj["g"] = c.green();
        colorObj["b"] = c.blue();
        colorObj["a"] = c.alpha();
        colorArray.append(colorObj);
    }
    params["colorMap"] = colorArray;

    params["colorDataDefines"] = rootItem->data(GlyphItem::ColorDataDefines).toString();

    QJsonArray colorVarArray;
    for (const QString& s : rootItem->data(GlyphItem::ColorVariables).toStringList())
        colorVarArray.append(s);
    params["colorVariables"] = colorVarArray;

    root["params"] = params;

    m_web_text_socket->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

void GlyphEditor::updateNumberOfVector( const int numberOfVector )
{
    m_number_of_vector = numberOfVector;
    if( m_number_of_vector < 3 ) // 成分数が3未満である場合はこのパネルは使用できない。
    {

    }
    else
    {

    }

    m_vector_list.clear();
    for( int i = 0; i < m_number_of_vector; i++ )
    {
        m_vector_list << QString( "q%1" ).arg( i + 1 );
    }

    ui->direction1ComboBox->clear();
    ui->direction2ComboBox->clear();
    ui->direction3ComboBox->clear();
    ui->direction1ComboBox->addItems( m_vector_list );
    ui->direction2ComboBox->addItems( m_vector_list );
    ui->direction3ComboBox->addItems( m_vector_list );

    ui->sizeTreeView->setItemDelegateForColumn( 1, new ComboBoxDelegate( m_vector_list, this ) );
    ui->colorDataTreeView->setItemDelegateForColumn( 1, new ComboBoxDelegate( m_vector_list, this ) );

    ui->sizeNumberOfVariablesSpinBox->setMaximum( m_number_of_vector );
    ui->colorDataNumberOfVariablesSpinBox->setMaximum( m_number_of_vector );
}

void GlyphEditor::updateOperatorState( bool operatorState )
{
    if( m_color_map_editor.isVisible() ) m_color_map_editor.close();

    ui->glyphTypeComboBox->setEnabled( operatorState );
    ui->scaleFactorDoubleSpinBox->setEnabled( operatorState );
    ui->direction1ComboBox->setEnabled( operatorState );
    ui->direction2ComboBox->setEnabled( operatorState );
    ui->direction3ComboBox->setEnabled( operatorState );
    ui->sizeConstantRadioButton->setEnabled( operatorState );
    ui->sizeVariableArrayRadioButton->setEnabled( operatorState );
    ui->sizeNumberOfVariablesSpinBox->setEnabled( operatorState );

    ui->uniformDistributionRadioButton->setEnabled( operatorState );
    ui->allPointsRadioButton->setEnabled( operatorState );
    ui->everyNthPointRadioButton->setEnabled( operatorState );

    ui->numberOfSamplePointsSpinBox->setEnabled( operatorState );
    ui->seedSpinBox->setEnabled( operatorState );
    ui->strideSpinBox->setEnabled( operatorState );

    ui->editColorMapPushButton->setEnabled( operatorState );

    ui->colorDataConstantRadioButton->setEnabled( operatorState );
    ui->colorDataVariableArrayRadioButton->setEnabled( operatorState );
    ui->colorDataNumberOfVariablesSpinBox->setEnabled( operatorState );

    ui->applyPushButton->setEnabled( operatorState );
}

void GlyphEditor::reset()
{
    // ui->numberOfTransferFunctionLabelSpinBox->setValue( 0 );
    // QVector<QColor> initialColors( 256, QColor( 0, 0, 0 ) );
    // ui->colorMapBar->setColors( initialColors );
    // QVector<float> initialOpacity( 256, 0.0f );
    // ui->opacityMapBar->setOpacities( initialOpacity );
    // std::vector<int> initialHistogram( 256, 0 );
    // ui->colorHistogram->setDatas( initialHistogram );
    // ui->opacityHistogram->setDatas( initialHistogram );
}

void GlyphEditor::saveParameter()
{
    // TODO KPIセーブ/ロード機能用
}

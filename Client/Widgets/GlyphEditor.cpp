#include "GlyphEditor.h"
#include "ui_GlyphEditor.h"

#include "Widgets/MergePanel.h"
#include "Widgets/Connect.h"

GlyphEditor::GlyphEditor(QWidget *parent, MergePanel* merge, Connect* connect_panel)
    : QDockWidget(parent)
    , ui(new Ui::GlyphEditor),
    m_merge( merge ),
    m_connect( connect_panel ),
    m_vector_list( new QStringList() )
{
    ui->setupUi(this);

    //初期値設定
    QStringList types;
    types << "Arrow" << "Diamond" << "Sphere";
    ui->glyphTypeComboBox->addItems( types );

    ui->scaleFactorDoubleSpinBox->setValue( 1.0 );

    ui->sizeConstantRadioBox->setChecked( true );
    ui->uniformDistributionRadioButton->setChecked( true );
    ui->numberOfSamplePoints->setValue( 1000 );
    ui->seedSpinBox->setValue( 1 );
    ui->strideSpinBox->setValue( 3 );
    ui->colorDataConstantRadioBox->setChecked( true );

    // スクロールエリア用のウィジェットを作成
    QWidget *sizeScrollContentWidget = new QWidget(this);
    m_size_variable_layout = new QVBoxLayout(sizeScrollContentWidget);
    ui->sizeScrollArea->setWidget(sizeScrollContentWidget);
    ui->sizeScrollArea->setWidgetResizable(true);  // スクロールエリアのコンテンツを自動リサイズする

    QWidget *colorDataScrollContentWidget = new QWidget(this);
    m_color_data_variable_layout = new QVBoxLayout(colorDataScrollContentWidget);
    ui->colorDataScrollArea->setWidget(colorDataScrollContentWidget);
    ui->colorDataScrollArea->setWidgetResizable(true);  // スクロールエリアのコンテンツを自動リサイズする

    // // directionComboBox のインデックスが重複しないようにする
    connect(ui->direction1ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GlyphEditor::onDirectionComboBoxIndexChanged);
    connect(ui->direction2ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GlyphEditor::onDirectionComboBoxIndexChanged);
    connect(ui->direction3ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GlyphEditor::onDirectionComboBoxIndexChanged);

    connect( ui->sizeNumberOfVariableSpinBox, &QSpinBox::valueChanged, this, &GlyphEditor::onSizeNumberOfVariableChanged );
    connect( ui->colorDataNumberOfVariableSpinBox, &QSpinBox::valueChanged, this, &GlyphEditor::onColorDataNumberOfVariableChanged );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &GlyphEditor::onApplyButtonClicked );
    this->setEnabled( false );
}

GlyphEditor::~GlyphEditor()
{
    delete m_vector_list;
    delete ui;
}

void GlyphEditor::updateNumberOfVector( jpv::ParticleTransferServerMessage& server_message )
{
    const int numberOfVector = server_message.m_number_ingredients;

    if( numberOfVector < 3 ) //成分数が3未満である場合、このパネルを操作不能にする。
    {
        this->setEnabled( false );
    }
    else
    {
        this->setEnabled( true );
    }

    ui->sizeNumberOfVariableSpinBox->setMaximum( numberOfVector );
    ui->colorDataNumberOfVariableSpinBox->setMaximum( numberOfVector );

    m_vector_list->clear();
    for( int i = 1; i <= numberOfVector; i++ )
    {
        m_vector_list->append(QString("q%1").arg(i));
    }

    {
        directionComboBoxBlockSignals( true );

        //directionComboBoxにアイテムを登録する。
        ui->direction1ComboBox->addItems( *m_vector_list );
        ui->direction2ComboBox->addItems( *m_vector_list );
        ui->direction3ComboBox->addItems( *m_vector_list );
        //デフォルトq1,q2,q3となるようにインデックスを設定する。
        ui->direction1ComboBox->setCurrentIndex( 0 );
        ui->direction2ComboBox->setCurrentIndex( 1 );
        ui->direction3ComboBox->setCurrentIndex( 2 );

        m_direction_previus_index[0] = ui->direction1ComboBox->currentIndex();
        m_direction_previus_index[1] = ui->direction2ComboBox->currentIndex();
        m_direction_previus_index[2] = ui->direction3ComboBox->currentIndex();

        directionComboBoxBlockSignals( false );
    }
}

//Directionコンボボックスの重複防止用メソッド
void GlyphEditor::onDirectionComboBoxIndexChanged(int index)
{
    int selectedIndex[3];
    selectedIndex[0] = ui->direction1ComboBox->currentIndex();
    selectedIndex[1] = ui->direction2ComboBox->currentIndex();
    selectedIndex[2] = ui->direction3ComboBox->currentIndex();

    // インデックスが被っていないか確認
    if ( selectedIndex[0] == selectedIndex[1] ||
         selectedIndex[0] == selectedIndex[2] ||
         selectedIndex[1] == selectedIndex[2] ) { // 被っている場合の処理
        directionComboBoxBlockSignals( true );
        if( m_direction_previus_index[0] != selectedIndex[0] )
        {
            ui->direction1ComboBox->setCurrentIndex( m_direction_previus_index[0] );
        }
        if( m_direction_previus_index[1] != selectedIndex[1] )
        {
            ui->direction2ComboBox->setCurrentIndex( m_direction_previus_index[1] );
        }
        if( m_direction_previus_index[2] != selectedIndex[2] )
        {
            ui->direction3ComboBox->setCurrentIndex( m_direction_previus_index[2] );
        }
        directionComboBoxBlockSignals( false );
    } else { // 被っていない場合の処理
        if( m_direction_previus_index[0] != selectedIndex[0] )
        {
            m_direction_previus_index[0] = selectedIndex[0];
        }
        if( m_direction_previus_index[1] != selectedIndex[1] )
        {
            m_direction_previus_index[1] = selectedIndex[1];
        }
        if( m_direction_previus_index[2] != selectedIndex[2] )
        {
            m_direction_previus_index[2] = selectedIndex[2];
        }
    }
}

void GlyphEditor::onSizeNumberOfVariableChanged(int value)
{
    int currentCount = m_size_variable_labels.size();

    if (value > currentCount) {
        // 必要な分だけラベルとコンボボックスを追加
        for (int i = currentCount; i < value; ++i) {
            // 水平レイアウトを作成
            QHBoxLayout *hLayout = new QHBoxLayout();

            // QLabelの作成とレイアウトへの追加
            QLabel *label = new QLabel(tr("Variable %1").arg(i + 1), this);
            m_size_variable_labels.append(label);
            hLayout->addWidget(label);

            // QComboBoxの作成とレイアウトへの追加
            QComboBox *comboBox = new QComboBox(this);
            m_size_variable_combo_boxes.append(comboBox);
            hLayout->addWidget(comboBox);

            // // コンボボックスにアイテムを追加
            comboBox->addItems( *m_vector_list );
            comboBox->setCurrentIndex( i );

            // 水平レイアウトをメインの垂直レイアウトに追加
            m_size_variable_layout->addLayout(hLayout);
        }
    } else if (value < currentCount) {
        // 余分なラベルとコンボボックスを削除
        for (int i = currentCount - 1; i >= value; --i) {
            QLabel *label = m_size_variable_labels.takeLast();
            m_size_variable_layout->removeWidget(label);
            delete label;

            QComboBox *comboBox = m_size_variable_combo_boxes.takeLast();
            m_size_variable_layout->removeWidget(comboBox);
            delete comboBox;
        }
    }

    // レイアウトを再調整
    m_size_variable_layout->update();
}

void GlyphEditor::onColorDataNumberOfVariableChanged(int value)
{
    int currentCount = m_color_data_variable_labels.size();

    if (value > currentCount) {
        // 必要な分だけラベルとコンボボックスを追加
        for (int i = currentCount; i < value; ++i) {
            // 水平レイアウトを作成
            QHBoxLayout *hLayout = new QHBoxLayout();

            // QLabelの作成とレイアウトへの追加
            QLabel *label = new QLabel(tr("Variable %1").arg(i + 1), this);
            m_color_data_variable_labels.append(label);
            hLayout->addWidget(label);

            // QComboBoxの作成とレイアウトへの追加
            QComboBox *comboBox = new QComboBox(this);
            m_color_data_variable_combo_boxes.append(comboBox);
            hLayout->addWidget(comboBox);

            // // コンボボックスにアイテムを追加
            comboBox->addItems( *m_vector_list );
            comboBox->setCurrentIndex( i );

            // 水平レイアウトをメインの垂直レイアウトに追加
            m_color_data_variable_layout->addLayout(hLayout);
        }
    } else if (value < currentCount) {
        // 余分なラベルとコンボボックスを削除
        for (int i = currentCount - 1; i >= value; --i) {
            QLabel *label = m_color_data_variable_labels.takeLast();
            m_color_data_variable_layout->removeWidget(label);
            delete label;

            QComboBox *comboBox = m_color_data_variable_combo_boxes.takeLast();
            m_color_data_variable_layout->removeWidget(comboBox);
            delete comboBox;
        }
    }

    // レイアウトを再調整
    m_color_data_variable_layout->update();
}

void GlyphEditor::onApplyButtonClicked()
{
    const int numberOfVector = 5;

    if( numberOfVector < 3 ) //成分数が3未満である場合、このパネルを操作不能にする。
    {
        this->setEnabled( false );
    }
    else
    {
        this->setEnabled( true );
    }

    ui->sizeNumberOfVariableSpinBox->setMaximum( numberOfVector );
    ui->colorDataNumberOfVariableSpinBox->setMaximum( numberOfVector );

    m_vector_list->clear();
    for( int i = 1; i <= numberOfVector; i++ )
    {
        m_vector_list->append(QString("q%1").arg(i));
    }

    {
        directionComboBoxBlockSignals( true );

        //directionComboBoxにアイテムを登録する。
        ui->direction1ComboBox->addItems( *m_vector_list );
        ui->direction2ComboBox->addItems( *m_vector_list );
        ui->direction3ComboBox->addItems( *m_vector_list );
        //デフォルトq1,q2,q3となるようにインデックスを設定する。
        ui->direction1ComboBox->setCurrentIndex( 0 );
        ui->direction2ComboBox->setCurrentIndex( 1 );
        ui->direction3ComboBox->setCurrentIndex( 2 );

        m_direction_previus_index[0] = ui->direction1ComboBox->currentIndex();
        m_direction_previus_index[1] = ui->direction2ComboBox->currentIndex();
        m_direction_previus_index[2] = ui->direction3ComboBox->currentIndex();

        directionComboBoxBlockSignals( false );
    }

    // ui->direction1ComboBox->addItems()


    // m_connect->getClientMessage()->m_directions1 = ui->directionLineEdit1->text().toStdString();
    // m_connect->getClientMessage()->m_directions2 = ui->directionLineEdit2->text().toStdString();
    // m_connect->getClientMessage()->m_directions3 = ui->directionLineEdit3->text().toStdString();

    // if (ui->sizeConstantRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_size_defines = DataDefines::Constant; //0
    // }
    // else if (ui->sizeSingleVariableRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_size_defines = DataDefines::SingleVariable; //1
    // }
    // else if (ui->sizeVariablesArrayRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_size_defines = DataDefines::VariableArray; //2
    // }
    // m_connect->getClientMessage()->m_size_variable1 = ui->sizeVariable1; //numberに変換する必要がありそう。
    // m_connect->getClientMessage()->m_size_variable2 = ui->sizeVariable2; //numberに変換する必要がありそう。

    // m_connect->getClientMessage()->m_scale_factor = ui->scaleFactorDoubleSpinBox->value(); //グリフ生成に必要なもの?? 表示に必要なもの??

    // if (ui->uniformDistributionRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_distribution = GlyphMode::UniformDistribution; //0
    // }
    // else if (ui->allPointsRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_distribution = GlyphMode::AllPoints; //1
    // }
    // else if (ui->everyNthPointsRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_distribution = GlyphMode::EveryNthPoints; //2
    // }

    // m_connect->getClientMessage()->m_glyph_color_map = ui->openGLWidget->getColor();

    // if (ui->colorConstantRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_color_defines = DataDefines::Constant; //0
    // }
    // else if (ui->colorSingleVariableRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_color_defines = DataDefines::SingleVariable; //1
    // }
    // else if (ui->colorVariablesArrayRadioButton->isChecked()) {
    //     m_connect->getClientMessage()->m_color_defines = DataDefines::VariableArray; //2
    // }
    // m_connect->getClientMessage()->m_color_variable1 = ui->colorVariable1; //numberに変換する必要がありそう。
    // m_connect->getClientMessage()->m_color_variable2 = ui->colorVariable2; //numberに変換する必要がありそう。
}

void GlyphEditor::directionComboBoxBlockSignals( bool block )
{
    ui->direction1ComboBox->blockSignals( block );
    ui->direction2ComboBox->blockSignals( block );
    ui->direction3ComboBox->blockSignals( block );
}

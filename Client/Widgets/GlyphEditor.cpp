#include "GlyphEditor.h"
#include "ui_GlyphEditor.h"

#include "Widgets/MergePanel.h"
#include "Widgets/Connect.h"

GlyphEditor::GlyphEditor(QWidget *parent, MergePanel* merge, Connect* connect_panel)
    : QDockWidget(parent)
    , ui(new Ui::GlyphEditor),
    m_merge( merge ),
    m_connect( connect_panel )
{
    ui->setupUi(this);

    // QStringList types;
    // types << "Arrow" << "Diamond" << "Sphere";

    // スクロールエリア用のウィジェットを作成
    QWidget *sizeScrollContentWidget = new QWidget(this);
    m_size_variable_layout = new QVBoxLayout(sizeScrollContentWidget);
    ui->sizeScrollArea->setWidget(sizeScrollContentWidget);
    ui->sizeScrollArea->setWidgetResizable(true);  // スクロールエリアのコンテンツを自動リサイズする

    QWidget *colorDataScrollContentWidget = new QWidget(this);
    m_color_data_variable_layout = new QVBoxLayout(colorDataScrollContentWidget);
    ui->colorDataScrollArea->setWidget(colorDataScrollContentWidget);
    ui->colorDataScrollArea->setWidgetResizable(true);  // スクロールエリアのコンテンツを自動リサイズする

    connect( ui->sizeNumberOfVariableSpinBox, &QSpinBox::valueChanged, this, &GlyphEditor::onSizeNumberOfVariableChanged );
    connect( ui->colorDataNumberOfVariableSpinBox, &QSpinBox::valueChanged, this, &GlyphEditor::onColorDataNumberOfVariableChanged );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &GlyphEditor::onApplyButtonClicked );
}

GlyphEditor::~GlyphEditor()
{
    delete ui;
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
            m_size_variable_comboboxes.append(comboBox);
            hLayout->addWidget(comboBox);

            // コンボボックスにアイテムを追加
            comboBox->addItem("Option 1");
            comboBox->addItem("Option 2");

            // 水平レイアウトをメインの垂直レイアウトに追加
            m_size_variable_layout->addLayout(hLayout);
        }
    } else if (value < currentCount) {
        // 余分なラベルとコンボボックスを削除
        for (int i = currentCount - 1; i >= value; --i) {
            QLabel *label = m_size_variable_labels.takeLast();
            m_size_variable_layout->removeWidget(label);
            delete label;

            QComboBox *comboBox = m_size_variable_comboboxes.takeLast();
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
            m_color_data_variable_comboboxes.append(comboBox);
            hLayout->addWidget(comboBox);

            // コンボボックスにアイテムを追加
            comboBox->addItem("Option 1");
            comboBox->addItem("Option 2");

            // 水平レイアウトをメインの垂直レイアウトに追加
            m_color_data_variable_layout->addLayout(hLayout);
        }
    } else if (value < currentCount) {
        // 余分なラベルとコンボボックスを削除
        for (int i = currentCount - 1; i >= value; --i) {
            QLabel *label = m_color_data_variable_labels.takeLast();
            m_color_data_variable_layout->removeWidget(label);
            delete label;

            QComboBox *comboBox = m_color_data_variable_comboboxes.takeLast();
            m_color_data_variable_layout->removeWidget(comboBox);
            delete comboBox;
        }
    }

    // レイアウトを再調整
    m_color_data_variable_layout->update();
}

void GlyphEditor::onApplyButtonClicked()
{


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

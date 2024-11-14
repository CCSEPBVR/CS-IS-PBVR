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

    QStringList types;
    types << "Arrow" << "Diamond" << "Sphere";
    ui->glyphTypeComboBox->addItems( types );

    connect( ui->applyPushButton, &QPushButton::clicked, this, &GlyphEditor::onApplyButtonClicked );
}

GlyphEditor::~GlyphEditor()
{
    delete ui;
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

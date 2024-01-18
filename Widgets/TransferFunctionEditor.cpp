#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"
#include <kvs/ColorMapBar>
#include <kvs/OpacityMapBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferFunctionEditor)
{
    ui->setupUi(this);
    connect( ui->editColorMapPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onColorMapEditorButtonClicked );
    connect( ui->editOpacityMapPBtn, &QPushButton::clicked, this, &TransferFunctionEditor::onOpacityMapEditorButtonClicked );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
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

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
    connect( ui->pushButton_7, &QPushButton::clicked, this, &TransferFunctionEditor::onColorMapEditorButtonClicked );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}

void TransferFunctionEditor::onColorMapEditorButtonClicked()
{
    if( m_color_map_editor.exec() == QDialog::Accepted )
    {
        ui->openGLWidget->setColorMap(m_color_map_editor.getColorMap());
    }
}



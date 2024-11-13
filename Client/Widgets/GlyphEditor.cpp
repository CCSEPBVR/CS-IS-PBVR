#include "GlyphEditor.h"
#include "ui_GlyphEditor.h"

GlyphEditor::GlyphEditor(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::GlyphEditor)
{
    ui->setupUi(this);
}

GlyphEditor::~GlyphEditor()
{
    delete ui;
}

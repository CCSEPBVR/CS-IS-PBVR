#include "GlyphEditor.h"
#include "ui_GlyphEditor.h"

GlyphEditor::GlyphEditor( QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::GlyphEditor )
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
}

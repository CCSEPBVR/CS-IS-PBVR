#include "RepetitionLevelControl.h"
#include "ui_RepetitionLevelControl.h"

RepetitionLevelControl::RepetitionLevelControl( QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::RepetitionLevelControl )
{
    initialize();
}

RepetitionLevelControl::~RepetitionLevelControl()
{
    delete ui;
}

void RepetitionLevelControl::initialize()
{
    ui->setupUi( this );
}

#include "MCPWidget.h"
#include "ui_MCPWidget.h"
#include <QCloseEvent>
#include <QDateTime>

MCPWidget::MCPWidget( QWidget* parent ) : QDockWidget( parent ), ui( new Ui::MCPWidget )
{
    ui->setupUi( this );
    setAttribute( Qt::WA_DeleteOnClose, false );
}

MCPWidget::~MCPWidget() { delete ui; }

void MCPWidget::appendEntry( const QString& kind, const QString& text )
{
    // One list item per command, whole JSON response, or error (not per line).
    ui->entries->addItem( QDateTime::currentDateTime().toString( "HH:mm:ss.zzz" ) +
                         " [" + kind + "] " + text );
    while( ui->entries->count() > 10 ) delete ui->entries->takeItem( 0 );
    ui->entries->scrollToBottom();
}

void MCPWidget::closeEvent( QCloseEvent* event )
{
    hide();
    event->accept();
}

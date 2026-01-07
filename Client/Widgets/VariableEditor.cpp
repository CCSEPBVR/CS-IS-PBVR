#include "VariableEditor.h"
#include "ui_VariableEditor.h"

VariableEditor::VariableEditor(QWidget *parent)
    : QDialog( parent )
    , ui (new Ui::VariableEditor )
    , m_model( new QStandardItemModel( this ) )
    , m_type( COLOR )
    , m_transfer_function( nullptr )
{
    ui->setupUi( this );

    ui->treeView->setModel( m_model );
    ui->treeView->setEditTriggers( QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked );

    connect( ui->cancelPushButton, &QPushButton::clicked, this, &VariableEditor::close );
    connect( ui->OKPushButton, &QPushButton::clicked, this, &VariableEditor::accept );
}

VariableEditor::~VariableEditor()
{
    delete ui;
}

void VariableEditor::setup( TYPE type, TransferFunction* transferFunction )
{
    m_type = type;
    m_transfer_function = transferFunction;
}

void VariableEditor::initialize()
{
    if( !m_transfer_function ) return;

    m_model->clear();
    m_model->setColumnCount( 2 );
    m_model->setHorizontalHeaderLabels( QStringList() << "Name" << "Variable" );

    const size_t count = m_transfer_function->count();

    for( size_t i = 0; i < count; ++i )
    {
        QString name;
        QString variable;

        if( m_type == COLOR )
        {
            name = QString( "C%1" ).arg( i + 1 );
            variable = QString::fromUtf8( m_transfer_function->at(i).color.variable );
        }
        else if( m_type == OPACITY )
        {
            name = QString( "O%1" ).arg( i + 1 );
            variable = QString::fromUtf8( m_transfer_function->at(i).opacity.variable );
        }

        QStandardItem* nameItem = new QStandardItem( name );
        nameItem->setEditable( false ); // Name列は編集不可

        QStandardItem* variableItem = new QStandardItem( variable );
        variableItem->setEditable( true ); // Variable列は編集可能

        QList<QStandardItem*> row;
        row << nameItem << variableItem;

        m_model->appendRow( row );
    }

    ui->treeView->resizeColumnToContents( 0 );
}

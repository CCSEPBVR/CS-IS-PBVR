#include "VariableEditor.h"
#include "ui_VariableEditor.h"

VariableEditor::VariableEditor( TYPE type, const QStandardItemModel& model, QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::VariableEditor )
    , m_type( type )
    , m_original_model( model )
{
    initialize();
}

VariableEditor::~VariableEditor()
{
    delete ui;
}

void VariableEditor::initialize()
{
    ui->setupUi( this );

    m_editable_mode.setColumnCount( 2 );
    m_editable_mode.setHorizontalHeaderLabels( QStringList() << "Function" << "f（algebraic formula）" );

    // 元データから必要情報だけをコピー（読み取り）
    for( int row = 0; row < m_original_model.rowCount(); row++ )
    {
        const QStandardItem* item = m_original_model.item( row );
        QString funcStr;
        QString varStr;

        switch ( m_type )
        {
        case TYPE::COLOR:
            funcStr = item->data( TransferFunctionItem::ColorFunction ).toString();
            varStr = item->data( TransferFunctionItem::ColorVariable ).toString();
            break;
        case TYPE::OPACITY:
            funcStr = item->data( TransferFunctionItem::OpacityFunction ).toString();
            varStr = item->data( TransferFunctionItem::OpacityVariable ).toString();
            break;
        default:
            break;
        }

        QStandardItem* funcItem = new QStandardItem( funcStr );
        funcItem->setFlags( funcItem->flags() & ~Qt::ItemIsEditable );  // 編集不可にする

        QStandardItem* varItem = new QStandardItem( varStr );

        QList<QStandardItem*> newRow;
        newRow << funcItem << varItem;

        m_editable_mode.appendRow( newRow );
    }

    ui->treeView->setModel( &m_editable_mode );
    ui->treeView->setEditTriggers( QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked );

    connect( ui->cancelPushButton, &QPushButton::clicked, this, &VariableEditor::close );
    connect( ui->OKPushButton, &QPushButton::clicked, this, &VariableEditor::accept );
}

#pragma once
#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include <QLabel>
#include <QFileInfo>

#include "WebSocketPair.h"

class RemoteFileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RemoteFileDialog( WebSocketPair* websockets, QWidget* parent = nullptr )
        : QDialog( parent )
        , m_web_sockets( websockets )
    {
        setWindowTitle( "Select 3D data files" );
        resize( 600, 500 );

        // UI構成
        m_pathLabel = new QLabel( "/", this );
        m_treeView  = new QTreeView( this );
        m_model     = new QStandardItemModel( this );
        m_model->setHorizontalHeaderLabels( { "Name", "Type" } );
        m_treeView->setModel( m_model );
        m_treeView->header()->setSectionResizeMode( QHeaderView::Stretch );
        m_treeView->setSelectionBehavior( QAbstractItemView::SelectRows );

        // ページング
        m_prevButton    = new QPushButton( "<- Previous", this );
        m_nextButton    = new QPushButton( "Next ->", this );
        m_pageLabel     = new QLabel( "Page: 1", this );
        QHBoxLayout* pageLayout = new QHBoxLayout;
        pageLayout->addWidget( m_prevButton );
        pageLayout->addWidget( m_pageLabel );
        pageLayout->addWidget( m_nextButton );
        pageLayout->addStretch();

        // OK/Cancel
        QPushButton* okButton       = new QPushButton( "OK", this );
        QPushButton* cancelButton   = new QPushButton( "Cancel", this );
        QHBoxLayout* btnLayout      = new QHBoxLayout;
        btnLayout->addStretch();
        btnLayout->addWidget( okButton );
        btnLayout->addWidget( cancelButton );

        // 全体レイアウト
        QVBoxLayout* layout = new QVBoxLayout( this );
        layout->addWidget( m_pathLabel );
        layout->addWidget( m_treeView );
        layout->addLayout( pageLayout );
        layout->addLayout( btnLayout );
        setLayout( layout );

        connect( m_treeView, &QTreeView::doubleClicked, this, &RemoteFileDialog::onDoubleClicked );
        connect( m_web_sockets->text(), &QWebSocket::textMessageReceived, this, &RemoteFileDialog::onMessageReceived );

        connect( m_prevButton, &QPushButton::clicked, this, [this]()
                {
                    if( m_currentPage > 1 )
                    {
                        m_currentPage--;
                        requestDir( m_currentPath, m_currentPage );
                    }
                }
                );
        connect( m_nextButton, &QPushButton::clicked, this, [this]()
                {
                    m_currentPage++;
                    requestDir( m_currentPath, m_currentPage );
                }
                );

        connect( okButton, &QPushButton::clicked, this, &RemoteFileDialog::onOk );
        connect( cancelButton, &QPushButton::clicked, this, &RemoteFileDialog::reject );

        requestDir( "/", 1 );
    }

    QString selectedFile() const { return m_selectedFile; }

private:
    // QWebSocket* m_socket;
    WebSocketPair* m_web_sockets = nullptr;
    QTreeView* m_treeView;
    QStandardItemModel* m_model;
    QLabel* m_pathLabel;
    QLabel* m_pageLabel;
    QPushButton* m_prevButton;
    QPushButton* m_nextButton;
    QString m_selectedFile;
    QString m_currentPath = "/";
    int m_currentPage = 1;

    void requestDir( const QString& path, int page )
    {
        QJsonObject msg;
        msg["event"] = "fileList";
        msg["path"] = path;
        msg["page"] = page;
        msg["per_page"] = 20;
        m_web_sockets->text()->sendTextMessage( QJsonDocument( msg ).toJson( QJsonDocument::Compact ) );
    }

private slots:
    void onOk()
    {
        QModelIndex index = m_treeView->currentIndex();
        if( !index.isValid() ) return;

        QString name = m_model->item( index.row(), 0 )->text();
        QString type = m_model->item( index.row(), 1 )->text();

        if( type == "file" )
        {
            m_selectedFile = m_currentPath + "/" + name;
            accept();
        }
    }

    void onDoubleClicked( const QModelIndex& index )
    {
        if( !index.isValid() ) return;

        QString name = m_model->item( index.row(), 0 )->text();
        QString type = m_model->item( index.row(), 1 )->text();

        // 「..」なら親フォルダに戻る
        if( name == ".." && type == "dir" )
        {
            QFileInfo info( m_currentPath );
            QString parent = info.path();
            if( parent.isEmpty() ) parent = "/";
            m_currentPath = parent;
            m_currentPage = 1;
            requestDir( m_currentPath, m_currentPage );
            return;
        }

        if( type == "dir" )
        {
            m_currentPath = ( m_currentPath == "/" ? "" : m_currentPath ) + "/" + name;
            m_currentPage = 1;
            requestDir( m_currentPath, m_currentPage );
        }
    }

    void onMessageReceived( const QString& msg )
    {
        auto doc = QJsonDocument::fromJson( msg.toUtf8() );
        if( !doc.isObject() ) return;

        auto obj = doc.object();
        if( !obj.contains( "files" ) ) return;

        m_model->removeRows( 0, m_model->rowCount() );
        for( auto f : obj["files"].toArray() )
        {
            QJsonObject file = f.toObject();
            QList<QStandardItem*> row;
            row << new QStandardItem( file["name"].toString() ) << new QStandardItem( file["type"].toString() );
            m_model->appendRow( row );
        }

        m_pathLabel->setText( "Path: " + ( m_currentPath.isEmpty() ? "/" : m_currentPath ) );
        m_pageLabel->setText( QString( "Page: %1" ).arg( m_currentPage ) );

        bool hasNext = obj.value( "has_next" ).toBool( false );
        m_nextButton->setEnabled( hasNext );
        m_prevButton->setEnabled( m_currentPage > 1 );
    }
};

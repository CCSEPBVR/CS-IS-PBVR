#pragma once

#include <functional>
#include <utility>

#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "RemoteFileDialog.h"
#include "VizMode.h"

class NetcdfAuxiliaryFileDialog : public QDialog
{
public:
    enum class Kind { CamConnectivity, SlacModes };

    NetcdfAuxiliaryFileDialog( Kind kind, Viz::Mode mode, WebSocketPair* web_sockets,
                               QWidget* parent = nullptr )
        : QDialog( parent ), m_kind( kind ), m_mode( mode ), m_web_sockets( web_sockets )
    {
        setWindowTitle( kind == Kind::CamConnectivity
                            ? tr( "CAM connectivity" )
                            : tr( "SLAC mode" ) );
        auto* layout = new QVBoxLayout( this );
        layout->addWidget( new QLabel(
            kind == Kind::CamConnectivity
                ? tr( "connectivityファイルを選択してください" )
                : tr( "modeファイルを選択してください" ), this ) );

        auto* path_layout = new QHBoxLayout;
        m_path = new QLineEdit( this );
        m_path->setPlaceholderText( kind == Kind::CamConnectivity
                                        ? QStringLiteral( "/data/connectivity.nc" )
                                        : QStringLiteral( "/data/mode_*.nc" ) );
        auto* browse_button = new QPushButton( tr( "参照" ), this );
        path_layout->addWidget( m_path );
        path_layout->addWidget( browse_button );
        layout->addLayout( path_layout );

        m_error = new QLabel( this );
        m_error->setWordWrap( true );
        m_error->setStyleSheet( QStringLiteral( "color: #b00020;" ) );
        layout->addWidget( m_error );

        auto* buttons = new QHBoxLayout;
        auto* ok = new QPushButton( tr( "OK" ), this );
        auto* cancel = new QPushButton( tr( "Cancel" ), this );
        buttons->addStretch();
        buttons->addWidget( ok );
        buttons->addWidget( cancel );
        layout->addLayout( buttons );

        connect( browse_button, &QPushButton::clicked, this, [this]() { this->browse(); } );
        connect( ok, &QPushButton::clicked, this, [this]() {
            const QString path = m_path->text().trimmed();
            if ( path.isEmpty() )
            {
                setError( tr( "ファイルパスを入力してください。" ) );
                return;
            }
            m_error->clear();
            if ( m_submit ) m_submit( path );
        } );
        connect( cancel, &QPushButton::clicked, this, &QDialog::reject );
    }

    void setSubmitHandler( std::function<void( const QString& )> handler )
    {
        m_submit = std::move( handler );
    }
    void setError( const QString& message ) { m_error->setText( message ); }

private:
    void browse()
    {
        QString selected;
        if ( m_mode == Viz::Mode::LocalClientAndServer )
        {
            selected = QFileDialog::getOpenFileName(
                this, windowTitle(), QFileInfo( m_path->text() ).absolutePath(),
                tr( "NetCDF files (*.nc *.ncdf);;All files (*)" ) );
        }
        else
        {
            QString initial = QFileInfo( m_path->text() ).path();
            if ( initial.isEmpty() || initial == QStringLiteral( "." ) ) initial = "/";
            RemoteFileDialog dialog( m_web_sockets, this, windowTitle(), initial );
            if ( dialog.exec() == QDialog::Accepted ) selected = dialog.selectedFile();
        }
        if ( !selected.isEmpty() ) m_path->setText( selected );
    }

    Kind m_kind;
    Viz::Mode m_mode;
    WebSocketPair* m_web_sockets = nullptr;
    QLineEdit* m_path = nullptr;
    QLabel* m_error = nullptr;
    std::function<void( const QString& )> m_submit;
};

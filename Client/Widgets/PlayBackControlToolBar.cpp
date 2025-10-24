#include "PlayBackControlToolBar.h"

PlayBackControlToolBar::PlayBackControlToolBar( QWidget* parent ) :
    QToolBar( parent )
{
    initialize();
}

PlayBackControlToolBar::~PlayBackControlToolBar() {}

void PlayBackControlToolBar::initialize()
{
    QSize iconSize( 60, 60 );
    QSize buttonSize( 50, 50 );

    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_first_time_step_push_button = createButton( "://Resources/images/first.svg", iconSize, buttonSize, this );

    m_previous_time_step_push_button = createButton( "://Resources/images/previous.svg", iconSize, buttonSize, this );

    m_reverse_push_button = createButton( "://Resources/images/reverse.svg", iconSize, buttonSize, this );
    m_reverse_push_button->setCheckable( true );

    m_play_push_button = createButton( "://Resources/images/play.svg", iconSize, buttonSize, this );
    m_play_push_button->setCheckable( true );

    m_next_time_step_push_button = createButton( "://Resources/images/next.svg", iconSize, buttonSize, this );

    m_last_time_step_push_button = createButton( "://Resources/images/last.svg", iconSize, buttonSize, this );

    m_keep_last_time_step_push_button = createButton( "://Resources/images/keepLast.svg", iconSize, buttonSize, this );
    m_keep_last_time_step_push_button->setCheckable( true );

    m_jump_push_button = createButton( "://Resources/images/jump.svg", iconSize, buttonSize, this );

    m_loop_push_button = createButton( "://Resources/images/loop.svg", iconSize, buttonSize, this );
    m_loop_push_button->setCheckable( true );

    layout->addWidget( m_first_time_step_push_button );
    layout->addWidget( m_previous_time_step_push_button );
    layout->addWidget( m_reverse_push_button );
    layout->addWidget( m_play_push_button );
    layout->addWidget( m_next_time_step_push_button );
    layout->addWidget( m_last_time_step_push_button );
    layout->addWidget( m_keep_last_time_step_push_button );
    layout->addWidget( m_jump_push_button );
    layout->addWidget( m_loop_push_button );

    this->addWidget( containerWidget );
    this->setMovable( false );

    connect( m_first_time_step_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBar::onFirst );
    connect( m_previous_time_step_push_button  , &QPushButton::clicked, this, &PlayBackControlToolBar::onPrevious );
    connect( m_reverse_push_button             , &QPushButton::clicked, this, &PlayBackControlToolBar::onReverse );
    connect( m_play_push_button                , &QPushButton::clicked, this, &PlayBackControlToolBar::onPlay );
    connect( m_next_time_step_push_button      , &QPushButton::clicked, this, &PlayBackControlToolBar::onNext );
    connect( m_last_time_step_push_button      , &QPushButton::clicked, this, &PlayBackControlToolBar::onLast );
    connect( m_keep_last_time_step_push_button , &QPushButton::clicked, this, &PlayBackControlToolBar::onKeepLast );
    connect( m_jump_push_button                , &QPushButton::clicked, this, &PlayBackControlToolBar::onJump );
    connect( m_loop_push_button                , &QPushButton::clicked, this, &PlayBackControlToolBar::onLoop );
}

QPushButton* PlayBackControlToolBar::createButton( const QString &iconPath, const QSize &iconSize, const QSize &buttonSize, QWidget *parent )
{
    QPushButton* button = new QPushButton( parent );
    button->setIcon( QIcon(iconPath ) );
    button->setIconSize( iconSize );
    button->setMinimumSize( buttonSize );
    button->setMaximumSize( buttonSize );
    button->setStyleSheet(
        "QPushButton {"
        "border: none;"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(0, 0, 0, 50);"
        "}"
        "QPushButton:checked {"
        "background-color: rgba(0, 0, 0, 80);"
        "}"
        ); // ボタンの枠を消し、押した時の背景色を設定
    return button;
}

void PlayBackControlToolBar::disableButtons()
{
    QVector<QPushButton*> buttons =
        {
            m_first_time_step_push_button,
            m_previous_time_step_push_button,
            m_reverse_push_button,
            m_play_push_button,
            m_next_time_step_push_button,
            m_last_time_step_push_button,
            m_keep_last_time_step_push_button,
            m_jump_push_button,
        };

    for ( auto button : buttons )
    {
        if (button == m_reverse_push_button && m_reverse_push_button->isChecked())
        {
            continue; // m_reverse_push_button が checked ならスキップ
        }
        if (button == m_play_push_button && m_play_push_button->isChecked())
        {
            continue; // m_play_push_button が checked ならスキップ
        }
        if( button == m_keep_last_time_step_push_button && m_keep_last_time_step_push_button->isChecked() )
        {
            continue; // m_keep_last_time_step_push_button 縺・checked 縺ｪ繧峨せ繧ｭ繝・・
        }
        button->setDisabled( true );
    }
}

void PlayBackControlToolBar::enableButtons()
{
    if( m_reverse_push_button->isChecked() )
    {
        return;
    }
    if( m_play_push_button->isChecked() )
    {
        return;
    }
    if( m_keep_last_time_step_push_button->isChecked() )
    {
        return;
    }
    QVector<QPushButton*> buttons =
        {
            m_first_time_step_push_button,
            m_previous_time_step_push_button,
            m_reverse_push_button,
            m_play_push_button,
            m_next_time_step_push_button,
            m_last_time_step_push_button,
            m_keep_last_time_step_push_button,
            m_jump_push_button,
        };

    for ( auto button : buttons )
    {
        button->setDisabled( false );
    }
}

void PlayBackControlToolBar::onFirst()
{
    disableButtons();
    emit fisrtTimeStep();
}

void PlayBackControlToolBar::onPrevious()
{
    disableButtons();
    emit previousTimeStep();
}

void PlayBackControlToolBar::onReverse()
{
    if( m_reverse_push_button->isChecked() )
    {
        disableButtons();
        m_reverse_push_button->setIcon( QIcon("://Resources/images/pause.svg") );
    }
    else
    {
        enableButtons();
        m_reverse_push_button->setIcon( QIcon("://Resources/images/reverse.svg") );
    }
    emit reverseTimeStep( m_reverse_push_button->isChecked() );
}

void PlayBackControlToolBar::onPlay()
{
    if( m_play_push_button->isChecked() )
    {
        disableButtons();
        m_play_push_button->setIcon( QIcon("://Resources/images/pause.svg") );
    }
    else
    {
        enableButtons();
        m_play_push_button->setIcon( QIcon("://Resources/images/play.svg") );
    }
    emit playTimeStep( m_play_push_button->isChecked() );
}

void PlayBackControlToolBar::onNext()
{
    disableButtons();
    emit nextTimeStep();
}

void PlayBackControlToolBar::onLast()
{
    disableButtons();
    emit lastTimeStep();
}

void PlayBackControlToolBar::onKeepLast()
{
    if( m_keep_last_time_step_push_button->isChecked() )
    {
        disableButtons();
        m_keep_last_time_step_push_button->setIcon( QIcon("://Resources/images/pause.svg") );
    }
    else
    {
        enableButtons();
        m_keep_last_time_step_push_button->setIcon( QIcon("://Resources/images/keepLast.svg") );
    }


    emit keepLastTimeStep( m_keep_last_time_step_push_button->isChecked() );
}

void PlayBackControlToolBar::onJump()
{
    disableButtons();
    emit jumpTimeStep();
}

void PlayBackControlToolBar::onLoop()
{
    emit loopMode( m_loop_push_button->isChecked() );
}

void PlayBackControlToolBar::doneMerge()
{
    enableButtons();
}

void PlayBackControlToolBar::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void PlayBackControlToolBar::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

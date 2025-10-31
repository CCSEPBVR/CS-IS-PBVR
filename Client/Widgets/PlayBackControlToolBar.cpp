#include "PlayBackControlToolBar.h"

PlayBackControlToolBar::PlayBackControlToolBar( QWidget* parent ) :
    QToolBar( parent )
{
    initialize();
}

PlayBackControlToolBar::~PlayBackControlToolBar() {}

void PlayBackControlToolBar::donePlayBackControlToolBar()
{
    enableButtons();
}

void PlayBackControlToolBar::initialize()
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    QSize iconSize( 60, 60 );
    QSize pushButtonSize( 50, 50 );

    m_first_push_button     = createPushButton( "://Resources/images/first.svg", iconSize, pushButtonSize, this );
    m_previous_push_button  = createPushButton( "://Resources/images/previous.svg", iconSize, pushButtonSize, this );
    m_reverse_push_button   = createPushButton( "://Resources/images/reverse.svg", iconSize, pushButtonSize, this );
    m_reverse_push_button   ->setCheckable( true );
    m_play_push_button      = createPushButton( "://Resources/images/play.svg", iconSize, pushButtonSize, this );
    m_play_push_button      ->setCheckable( true );
    m_next_push_button      = createPushButton( "://Resources/images/next.svg", iconSize, pushButtonSize, this );
    m_last_push_button      = createPushButton( "://Resources/images/last.svg", iconSize, pushButtonSize, this );
    m_keep_last_push_button = createPushButton( "://Resources/images/keepLast.svg", iconSize, pushButtonSize, this );
    m_keep_last_push_button ->setCheckable( true );
    m_jump_push_button      = createPushButton( "://Resources/images/jump.svg", iconSize, pushButtonSize, this );
    m_loop_push_button      = createPushButton( "://Resources/images/loop.svg", iconSize, pushButtonSize, this );
    m_loop_push_button      ->setCheckable( true );

    layout->addWidget( m_first_push_button );
    layout->addWidget( m_previous_push_button );
    layout->addWidget( m_reverse_push_button );
    layout->addWidget( m_play_push_button );
    layout->addWidget( m_next_push_button );
    layout->addWidget( m_last_push_button );
    layout->addWidget( m_keep_last_push_button );
    layout->addWidget( m_jump_push_button );
    layout->addWidget( m_loop_push_button );

    this->addWidget( containerWidget );

    connect( m_first_push_button    , &QPushButton::clicked, this, &PlayBackControlToolBar::onFirst );
    connect( m_previous_push_button , &QPushButton::clicked, this, &PlayBackControlToolBar::onPrevious );
    connect( m_reverse_push_button  , &QPushButton::clicked, this, &PlayBackControlToolBar::onReverse );
    connect( m_play_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBar::onPlay );
    connect( m_next_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBar::onNext );
    connect( m_last_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBar::onLast );
    connect( m_keep_last_push_button, &QPushButton::clicked, this, &PlayBackControlToolBar::onKeepLast );
    connect( m_jump_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBar::onJump );
    connect( m_loop_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBar::onLoop );
}

QPushButton* PlayBackControlToolBar::createPushButton( const QString &iconPath, const QSize &iconSize, const QSize &buttonSize, QWidget *parent )
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
            m_first_push_button,
            m_previous_push_button,
            m_reverse_push_button,
            m_play_push_button,
            m_next_push_button,
            m_last_push_button,
            m_keep_last_push_button,
            m_jump_push_button,
        };

    for( auto button : buttons )
    {
        if (button == m_reverse_push_button && m_reverse_push_button->isChecked())
        {
            continue; // m_reverse_push_button が checked ならスキップ
        }
        if (button == m_play_push_button && m_play_push_button->isChecked())
        {
            continue; // m_play_push_button が checked ならスキップ
        }
        if( button == m_keep_last_push_button && m_keep_last_push_button->isChecked() )
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
    if( m_keep_last_push_button->isChecked() )
    {
        return;
    }
    QVector<QPushButton*> buttons =
        {
            m_first_push_button,
            m_previous_push_button,
            m_reverse_push_button,
            m_play_push_button,
            m_next_push_button,
            m_last_push_button,
            m_keep_last_push_button,
            m_jump_push_button,
        };

    for( auto button : buttons )
    {
        button->setDisabled( false );
    }
}

void PlayBackControlToolBar::onFirst()
{
    disableButtons();
    emit first();
}

void PlayBackControlToolBar::onPrevious()
{
    disableButtons();
    emit previous();
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
    emit reverse( m_reverse_push_button->isChecked() );
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
    emit play( m_play_push_button->isChecked() );
}

void PlayBackControlToolBar::onNext()
{
    disableButtons();
    emit next();
}

void PlayBackControlToolBar::onLast()
{
    disableButtons();
    emit last();
}

void PlayBackControlToolBar::onKeepLast()
{
    if( m_keep_last_push_button->isChecked() )
    {
        disableButtons();
        m_keep_last_push_button->setIcon( QIcon("://Resources/images/pause.svg") );
    }
    else
    {
        enableButtons();
        m_keep_last_push_button->setIcon( QIcon("://Resources/images/keepLast.svg") );
    }

    emit keepLast( m_keep_last_push_button->isChecked() );
}

void PlayBackControlToolBar::onJump()
{
    disableButtons();
    emit jump();
}

void PlayBackControlToolBar::onLoop()
{
    emit loop( m_loop_push_button->isChecked() );
}

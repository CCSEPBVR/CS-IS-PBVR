#include "PlayBackControlToolBarWIP.h"

PlayBackControlToolBarWIP::PlayBackControlToolBarWIP( QWidget* parent )
    : QToolBar( parent )
    , m_is_operator( false )
{
    const QSize iconSize( 60, 60 );
    const QSize pushButtonSize( 50, 50 );

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

    QWidget* containerWidget    = new QWidget( this );
    QHBoxLayout* layout         = new QHBoxLayout( containerWidget );
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

    m_buttons =
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

    connect( m_first_push_button    , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onFirst );
    connect( m_previous_push_button , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onPrevious );
    connect( m_reverse_push_button  , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onReverse );
    connect( m_play_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onPlay );
    connect( m_next_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onNext );
    connect( m_last_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onLast );
    connect( m_keep_last_push_button, &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onKeepLast );
    connect( m_jump_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onJump );
    connect( m_loop_push_button     , &QPushButton::clicked, this, &PlayBackControlToolBarWIP::onLoop );
}

PlayBackControlToolBarWIP::~PlayBackControlToolBarWIP() {}

void PlayBackControlToolBarWIP::onDataRequestCompleted()
{
    if( m_is_operator ) enableButtons();
}

void PlayBackControlToolBarWIP::onOperatorStateUpdate( bool operatorState )
{
    m_is_operator = operatorState;
    if( m_is_operator )
    {
        m_first_push_button    ->setEnabled( true );
        m_previous_push_button ->setEnabled( true );
        m_reverse_push_button  ->setEnabled( true );
        m_play_push_button     ->setEnabled( true );
        m_next_push_button     ->setEnabled( true );
        m_last_push_button     ->setEnabled( true );
        m_keep_last_push_button->setEnabled( true );
        m_jump_push_button     ->setEnabled( true );
        m_loop_push_button     ->setEnabled( true );
    }
    else
    {
        m_reverse_push_button   ->setChecked( false ); onReverse();
        m_play_push_button      ->setChecked( false ); onPlay();
        m_keep_last_push_button ->setChecked( false ); onKeepLast();
        m_loop_push_button      ->setChecked( false ); onLoop();
        m_loop_push_button      ->setEnabled( false ); disableButtons();
    }
}

QPushButton* PlayBackControlToolBarWIP::createPushButton( const QString& iconPath, const QSize& iconSize, const QSize& buttonSize, QWidget* parent )
{
    QPushButton* button = new QPushButton( parent );
    button->setIcon( QIcon( iconPath ) );
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

void PlayBackControlToolBarWIP::disableButtons()
{
    for( QPushButton* button : m_buttons )
    {
        // リバース、プレイ、キープラストボタンのいずれかが押されている場合はスキップ
        if( ( button == m_reverse_push_button  && button->isChecked() ) ||
            ( button == m_play_push_button     && button->isChecked() ) ||
            ( button == m_keep_last_push_button && button->isChecked()) )
        {
            continue;
        }

        button->setEnabled( false );
    }
}

void PlayBackControlToolBarWIP::enableButtons()
{
    // リバース、プレイ、キープラストボタンのいずれかが押されている場合は何もしない
    if( m_reverse_push_button->isChecked() ||
        m_play_push_button->isChecked() ||
        m_keep_last_push_button->isChecked() )
    {
        return;
    }

    for( QPushButton* button : m_buttons )
    {
        button->setEnabled( true );
    }
}

void PlayBackControlToolBarWIP::onFirst()
{
    disableButtons();
    emit first();
}

void PlayBackControlToolBarWIP::onPrevious()
{
    disableButtons();
    emit previous();
}

void PlayBackControlToolBarWIP::onReverse()
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

void PlayBackControlToolBarWIP::onPlay()
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

void PlayBackControlToolBarWIP::onNext()
{
    disableButtons();
    emit next();
}

void PlayBackControlToolBarWIP::onLast()
{
    disableButtons();
    emit last();
}

void PlayBackControlToolBarWIP::onKeepLast()
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

void PlayBackControlToolBarWIP::onJump()
{
    disableButtons();
    emit jump();
}

void PlayBackControlToolBarWIP::onLoop()
{
    emit loop( m_loop_push_button->isChecked() );
}

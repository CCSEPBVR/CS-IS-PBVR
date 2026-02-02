#include "TimeStepControlToolBar.h"

TimeStepControlToolBar::TimeStepControlToolBar( WebSocketPair* websockets, QWidget* parent )
    : QToolBar( parent )
    , m_web_sockets( websockets )
    , m_is_operator( false )
{
    m_current_time_step_label     = new QLabel( "Current Time Step :", this );
    m_current_time_step_line_edit = new QLineEdit( this );
    m_current_time_step_line_edit->setReadOnly( true );
    m_current_time_step_line_edit->setFixedWidth( 100 );

    QFrame* verticleLine1 = new QFrame( this );
    verticleLine1->setFrameShape( QFrame::VLine );

    m_next_time_step_label    = new QLabel( "Next Time Step :", this );
    m_next_time_step_spin_box = new QSpinBox( this );
    m_next_time_step_spin_box->setRange( 0, 0 );
    m_next_time_step_spin_box->setFixedWidth( 100 );

    m_min_limit_time_step_label    = new QLabel( "Min :", this );
    m_min_limit_time_step_spin_box = new QSpinBox( this );
    m_min_limit_time_step_spin_box->setRange( 0, 0 );
    m_min_limit_time_step_spin_box->setFixedWidth( 100 );

    m_max_limit_time_step_label    = new QLabel( "Max :", this );
    m_max_limit_time_step_spin_box = new QSpinBox( this );
    m_max_limit_time_step_spin_box->setRange( 0, 0 );
    m_max_limit_time_step_spin_box->setFixedWidth( 100 );

    QFrame* verticleLine2 = new QFrame( this );
    verticleLine2->setFrameShape( QFrame::VLine );

    m_total_time_step_range_label = new QLabel( QString( "(Min :%1, Max :%2)" ).arg( 0 ).arg( 0 ), this );

    QFrame* verticleLine3 = new QFrame( this );
    verticleLine3->setFrameShape( QFrame::VLine );

    m_update_interval_label    = new QLabel( "Step Interval :", this );
    m_update_interval_spin_box = new QSpinBox( this );
    m_update_interval_spin_box->setRange( 0, 60000 );
    m_update_interval_spin_box->setValue( 100 );
    m_update_interval_spin_box->setSuffix( "ms" );
    m_update_interval_spin_box->setFixedWidth( 100 );

    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout      = new QHBoxLayout( containerWidget );
    layout->addWidget( m_current_time_step_label );
    layout->addWidget( m_current_time_step_line_edit );
    layout->addWidget( verticleLine1 );
    layout->addWidget( m_next_time_step_label );
    layout->addWidget( m_next_time_step_spin_box );
    layout->addWidget( m_next_time_step_label );
    layout->addWidget( m_next_time_step_spin_box );
    layout->addWidget( m_min_limit_time_step_label );
    layout->addWidget( m_min_limit_time_step_spin_box );
    layout->addWidget( m_max_limit_time_step_label );
    layout->addWidget( m_max_limit_time_step_spin_box );
    layout->addWidget( verticleLine2 );
    layout->addWidget( m_total_time_step_range_label );
    layout->addWidget( verticleLine3 );
    layout->addWidget( m_update_interval_label );
    layout->addWidget( m_update_interval_spin_box );
    layout->addStretch(); // NOTE:左詰め

    this->addWidget( containerWidget );
    this->setMovable( false );

    m_timer.setInterval( m_update_interval_spin_box->value() );

    connect( m_next_time_step_spin_box     , QOverload<int>::of( &QSpinBox::valueChanged ), this, &TimeStepControlToolBar::onUpdateNext );
    connect( m_min_limit_time_step_spin_box, QOverload<int>::of( &QSpinBox::valueChanged ), this, &TimeStepControlToolBar::onUpdateMinLimit );
    connect( m_max_limit_time_step_spin_box, QOverload<int>::of( &QSpinBox::valueChanged ), this, &TimeStepControlToolBar::onUpdateMaxLimit );
    connect( m_update_interval_spin_box    , QOverload<int>::of( &QSpinBox::valueChanged ), this, &TimeStepControlToolBar::onUpdateInterval );
    connect( &m_timer                      , &QTimer::timeout                             , this, &TimeStepControlToolBar::onUpdateTimeStep );
}

TimeStepControlToolBar::~TimeStepControlToolBar() {}

void TimeStepControlToolBar::reset()
{

}

void TimeStepControlToolBar::onOperatorStateUpdate( const bool operatorState )
{
    m_is_operator = operatorState;

    m_next_time_step_spin_box     ->setEnabled( m_is_operator );
    m_min_limit_time_step_spin_box->setEnabled( m_is_operator );
    m_max_limit_time_step_spin_box->setEnabled( m_is_operator );
}

void TimeStepControlToolBar::onReceiveTimeStepControlParameter( const QJsonObject& timeStepControlParameter )
{
    if( timeStepControlParameter.contains( QString::fromUtf8( Protocol::Key::NextTimeStep ) ) )
        m_next_time_step_spin_box->setValue( timeStepControlParameter.value( QString::fromUtf8( Protocol::Key::NextTimeStep ) ).toInt() );

    if( timeStepControlParameter.contains( QString::fromUtf8( Protocol::Key::MinLimit  ) ) )
        m_min_limit_time_step_spin_box->setValue( timeStepControlParameter.value( QString::fromUtf8( Protocol::Key::MinLimit ) ).toInt() );

    if( timeStepControlParameter.contains( QString::fromUtf8( Protocol::Key::MaxLimit ) ) )
        m_max_limit_time_step_spin_box->setValue( timeStepControlParameter.value( QString::fromUtf8( Protocol::Key::MaxLimit ) ).toInt() );
}

void TimeStepControlToolBar::onUpdateTotalTimeStepRange( int min, int max, const bool isSingleObject )
{
    if( min == std::numeric_limits<int>::max() && max == std::numeric_limits<int>::min() )
    {
        min = 0;
        max = 0;
    }

    m_total_time_step_range_label->setText( QString( "(Min :%1, Max :%2)" ).arg( min ).arg( max ) );
    m_min_limit_time_step_spin_box->setMinimum( min );
    m_min_limit_time_step_spin_box->setMaximum( max );
    m_max_limit_time_step_spin_box->setMinimum( min );
    m_max_limit_time_step_spin_box->setMaximum( max );

    if( isSingleObject )
    {
        m_min_limit_time_step_spin_box->setValue( min );
        m_max_limit_time_step_spin_box->setValue( max );
    }
}

void TimeStepControlToolBar::onDataRequestCompleted( const int requestTimeStep )
{
    m_current_time_step_line_edit->setText( QString::number( requestTimeStep ) );
    m_is_merging = false;
    emit dataRequestCompleted( requestTimeStep );
}

void TimeStepControlToolBar::onFirst()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    m_next_time_step_spin_box->setValue( m_min_limit_time_step_spin_box->minimum() );
    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::onPrevious()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() - 1 );
    }
    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::onReverse( bool isChecked )
{
    if( isChecked )
    {
        m_is_reverse_mode = true;
        m_timer.start();
    }
    else
    {
        m_timer.stop();
    }
}

void TimeStepControlToolBar::onPlay( bool isChecked )
{
    if( isChecked )
    {
        m_is_reverse_mode = false;
        m_timer.start();
    }
    else
    {
        m_timer.stop();
    }
}

void TimeStepControlToolBar::onNext()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() + 1 );
    }
    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::onLast()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    m_next_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::onKeepLast( bool isChecked )
{
    if( isChecked )
    {
        m_is_last_mode = true;
        m_timer.start();
    }
    else
    {
        m_is_last_mode = false;
        m_timer.stop();
    }
}

void TimeStepControlToolBar::onJump()
{
    if( m_is_merging ) return;

    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::onLoop( bool isChecked )
{
    m_next_time_step_spin_box->setWrapping( isChecked );
}

void TimeStepControlToolBar::onLoadParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TimeStepControlToolBar::onSaveParameter( const QString& filePath )
{
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TimeStepControlToolBar::decrementTimeStep()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() - 1 );
    }

    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::incrementTimeStep()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() + 1 );
    }

    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::keepLastTimeStep()
{
    if( m_is_merging ) return; // NOTE:データ要求中は何もしない

    m_max_limit_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_next_time_step_spin_box     ->setValue( m_max_limit_time_step_spin_box->maximum() );

    m_is_merging = true;
    emit requestDataAt( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::onUpdateNext( const int next )
{
    if( !m_web_sockets->isConnected() ) return;

    if( m_is_operator )
    {
        QJsonObject timeStepControlParameter;
        timeStepControlParameter[QString::fromUtf8( Protocol::Key::Event )]        = QString::fromUtf8( Protocol::Events::TimeStepControlParameter );
        timeStepControlParameter[QString::fromUtf8( Protocol::Key::NextTimeStep )] = next;

        m_web_sockets->text()->sendTextMessage( QJsonDocument( timeStepControlParameter ).toJson( QJsonDocument::Compact ) );
    }
}

void TimeStepControlToolBar::onUpdateMinLimit( const int minLimit )
{
    // NOTE:所属スレッドのイベントループで後から実行する
    QMetaObject::invokeMethod( this, [=]() {
        m_max_limit_time_step_spin_box->blockSignals( true );
        m_max_limit_time_step_spin_box->setMinimum( minLimit );
        m_max_limit_time_step_spin_box->blockSignals( false );
        m_next_time_step_spin_box->setMinimum( minLimit );
    }, Qt::QueuedConnection );

    if( !m_web_sockets->isConnected() ) return;

    if( m_is_operator )
    {
        QJsonObject timeStepControlParameter;
        timeStepControlParameter[QString::fromUtf8( Protocol::Key::Event )]    = QString::fromUtf8( Protocol::Events::TimeStepControlParameter );
        timeStepControlParameter[QString::fromUtf8( Protocol::Key::MinLimit )] = minLimit;

        m_web_sockets->text()->sendTextMessage( QJsonDocument( timeStepControlParameter ).toJson( QJsonDocument::Compact ) );
    }
}

void TimeStepControlToolBar::onUpdateMaxLimit( const int maxLimit )
{
    // NOTE:所属スレッドのイベントループで後から実行する
    QMetaObject::invokeMethod( this, [=]() {
        m_min_limit_time_step_spin_box->blockSignals( true );
        m_min_limit_time_step_spin_box->setMaximum( maxLimit );
        m_min_limit_time_step_spin_box->blockSignals( false );
        m_next_time_step_spin_box->setMaximum( maxLimit );
    }, Qt::QueuedConnection );

    if( !m_web_sockets->isConnected() ) return;

    if( m_is_operator )
    {
        QJsonObject timeStepControlParameter;
        timeStepControlParameter[QString::fromUtf8( Protocol::Key::Event )]    = QString::fromUtf8( Protocol::Events::TimeStepControlParameter );
        timeStepControlParameter[QString::fromUtf8( Protocol::Key::MaxLimit )] = maxLimit;

        m_web_sockets->text()->sendTextMessage( QJsonDocument( timeStepControlParameter ).toJson( QJsonDocument::Compact ) );
    }
}

void TimeStepControlToolBar::onUpdateInterval( const int interval )
{
    m_timer.setInterval( interval );
}

void TimeStepControlToolBar::onUpdateTimeStep()
{
    if( m_is_reverse_mode && !m_is_last_mode ) // 逆再生
    {
        decrementTimeStep();
    }
    else if( !m_is_reverse_mode && !m_is_last_mode ) // 再生
    {
        incrementTimeStep();
    }
    else
    {
        keepLastTimeStep();
    }
}

#include "TimeStepControlToolBar.h"

TimeStepControlToolBar::TimeStepControlToolBar( QWidget* parent ) :
    QToolBar( parent )
{
    initialize();
}

TimeStepControlToolBar::~TimeStepControlToolBar() {}

void TimeStepControlToolBar::initialize()
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_current_time_step_label = new QLabel( "Current Time Step : ", this );
    m_current_time_step_display = new QLabel( "NO DATA", this );

    QFrame *verticalVertical1 = new QFrame( this );
    verticalVertical1->setFrameShape( QFrame::VLine );

    m_jump_time_step_label = new QLabel( "Next Time Step : ", this );
    m_jump_time_step_spin_box = new QSpinBox( this );
    m_jump_time_step_spin_box->setRange( 0, 0 );
    m_jump_time_step_spin_box->setFixedWidth( 100 );

    m_min_limit_time_step_label = new QLabel( "Min : ", this );
    m_min_limit_time_step_spin_box = new QSpinBox( this );
    m_min_limit_time_step_spin_box->setRange( 0, 0 );
    m_min_limit_time_step_spin_box->setFixedWidth( 100 );

    m_max_limit_time_step_label = new QLabel( "Max : ", this );
    m_max_limit_time_step_spin_box = new QSpinBox( this );
    m_max_limit_time_step_spin_box->setRange( 0, 0 );
    m_max_limit_time_step_spin_box->setFixedWidth( 100 );

    QFrame *verticalVertical2 = new QFrame( this );
    verticalVertical2->setFrameShape( QFrame::VLine );

    m_total_time_step_range_label = new QLabel( "(Min : 0 ,Max : 0)", this );

    QFrame *verticalVertical3 = new QFrame( this );
    verticalVertical3->setFrameShape( QFrame::VLine );

    m_update_interval_label = new QLabel( "Step Interval : ", this );
    m_update_interval_spin_box = new QSpinBox( this );
    m_update_interval_spin_box->setRange( 0, 60000 );
    m_update_interval_spin_box->setValue( 100 );
    m_update_interval_spin_box->setSuffix( "ms" );
    m_update_interval_spin_box->setFixedWidth( 100 );

    // Layoutにウィジェットを追加
    layout->addWidget( m_current_time_step_label );
    layout->addWidget( m_current_time_step_display );
    layout->addWidget( verticalVertical1 );
    layout->addWidget( m_jump_time_step_label );
    layout->addWidget( m_jump_time_step_spin_box );
    layout->addWidget( m_min_limit_time_step_label );
    layout->addWidget( m_min_limit_time_step_spin_box );
    layout->addWidget( m_max_limit_time_step_label );
    layout->addWidget( m_max_limit_time_step_spin_box );
    layout->addWidget( verticalVertical2  );
    layout->addWidget( m_total_time_step_range_label );
    layout->addWidget( verticalVertical3 );
    layout->addWidget( m_update_interval_label );
    layout->addWidget( m_update_interval_spin_box );

    // ToolBarにウィジェットを追加
    this->addWidget( containerWidget );
    this->setMovable( false );

    m_timer.setInterval( 100 );

    connect( m_min_limit_time_step_spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this, &TimeStepControlToolBar::updateMinLimit );
    connect( m_max_limit_time_step_spin_box, QOverload<int>::of(&QSpinBox::valueChanged), this, &TimeStepControlToolBar::updateMaxLimit );
    connect( m_update_interval_spin_box, &QSpinBox::valueChanged, this, &TimeStepControlToolBar::updateInternal );
    connect( &m_timer, &QTimer::timeout, this, &TimeStepControlToolBar::updateTimeStep );
}

// private slots:
void TimeStepControlToolBar::updateMinLimit( int value )
{
    m_max_limit_time_step_spin_box->blockSignals( true );
    m_max_limit_time_step_spin_box->setMinimum( value );
    m_max_limit_time_step_spin_box->blockSignals( false );
    m_jump_time_step_spin_box->setMinimum( value );
}

void TimeStepControlToolBar::updateMaxLimit( int value )
{
    QMetaObject::invokeMethod( this, [=]() {
            m_min_limit_time_step_spin_box->blockSignals( true );
            m_min_limit_time_step_spin_box->setMaximum( value );
            m_min_limit_time_step_spin_box->blockSignals( false );
            m_jump_time_step_spin_box->setMaximum( value );
        }, Qt::QueuedConnection ); // GUIスレッドで安全に処理
}

void TimeStepControlToolBar::updateInternal()
{
    m_timer.setInterval( m_update_interval_spin_box->value() );
}

void TimeStepControlToolBar::updateTimeStep()
{
    if ( m_is_reverse_mode && !m_is_last_mode )
    {
        decrementTimeStep(); // 再生: 値を減らす
    }
    else if( !m_is_reverse_mode && !m_is_last_mode )
    {
        incrementTimeStep(); // 騾・・逕・ 蛟､繧貞｢励ｄ縺
    }
    else
    {
        keepLast();
    }
}

void TimeStepControlToolBar::decrementTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    if( m_current_time_step_display->text() == "NO DATA" )
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() );
    }
    else
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() - 1 );
    }
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::incrementTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    if( m_current_time_step_display->text() == "NO DATA" )
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() );
    }
    else
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() + 1 );
    }
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::keepLast()
{
    if( m_is_merging )
    {
        return;
    }
    m_max_limit_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_jump_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::updateTotalTimeStepRange( int min, int max, bool isSingleObject )
{
    m_total_time_step_range_label->setText( QString( "(Min : %1 , Max : %2)" ).arg( min ).arg( max ) );
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

void TimeStepControlToolBar::fisrtTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    m_jump_time_step_spin_box->setValue( m_min_limit_time_step_spin_box->minimum() );
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::previousTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    if( m_current_time_step_display->text() == "NO DATA" )
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() );
    }
    else
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() - 1 );
    }
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::reverseTimeStep( bool isChecked )
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

void TimeStepControlToolBar::playTimeStep( bool isChecked )
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

void TimeStepControlToolBar::nextTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    if( m_current_time_step_display->text() == "NO DATA" )
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() );
    }
    else
    {
        m_jump_time_step_spin_box->setValue( m_jump_time_step_spin_box->value() + 1 );
    }
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::lastTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    m_jump_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::keepLastTimeStep( bool isChecked )
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

void TimeStepControlToolBar::jumpTimeStep()
{
    if( m_is_merging )
    {
        return;
    }

    m_is_merging = true;
    emit requestMerge( m_jump_time_step_spin_box->value() );
}

void TimeStepControlToolBar::loopMode( bool isChecked )
{
    m_jump_time_step_spin_box->setWrapping( isChecked );
}

void TimeStepControlToolBar::mergeFinish( int requestTimeStep )
{
    m_current_time_step_display->setText( QString::number( requestTimeStep ) );
    m_is_merging = false;
    emit doneMerge();
}

void TimeStepControlToolBar::noItems()
{
    m_current_time_step_display->setText( "NO DATA" );
}

void TimeStepControlToolBar::updateInSituObjectMinMaxTimeStep( int min, int max )
{
    m_max_limit_time_step_spin_box->setMaximum( max );
    m_max_limit_time_step_spin_box->setValue( max );
}

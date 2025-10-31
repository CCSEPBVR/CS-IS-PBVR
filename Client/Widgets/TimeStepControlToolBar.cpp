#include "TimeStepControlToolBar.h"

TimeStepControlToolBar::TimeStepControlToolBar( QWidget* parent ) :
    QToolBar( parent )
{
    initialize();
}

TimeStepControlToolBar::~TimeStepControlToolBar() {}

void TimeStepControlToolBar::updateTotalTimeStepRange( int min, int max, bool isSingleObject )
{
    if( min == std::numeric_limits<int>::max() && max == std::numeric_limits<int>::min() )
    {
        min = 0;
        max = 0;
    }

    m_total_time_step_range_label->setText( QString("(Min :%1, Max :%2)").arg( min ).arg( max ) );
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

void TimeStepControlToolBar::first()
{
    if( m_is_merging ) return;

    m_next_time_step_spin_box->setValue( m_min_limit_time_step_spin_box->minimum() );
    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::previous()
{
    if( m_is_merging ) return;

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() - 1 );
    }
    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::reverse( bool isChecked )
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

void TimeStepControlToolBar::play( bool isChecked )
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

void TimeStepControlToolBar::next()
{
    if( m_is_merging ) return;

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() + 1 );
    }
    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::last()
{
    if( m_is_merging ) return;

    m_next_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::keepLast( bool isChecked )
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

void TimeStepControlToolBar::jump()
{
    if( m_is_merging ) return;

    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::loop( bool isChecked )
{
    m_next_time_step_spin_box->setWrapping( isChecked );
}

void TimeStepControlToolBar::doneTimeControlToolBar( int requestTimeStep )
{
    m_current_time_step_line_edit->setText( QString::number( requestTimeStep ) );
    m_is_merging = false;
    emit done();
}

void TimeStepControlToolBar::loadParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TimeStepControlToolBar::saveParameter( const QString& filePath )
{
    // TODO:KPI
    qDebug() << __FILE__ << ":" << __func__ << ":" << filePath;
}

void TimeStepControlToolBar::initialize()
{
    QWidget* containerWidget = new QWidget( this );
    QHBoxLayout* layout = new QHBoxLayout( containerWidget );

    m_current_time_step_label = new QLabel( "Current Time Step :", this );
    m_current_time_step_line_edit = new QLineEdit( this );
    m_current_time_step_line_edit->setReadOnly( true );
    m_current_time_step_line_edit->setFixedWidth( 100 );

    QFrame* verticleLine1 = new QFrame( this );
    verticleLine1->setFrameShape( QFrame::VLine );

    m_next_time_step_label = new QLabel( "Next Time Step :", this );
    m_next_time_step_spin_box = new QSpinBox( this );
    m_next_time_step_spin_box->setRange( 0, 0 );
    m_next_time_step_spin_box->setFixedWidth( 100 );

    m_min_limit_time_step_label = new QLabel( "Min :", this );
    m_min_limit_time_step_spin_box = new QSpinBox( this );
    m_min_limit_time_step_spin_box->setRange( 0, 0 );
    m_min_limit_time_step_spin_box->setFixedWidth( 100 );

    m_max_limit_time_step_label = new QLabel( "Max :", this );
    m_max_limit_time_step_spin_box = new QSpinBox( this );
    m_max_limit_time_step_spin_box->setRange( 0, 0 );
    m_max_limit_time_step_spin_box->setFixedWidth( 100 );

    QFrame* verticleLine2 = new QFrame( this );
    verticleLine2->setFrameShape( QFrame::VLine );

    m_total_time_step_range_label = new QLabel( QString("(Min :%1, Max :%2)").arg(0).arg(0), this );

    QFrame* verticleLine3 = new QFrame( this );
    verticleLine3->setFrameShape( QFrame::VLine );

    m_update_interval_label = new QLabel( "Step Interval :", this );
    m_update_interval_spin_box = new QSpinBox( this );
    m_update_interval_spin_box->setRange( 0, 60000 );
    m_update_interval_spin_box->setValue( 100 );
    m_update_interval_spin_box->setSuffix( "ms" );
    m_update_interval_spin_box->setFixedWidth( 100 );

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
    layout->addStretch(); // 左詰め

    this->addWidget( containerWidget );

    m_timer.setInterval( m_update_interval_spin_box->value() );

    connect( m_min_limit_time_step_spin_box , QOverload<int>::of( &QSpinBox::valueChanged ) , this, &TimeStepControlToolBar::updateMinLimit );
    connect( m_max_limit_time_step_spin_box , QOverload<int>::of( &QSpinBox::valueChanged ) , this, &TimeStepControlToolBar::updateMaxLimit );
    connect( m_update_interval_spin_box     , &QSpinBox::valueChanged                       , this, &TimeStepControlToolBar::updateInterval );
    connect( &m_timer                       , &QTimer::timeout                              , this, &TimeStepControlToolBar::updateTimeStep );
}

void TimeStepControlToolBar::decrementTimeStep()
{
    if( m_is_merging ) return;

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() - 1 );
    }

    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::incrementTimeStep()
{
    if( m_is_merging ) return;

    if( m_current_time_step_line_edit->text().isEmpty() )
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() );
    }
    else
    {
        m_next_time_step_spin_box->setValue( m_next_time_step_spin_box->value() + 1 );
    }

    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::keepLastTimeStep()
{
    if( m_is_merging ) return;
    m_max_limit_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );
    m_next_time_step_spin_box->setValue( m_max_limit_time_step_spin_box->maximum() );

    m_is_merging = true;
    emit requestMerge( m_next_time_step_spin_box->value() );
}

void TimeStepControlToolBar::updateMinLimit( int minLimit )
{
    QMetaObject::invokeMethod( this, [=]() {
            m_max_limit_time_step_spin_box->blockSignals( true );
            m_max_limit_time_step_spin_box->setMinimum( minLimit );
            m_max_limit_time_step_spin_box->blockSignals( false );
            m_next_time_step_spin_box->setMinimum( minLimit );
        }, Qt::QueuedConnection ); // GUIスレッドで安全に処理
}

void TimeStepControlToolBar::updateMaxLimit( int maxLimit )
{
    QMetaObject::invokeMethod( this, [=]() {
            m_min_limit_time_step_spin_box->blockSignals( true );
            m_min_limit_time_step_spin_box->setMaximum( maxLimit );
            m_min_limit_time_step_spin_box->blockSignals( false );
            m_next_time_step_spin_box->setMaximum( maxLimit );
        }, Qt::QueuedConnection ); // GUIスレッドで安全に処理
}

void TimeStepControlToolBar::updateInterval()
{
    m_timer.setInterval( m_update_interval_spin_box->value() );
}

void TimeStepControlToolBar::updateTimeStep()
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

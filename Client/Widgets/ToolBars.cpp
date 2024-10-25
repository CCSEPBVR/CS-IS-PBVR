#include "ToolBars.h"

#include "Widgets/MergePanel.h"
/* A */
TimeControllerA::TimeControllerA(QWidget *parent) :
    QToolBar(parent)
{
        m_current_time_step_label = new QLabel("Current Time Step : ", this);
        QWidgetAction *currentTimeStepLblAction = new QWidgetAction(this);
        currentTimeStepLblAction->setDefaultWidget(m_current_time_step_label);
        this->addAction(currentTimeStepLblAction);

        m_current_time_step_spin_box = new QSpinBox(this);
        m_current_time_step_spin_box->setFixedWidth(100);
        m_current_time_step_spin_box->setMinimum(-1);
        m_current_time_step_spin_box->setMaximum( INT_MAX );
        m_current_time_step_spin_box->setValue(-1);
        m_current_time_step_spin_box->setSpecialValueText( "NO OBJECT" );
        m_current_time_step_spin_box->setButtonSymbols( QAbstractSpinBox::NoButtons );
        m_current_time_step_spin_box->setEnabled(false);
        QWidgetAction *m_current_time_step_line_editAction = new QWidgetAction(this);
        m_current_time_step_line_editAction->setDefaultWidget(m_current_time_step_spin_box);
        this->addAction(m_current_time_step_line_editAction);

        this->addSeparator();

        m_jump_time_step_label = new QLabel("Next Time Step : ", this);
        QWidgetAction *jumpTimeStepLblAction = new QWidgetAction(this);
        jumpTimeStepLblAction->setDefaultWidget(m_jump_time_step_label);
        this->addAction(jumpTimeStepLblAction);

        m_jump_time_step_spin_box = new QSpinBox(this);
        m_jump_time_step_spin_box->setFixedWidth(100);
        m_jump_time_step_spin_box->setMinimum(0);
        m_jump_time_step_spin_box->setMaximum(0);
        QWidgetAction *jumpTimeStepSBoxAction = new QWidgetAction(this);
        jumpTimeStepSBoxAction->setDefaultWidget(m_jump_time_step_spin_box);
        this->addAction(jumpTimeStepSBoxAction);

        this->addSeparator();

        m_min_limit_time_step_label = new QLabel("Min : ", this);
        QWidgetAction *minTimeStepLblAction = new QWidgetAction(this);
        minTimeStepLblAction->setDefaultWidget(m_min_limit_time_step_label);
        this->addAction(minTimeStepLblAction);

        m_min_limit_time_step_spin_box = new QSpinBox(this);
        m_min_limit_time_step_spin_box->setFixedWidth(100);
        m_min_limit_time_step_spin_box->setMinimum(0);
        m_min_limit_time_step_spin_box->setMaximum(0);
        QWidgetAction *minTimeStepSBoxAction = new QWidgetAction(this);
        minTimeStepSBoxAction->setDefaultWidget(m_min_limit_time_step_spin_box);
        this->addAction(minTimeStepSBoxAction);

        m_max_limit_time_step_label = new QLabel("Max : ", this);
        QWidgetAction *maxTimeStepLblAction = new QWidgetAction(this);
        maxTimeStepLblAction->setDefaultWidget(m_max_limit_time_step_label);
        this->addAction(maxTimeStepLblAction);

        m_max_limit_time_step_spin_box = new QSpinBox(this);
        m_max_limit_time_step_spin_box->setFixedWidth(100);
        m_max_limit_time_step_spin_box->setMinimum(0);
        m_max_limit_time_step_spin_box->setMaximum(0);
        QWidgetAction *maxTimeStepSBoxAction = new QWidgetAction(this);
        maxTimeStepSBoxAction->setDefaultWidget(m_max_limit_time_step_spin_box);
        this->addAction(maxTimeStepSBoxAction);

        m_total_time_step_range_label = new QLabel("(Min : 0 ,Max : 0)", this);
        QWidgetAction *totalTimeStepRangeAction = new QWidgetAction(this);
        totalTimeStepRangeAction->setDefaultWidget(m_total_time_step_range_label);
        this->addAction(totalTimeStepRangeAction);

        this->addSeparator();

        m_update_interval_label = new QLabel("Step Interval : ", this);
        QWidgetAction *stepIntervalAction = new QWidgetAction(this);
        stepIntervalAction->setDefaultWidget(m_update_interval_label);
        this->addAction(stepIntervalAction);

        m_update_interval_spin_box = new QSpinBox(this);
        m_update_interval_spin_box->setFixedWidth( 100 );
        m_update_interval_spin_box->setMinimum( 0 );
        m_update_interval_spin_box->setMaximum( 60000 );
        m_update_interval_spin_box->setValue( 100 );
        m_update_interval_spin_box->setSuffix( "ms" );
        QWidgetAction *stepIntervalSBoxAction = new QWidgetAction(this);
        stepIntervalSBoxAction->setDefaultWidget(m_update_interval_spin_box);
        this->addAction(stepIntervalSBoxAction);
}

TimeControllerA::~TimeControllerA()
{
}

/* B */
TimeControllerB::TimeControllerB( QWidget *parent, TimeControllerA *time_controller_a, MergePanel* merge ) :
    QToolBar(parent),
    m_time_controller_a( time_controller_a ),
    m_merge( merge ),
    m_is_long_press_active( false )
{
    QSize iconSize(60, 60);
    QSize buttonSize(50, 50);

    m_first_time_step_push_button = createButton("://Resources/images/first.svg", iconSize, buttonSize, this);
    QWidgetAction *firstAction = new QWidgetAction(this);
    firstAction->setDefaultWidget(m_first_time_step_push_button);
    this->addAction(firstAction);

    m_previous_time_step_push_button = createButton("://Resources/images/previous.svg", iconSize, buttonSize, this);
    QWidgetAction *previousAction = new QWidgetAction(this);
    previousAction->setDefaultWidget(m_previous_time_step_push_button);
    this->addAction(previousAction);

    m_reverse_push_button = createButton("://Resources/images/reverse.svg", iconSize, buttonSize, this);
    m_reverse_push_button->setCheckable( true );
    QWidgetAction *reverseAction = new QWidgetAction(this);
    reverseAction->setDefaultWidget(m_reverse_push_button);
    this->addAction(reverseAction);

    m_play_push_button = createButton("://Resources/images/play.svg", iconSize, buttonSize, this);
    m_play_push_button->setCheckable( true );
    QWidgetAction *playAction = new QWidgetAction(this);
    playAction->setDefaultWidget(m_play_push_button);
    this->addAction(playAction);

    m_next_time_step_push_button = createButton("://Resources/images/next.svg", iconSize, buttonSize, this);
    QWidgetAction *nextAction = new QWidgetAction(this);
    nextAction->setDefaultWidget(m_next_time_step_push_button);
    this->addAction(nextAction);

    m_last_time_step_push_button = createButton("://Resources/images/last.svg", iconSize, buttonSize, this);
    QWidgetAction *lastAction = new QWidgetAction(this);
    lastAction->setDefaultWidget(m_last_time_step_push_button);
    this->addAction(lastAction);

    m_jump_push_button = createButton("://Resources/images/jump.svg", iconSize, buttonSize, this);
    QWidgetAction *jumpAction = new QWidgetAction(this);
    jumpAction->setDefaultWidget(m_jump_push_button);
    this->addAction(jumpAction);

    m_loop_push_button = createButton("://Resources/images/loop.svg", iconSize, buttonSize, this);
    m_loop_push_button->setCheckable( true );
    QWidgetAction *loopAction = new QWidgetAction(this);
    loopAction->setDefaultWidget(m_loop_push_button);
    this->addAction(loopAction);

    //各ボタンが押された時のスロット割り当て
    connect( m_first_time_step_push_button, &QPushButton::clicked, this, &TimeControllerB::onFirst );
    connect( m_previous_time_step_push_button, &QPushButton::clicked, this, &TimeControllerB::onPrevious );
    connect( m_reverse_push_button, &QPushButton::clicked, this, &TimeControllerB::onReverse );
    connect( m_play_push_button, &QPushButton::clicked, this, &TimeControllerB::onPlay );
    connect( m_next_time_step_push_button, &QPushButton::clicked, this, &TimeControllerB::onNext );
    connect( m_last_time_step_push_button, &QPushButton::pressed, this, &TimeControllerB::onLastPressed );
    connect( m_last_time_step_push_button, &QPushButton::released, this, &TimeControllerB::onLastReleased );
    connect( m_jump_push_button, &QPushButton::clicked, this, &TimeControllerB::onJump );
    connect( m_loop_push_button, &QPushButton::clicked, this, &TimeControllerB::onLoop );

    connect( &m_timer, &QTimer::timeout, this, &TimeControllerB::onTimerStart );

    connect( m_time_controller_a->getMinLimitTimeStepSpinBox(), &QSpinBox::valueChanged, this, &TimeControllerB::onMinLimit );
    connect( m_time_controller_a->getMaxLimitTimeStepSpinBox(), &QSpinBox::valueChanged, this, &TimeControllerB::onMaxLimit );
}

TimeControllerB::~TimeControllerB()
{
}

bool TimeControllerB::getLoopButtonEnabled() const
{
    return m_loop_push_button->isChecked();
}

void TimeControllerB::updateMinMax( int min, int max, int totalFiles )
{
    QMetaObject::invokeMethod(this, [this, min, max, totalFiles]() {
            m_time_controller_a->getMinLimitTimeStepSpinBox()->setMinimum( min );
            m_time_controller_a->getMinLimitTimeStepSpinBox()->setMaximum( max );

            m_time_controller_a->getMaxLimitTimeStepSpinBox()->setMinimum( min );
            m_time_controller_a->getMaxLimitTimeStepSpinBox()->setMaximum( max );

            m_time_controller_a->getTotalTimeStepRangeLabel()->setText( QString( "(Min : %1, Max : %2)" ).arg( min ).arg( max ) );

            switch ( totalFiles )
            {
            case 0:
                m_time_controller_a->getCurrentTimeStepLineEdit()->setValue( -1 );

                m_time_controller_a->getMinLimitTimeStepSpinBox()->setValue( 0 );
                m_time_controller_a->getMinLimitTimeStepSpinBox()->setMinimum( 0 );
                m_time_controller_a->getMinLimitTimeStepSpinBox()->setMaximum( 0 );

                m_time_controller_a->getMaxLimitTimeStepSpinBox()->setValue( 0 );
                m_time_controller_a->getMaxLimitTimeStepSpinBox()->setMinimum( 0 );
                m_time_controller_a->getMaxLimitTimeStepSpinBox()->setMaximum( 0 );
                m_time_controller_a->getTotalTimeStepRangeLabel()->setText( QString( "(Min : %1, Max : %2)" ).arg( 0 ).arg( 0 ) );
                break;
            case 1:
                m_time_controller_a->getMinLimitTimeStepSpinBox()->setValue( min );
                m_time_controller_a->getMaxLimitTimeStepSpinBox()->setValue( max );
                break;
            default:
                if( m_time_controller_a->getCurrentTimeStepLineEdit()->value() == -1 )
                {
                    m_time_controller_a->getMinLimitTimeStepSpinBox()->setValue( min );
                    m_time_controller_a->getMaxLimitTimeStepSpinBox()->setValue( max );
                }
                break;
            }
        }, Qt::QueuedConnection);
}

void TimeControllerB::updateTimeStep()
{
    qInfo() << "update!";
    int currentValue = m_time_controller_a->getCurrentTimeStepLineEdit()->value();
    if( m_reverse_push_button->isChecked() == true )
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue(currentValue - 1);
    }
    else if( m_play_push_button->isChecked() == true )
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue(currentValue + 1);
    }
    else
    {
        enableButtons();
    }
}

QPushButton* TimeControllerB::createButton(const QString &iconPath, const QSize &iconSize, const QSize &buttonSize, QWidget *parent)
{
    QPushButton *button = new QPushButton(parent);
    button->setIcon(QIcon(iconPath));
    button->setIconSize(iconSize);
    button->setMinimumSize(buttonSize);
    button->setMaximumSize(buttonSize);
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

void TimeControllerB::disableButtons(QPushButton *pressedButton)
{
    // 押されたボタン以外のボタンを無効化する
    m_first_time_step_push_button->setDisabled(true);
    m_previous_time_step_push_button->setDisabled(true);
    if( (pressedButton == m_reverse_push_button) == false )
    {
        m_reverse_push_button->setDisabled(true);
    }
    if( (pressedButton == m_play_push_button) == false )
    {
        m_play_push_button->setDisabled(true);
    }
    m_next_time_step_push_button->setDisabled(true);
    if( (pressedButton == m_last_time_step_push_button) == false )
    {
        m_last_time_step_push_button->setDisabled(true);
    }
    m_jump_push_button->setDisabled(true);
}

void TimeControllerB::enableButtons()
{
    // ボタンの配列を定義
    QPushButton* buttons[] = {
        m_first_time_step_push_button,
        m_previous_time_step_push_button,
        m_reverse_push_button,
        m_play_push_button,
        m_next_time_step_push_button,
        m_last_time_step_push_button,
        m_jump_push_button
    };

    // すべてのボタンを有効にする
    bool allEnabled = true;
    if (m_reverse_push_button->isChecked() == false || m_play_push_button->isChecked() == false)
    {
        allEnabled = true;
    }
    else if (m_first_time_step_push_button->isChecked() || m_previous_time_step_push_button->isChecked() || m_next_time_step_push_button->isChecked() || m_last_time_step_push_button->isChecked())
    {
        allEnabled = true;
    }
    else
    {
        allEnabled = false;
    }

    // ボタンの状態を設定
    for (QPushButton* button : buttons)
    {
        button->setEnabled(allEnabled);
    }
}

void TimeControllerB::onFirst()
{
    qDebug() << "first!!";
    disableButtons( m_first_time_step_push_button );
    m_time_controller_a->getJumpTimeStepSpinBox()->setValue( m_time_controller_a->getJumpTimeStepSpinBox()->minimum() );
    m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
}

void TimeControllerB::onPrevious()
{
    qDebug() << "previous!!";
    disableButtons( m_previous_time_step_push_button );
    int currentValue = m_time_controller_a->getCurrentTimeStepLineEdit()->value();
    if( currentValue == -1 )
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue( 0 );
    }
    else
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue( currentValue - 1 );
    }
    m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
}

void TimeControllerB::onReverse()
{
    if( m_reverse_push_button->isChecked() )
    {
        qDebug() << "reverse!";
        disableButtons( m_reverse_push_button );
        m_time_controller_a->getJumpTimeStepSpinBox()->setDisabled( true );
        m_time_controller_a->getMinLimitTimeStepSpinBox()->setDisabled( true );
        m_time_controller_a->getMaxLimitTimeStepSpinBox()->setDisabled( true );
        m_reverse_push_button->setIcon( QIcon("://Resources/images/pause.svg") );
        m_timer.start();
    }
    else
    {
        qDebug() << "stop!";
        enableButtons();
        m_time_controller_a->getJumpTimeStepSpinBox()->setDisabled( false );
        m_time_controller_a->getMinLimitTimeStepSpinBox()->setDisabled( false );
        m_time_controller_a->getMaxLimitTimeStepSpinBox()->setDisabled( false );
        m_reverse_push_button->setIcon( QIcon("://Resources/images/reverse.svg") );
        m_timer.stop();
    }
}

void TimeControllerB::onPlay()
{    
    if( m_play_push_button->isChecked() )
    {
        qDebug() << "play!";
        disableButtons( m_play_push_button );
        m_time_controller_a->getJumpTimeStepSpinBox()->setDisabled( true );
        m_time_controller_a->getMinLimitTimeStepSpinBox()->setDisabled( true );
        m_time_controller_a->getMaxLimitTimeStepSpinBox()->setDisabled( true );
        m_play_push_button->setIcon( QIcon("://Resources/images/pause.svg") );
        m_timer.start();
    }
    else
    {
        qDebug() << "stop!";
        m_reverse_push_button->setEnabled( true );
        enableButtons();
        m_time_controller_a->getJumpTimeStepSpinBox()->setDisabled( false );
        m_time_controller_a->getMinLimitTimeStepSpinBox()->setDisabled( false );
        m_time_controller_a->getMaxLimitTimeStepSpinBox()->setDisabled( false );
        m_play_push_button->setIcon( QIcon("://Resources/images/play.svg") );
        m_timer.stop();
    }
}

void TimeControllerB::onNext()
{
    qDebug() << "next!!";
    disableButtons( m_next_time_step_push_button );
    int currentValue = m_time_controller_a->getCurrentTimeStepLineEdit()->value();
    m_time_controller_a->getJumpTimeStepSpinBox()->setValue( currentValue + 1 );
    m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
}

void TimeControllerB::onLastPressed()
{
    m_last_pressed_timer.start();
}

void TimeControllerB::onLastReleased()
{
    qint64 elapsed = m_last_pressed_timer.elapsed();

    if(elapsed > 500) // 長押しと判断される閾値（例えば500ミリ秒）
    {
        onLastLong();
    }
    else
    {
        onLastShort();
    }
}

void TimeControllerB::onLastShort()
{
    if( m_is_long_press_active == false )
    {
        qDebug() << "last short!!";
        disableButtons( m_last_time_step_push_button );
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue( m_time_controller_a->getJumpTimeStepSpinBox()->maximum() );
        m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
    }
    else
    {
        m_last_time_step_push_button->setChecked(true);
    }
}

void TimeControllerB::onLastLong()
{
    if( m_last_time_step_push_button->isCheckable() == false )
    {
        qDebug() << "last long start!!";
        disableButtons( m_last_time_step_push_button );
        m_last_time_step_push_button->setCheckable( true );
        m_last_time_step_push_button->setChecked( true );
        m_is_long_press_active = true;
        m_timer.start();
    }
    else
    {
        qDebug() << "last long stop!!";
        enableButtons();
        m_last_time_step_push_button->setCheckable( false );
        m_is_long_press_active = false;
        m_timer.stop();
    }
}

void TimeControllerB::onJump()
{
    qDebug() << "jump!!";
    disableButtons( m_jump_push_button );
    m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );

}

void TimeControllerB::onLoop()
{    
    qDebug() << "loop!!";

    if( m_loop_push_button->isChecked() )
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setWrapping( true );
    }
    else
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setWrapping( false );
    }
}

void TimeControllerB::onTimerStart()
{
    m_timer.setInterval( m_time_controller_a->getUpdateIntervalSpinBox()->value() );
    int currentValue = m_time_controller_a->getCurrentTimeStepLineEdit()->value();

    if( m_play_push_button->isChecked() )
    {
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue( currentValue + 1 );
        m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
    }
    else if( m_reverse_push_button->isChecked() )
    {
        if( currentValue == -1 )
        {
            m_time_controller_a->getJumpTimeStepSpinBox()->setValue( 0 );
        }
        else
        {
            m_time_controller_a->getJumpTimeStepSpinBox()->setValue( currentValue - 1 );
        }
        m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
    }
    else if( m_last_time_step_push_button->isChecked() )
    {
        m_time_controller_a->getMaxLimitTimeStepSpinBox()->setValue( m_time_controller_a->getMaxLimitTimeStepSpinBox()->maximum() );
        m_time_controller_a->getJumpTimeStepSpinBox()->setValue( m_time_controller_a->getJumpTimeStepSpinBox()->maximum() );
        m_merge->setIsParticleGenerationNeeded( true );
        m_merge->mergeObjects( m_time_controller_a->getCurrentTimeStepLineEdit()->value(), m_time_controller_a->getJumpTimeStepSpinBox()->value() );
    }
}

void TimeControllerB::onMinLimit()
{
    m_time_controller_a->getJumpTimeStepSpinBox()->setMinimum( m_time_controller_a->getMinLimitTimeStepSpinBox()->value() );
    m_time_controller_a->getMaxLimitTimeStepSpinBox()->setMinimum( m_time_controller_a->getMinLimitTimeStepSpinBox()->value() );
}

void TimeControllerB::onMaxLimit()
{
    m_time_controller_a->getJumpTimeStepSpinBox()->setMaximum( m_time_controller_a->getMaxLimitTimeStepSpinBox()->value() );
    m_time_controller_a->getMinLimitTimeStepSpinBox()->setMaximum( m_time_controller_a->getMaxLimitTimeStepSpinBox()->value() );
}

TotalParticles::TotalParticles( QWidget *parent ) :
    QToolBar(parent)
{
    m_total_particles_label = new QLabel( "Total Particles : ", this );
    QWidgetAction *totalParticlesLableAction = new QWidgetAction( this );
    totalParticlesLableAction->setDefaultWidget( m_total_particles_label );
    this->addAction( totalParticlesLableAction );

    m_total_particles_line_edit = new QLineEdit( this );
    m_total_particles_line_edit->setFixedWidth(100);
    m_total_particles_line_edit->setDisabled( true );
    QWidgetAction *totalParticlesLineEditAction = new QWidgetAction( this );
    totalParticlesLineEditAction->setDefaultWidget( m_total_particles_line_edit );
    this->addAction( totalParticlesLineEditAction );
}

TotalParticles::~TotalParticles()
{
}

void TotalParticles::setTotalParticles( int totalParticles )
{
    m_total_particles_line_edit->setText( QString::number( totalParticles ) );
}

#include "App/pbvrgui.h"
ColorMapBarSelector::ColorMapBarSelector( QWidget *parent, PBVRGUI *pbvr_gui, TransferFunctionEditor* transfer_function_editor ) :
    QToolBar( parent ),
    m_pbvr_gui( pbvr_gui ),
    m_transfer_function_editor( transfer_function_editor )
{
    m_color_map_bar_selector_label = new QLabel( "Color Function : ", this );
    QWidgetAction *colorMapBarSelectorLableAction = new QWidgetAction( this );
    colorMapBarSelectorLableAction->setDefaultWidget( m_color_map_bar_selector_label );
    this->addAction( colorMapBarSelectorLableAction );

    m_color_map_bar_selector_combo_box = new QComboBox( this );
    m_color_map_bar_selector_combo_box->setFixedWidth( 100 );
    QWidgetAction *colorMapBarSelectorComboBox = new QWidgetAction( this );
    colorMapBarSelectorComboBox->setDefaultWidget( m_color_map_bar_selector_combo_box );
    this->addAction( colorMapBarSelectorComboBox );

    connect( m_color_map_bar_selector_combo_box, &QComboBox::currentIndexChanged, this, &ColorMapBarSelector::onColorFunctionChanged );
}

ColorMapBarSelector::~ColorMapBarSelector()
{
}

void ColorMapBarSelector::populateColorFunctionLists(int n)
{
    QStringList itemList;
    for (int i=1;i <=n;i++ )
    {
        itemList.append(QString("C%1").arg(i));
    }
    m_color_map_bar_selector_combo_box->clear();
    m_color_map_bar_selector_combo_box->addItems( itemList );
}

void ColorMapBarSelector::onColorFunctionChanged( int index )
{
    const NamedTransferFunctionParameter *transfer_function_color = m_extended_transfer_function_message->getColorTransferFunction( index + 1 );
    if( transfer_function_color != NULL )
    {
        kvs::ColorMap color_map = transfer_function_color->colorMap();
        m_pbvr_gui->colorMapBar()->setColorMap( color_map );
        m_pbvr_gui->colorMapBar()->setRange( m_extended_transfer_function_message->getColorTransferFunction(index + 1)->m_color_variable_min, m_extended_transfer_function_message->getColorTransferFunction(index + 1)->m_color_variable_max );
        m_pbvr_gui->screen()->update();
    }
}

void ColorMapBarSelector::updateColorMap()
{
    const NamedTransferFunctionParameter *transfer_function_color = m_extended_transfer_function_message->getColorTransferFunction( m_color_map_bar_selector_combo_box->currentIndex() + 1  );
    if( transfer_function_color != NULL )
    {
        kvs::ColorMap color_map = transfer_function_color->colorMap();
        m_pbvr_gui->colorMapBar()->setColorMap( color_map );
        m_pbvr_gui->colorMapBar()->setRange( m_extended_transfer_function_message->getColorTransferFunction( m_color_map_bar_selector_combo_box->currentIndex() + 1)->m_color_variable_min, m_extended_transfer_function_message->getColorTransferFunction( m_color_map_bar_selector_combo_box->currentIndex() + 1)->m_color_variable_max );
        m_pbvr_gui->screen()->update();
    }
}

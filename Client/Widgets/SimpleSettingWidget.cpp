#include "SimpleSettingWidget.h"
#include "Communication.h"
#include "TransferFunctionEditor.h"
#include "WebSocketPair.h"
#include "ObjectEditor.h"
#include "GlyphEditor.h"
#include "PlotOverLineEditor.h"
#include "PointSizeControl.h"
#include "RepetitionLevelControl.h"

#include <QButtonGroup>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QUuid>

namespace
{
struct Target
{
    const char* label;
    const char* volumePath;
    const char* transferFunctionPath;
    int colorFunction;
    const char* terrainPath;
};
const Target targets[] = {
    { "CityLBM 放射性物質",
      R"(C:\Users\vis_team\work\CityLBMVolumeDataDownSize4\pvtu_value-city-downsize4-ens0000-step0.pfl)",
      R"(C:\Users\vis_team\work\CityLBMVolumeDataDownSize4\CityLBM.tfe)", 6,
      R"(C:\Users\vis_team\work\CityLMBVolumeDataZ150\z150\buildingsv2_00000.kvsml)" },
    { "WindAroundBuilding Q値",
      R"(C:\Users\vis_team\work\WindAroundBuildingVolumeData\internal_0.pfl)",
      R"(C:\Users\vis_team\work\WindAroundBuildingVolumeData\default.tfe)", 1,
      R"(C:\Users\vis_team\work\STLToKVSML\Output\WindAroundBuilding_00000.kvsml)" },
    { "冷却対流 温度",
      R"(C:\Users\vis_team\work\mej\example.pfl)",
      R"(C:\Users\vis_team\work\mej\mej_v2.tfe)", 1, nullptr }
};
}

SimpleSettingWidget::SimpleSettingWidget( Communication* communication, TransferFunctionEditor* editor,
                                          WebSocketPair* sockets, ObjectEditor* objects, GlyphEditor* glyphs,
                                          PointSizeControl* pointSize, RepetitionLevelControl* repetition,
                                          PlotOverLineEditor* plotOverLine,
                                          QWidget* parent )
    : QDockWidget( tr( "簡単可視化設定" ), parent )
    , m_communication( communication )
    , m_editor( editor )
    , m_sockets( sockets )
    , m_objects( objects )
    , m_glyphs( glyphs )
    , m_plot_over_line( plotOverLine )
    , m_point_size( pointSize )
    , m_repetition( repetition )
    , m_selector( this )
{
    setObjectName( "SimpleSettingWidget" );
    auto* container = new QWidget( this );
    auto* layout = new QVBoxLayout( container );
    layout->addWidget( new QLabel( tr( "可視化対象選択" ), container ) );
    m_targets = new QButtonGroup( this );
    m_targets->setExclusive( true );
    for ( int i = 0; i < 3; ++i )
    {
        auto* radio = new QRadioButton( QString::fromUtf8( targets[i].label ), container );
        m_targets->addButton( radio, i );
        layout->addWidget( radio );
        if ( i == 0 ) radio->setChecked( true );
    }
    m_target_apply = new QPushButton( tr( "Apply" ), container );
    layout->addWidget( m_target_apply );
    layout->addWidget( new QLabel( tr( "Color Map選択" ), container ) );
    m_preview = new ColorMap( container, QVector<QColor>( 2, QColor( Qt::black ) ) );
    m_preview->setMinimumHeight( 24 );
    layout->addWidget( m_preview );
    m_color_select = new QPushButton( tr( "Color Map選択" ), container );
    layout->addWidget( m_color_select );
    m_color_apply = new QPushButton( tr( "Apply" ), container );
    layout->addWidget( m_color_apply );
    layout->addStretch();
    setWidget( container );
    m_selector.setPresetOnly();
    m_timeout.setSingleShot( true );

    connect( m_target_apply, &QPushButton::clicked, this, &SimpleSettingWidget::applyTarget );
    connect( m_color_select, &QPushButton::clicked, this, [this]() {
        m_selector.setDefaultColorMap( m_preview->getColors() );
        const int target = m_applied_target;
        const quint64 generation = m_sockets->generation();
        if ( m_selector.exec() == QDialog::Accepted && m_stage == Stage::Idle &&
             target == m_applied_target && generation == m_sockets->generation() && target >= 0 )
        {
            m_preview->setColors( m_selector.getColorMap() );
            m_pending_color = true;
            updateControls();
        }
    } );
    connect( m_color_apply, &QPushButton::clicked, this, [this]() {
        if ( m_stage != Stage::Idle || m_external_initializing || m_applied_target < 0 ) return;
        m_last_error.clear();
        m_generation = m_sockets->generation();
        m_stage = Stage::ApplyingColorMap;
        beginOperation();
        updateControls();
        if ( !m_editor->applyColorMap( appliedColorFunction(), m_preview->getColors() ) )
        {
            finishOperation();
            setStatus( tr( "Color Mapを適用できません。接続・操作権限・対象C番号を確認してください。" ), true );
            return;
        }
        if ( m_stage != Stage::ApplyingColorMap ) return;
        m_pending_color = false;
        refreshColorMap();
        startAutomaticJump();
    } );
    connect( m_sockets, &WebSocketPair::stateChanged, this, [this]( WebSocketPair::State state ) {
        if ( state != WebSocketPair::State::Connected ) invalidateTarget();
        updateControls();
    } );
    // Communication's earlier disconnected slot performs the existing object/editor cleanup first.
    connect( m_sockets, &WebSocketPair::disconnected, this, [this]( quint64, const QString& reason ) {
        m_external_initializing = false;
        invalidateTarget();
        if ( m_stage == Stage::Disconnecting ) startConnection();
        else if ( m_stage != Stage::Idle ) fail( reason.isEmpty() ? tr( "接続が切断されました。" ) : reason );
        updateControls();
    } );
    connect( m_sockets, &WebSocketPair::errorOccurred, this, [this]( const QString& reason ) {
        if ( m_stage != Stage::Idle && m_stage != Stage::Disconnecting )
        {
            fail( reason );
            m_communication->disconnectFromServer();
        }
    } );
    // Both sockets must be connected. This server confirms operator privilege during Initialize.
    connect( m_communication, &Communication::connectionReady, this, [this]( bool isOperator ) {
        if ( m_stage != Stage::Connecting || m_generation != m_sockets->generation() ) return;
        if ( !isOperator ) { fail( tr( "サーバーの操作権限がありません。" ) ); return; }
        m_stage = Stage::Initializing;
        m_timeout.start( 300000 );
        setStatus( tr( "可視化対象を読み込んでいます。" ) );
        m_communication->applySettings();
    } );
    connect( m_communication, &Communication::initializationStarted, this, [this]() {
        if ( m_stage == Stage::Idle ) m_external_initializing = true;
        invalidateTarget();
        updateControls();
    } );
    connect( m_communication, &Communication::initializationFinished,
             this, &SimpleSettingWidget::finishInitialization );
    connect( m_communication, &Communication::updateServerState, this, [this]( bool ) {
        // The existing MainWindow handler resets the dataset on this notification.
        if ( m_stage == Stage::WaitingForObjects || m_stage == Stage::AddingTerrain ||
             m_stage == Stage::ApplyingColorMap || m_stage == Stage::WaitingForJump || m_stage == Stage::WaitingForJumpData )
            fail( tr( "サーバー状態が変更されたため、自動設定を中止しました。対象Applyで再読み込みしてください。" ) );
        invalidateTarget();
        updateControls();
    } );
    connect( m_communication, &Communication::updateOperatorState, this, [this]( bool isOperator ) {
        if ( !isOperator && m_stage == Stage::WaitingForObjects )
            fail( tr( "自動設定中に操作権限を失いました。" ) );
        else if ( !isOperator && ( m_stage == Stage::ApplyingColorMap || m_stage == Stage::WaitingForJump || m_stage == Stage::WaitingForJumpData ) )
            fail( tr( "自動Jump中に操作権限を失いました。" ) );
        else if ( !isOperator && m_stage == Stage::AddingTerrain )
            finishTerrainAddition( false, tr( "地形追加中に操作権限を失いました。" ) );
        else if ( !isOperator && m_stage == Stage::Initializing )
        {
            fail( tr( "読み込み中に操作権限を失いました。" ) );
            m_communication->disconnectFromServer();
        }
        updateControls();
    } );
    connect( m_editor, &TransferFunctionEditor::colorMapsChanged, this, &SimpleSettingWidget::refreshColorMap );
    // Queue until the complete row-registration handler has finished updating ObjectEditor.
    connect( m_objects, &ObjectEditor::objectAdded, this, [this]( const QString& uuid ) {
        if ( m_generation != m_sockets->generation() || !m_sockets->isConnected() ) return;
        if ( m_stage == Stage::WaitingForObjects ) applyInitialSettings();
        else if ( m_stage == Stage::AddingTerrain && uuid == m_terrain_uuid )
        {
            const bool success = m_objects->configureTerrainAndApply( uuid );
            finishTerrainAddition( success, tr( "地形がKVSMLまたはSTLポリゴンでないか、設定を適用できませんでした。" ) );
        }
    }, Qt::QueuedConnection );
    connect( m_glyphs, &GlyphEditor::settingsReady, this, &SimpleSettingWidget::applyInitialSettings,
             Qt::QueuedConnection );
    connect( &m_timeout, &QTimer::timeout, this, [this]() {
        if ( m_stage == Stage::WaitingForJump || m_stage == Stage::WaitingForJumpData )
        {
            fail( tr( "自動Jumpのデータ要求完了待ちがタイムアウトしました。対象Applyで再読み込みしてください。" ) );
            return;
        }
        if ( m_stage == Stage::WaitingForObjects )
        {
            fail( tr( "Object一覧またはGlyph初期設定の受信待ちがタイムアウトしました。対象Applyで再読み込みしてください。" ) );
            return;
        }
        if ( m_stage == Stage::AddingTerrain )
        {
            finishTerrainAddition( false, tr( "地形追加の完了待ちがタイムアウトしました。サーバー側のファイルを確認してください: %1" ).arg( QString::fromUtf8( targets[m_applied_target].terrainPath ) ) );
            return;
        }
        fail( tr( "接続または初期化の完了待ちがタイムアウトしました。" ) );
        m_communication->disconnectFromServer();
    } );
    setStatus( tr( "可視化対象を選択し、Applyを押してください。" ) );
    updateControls();
}

int SimpleSettingWidget::appliedColorFunction() const
{
    return m_applied_target < 0 ? 0 : targets[m_applied_target].colorFunction;
}

void SimpleSettingWidget::applyTarget()
{
    const auto state = m_sockets->state();
    if ( m_stage != Stage::Idle || m_external_initializing ||
         ( state != WebSocketPair::State::Disconnected && state != WebSocketPair::State::Connected ) ||
         ( m_sockets->isConnected() && !m_communication->isOperator() ) ) return;
    m_requested_target = m_targets->checkedId();
    m_last_error.clear();
    beginOperation();
    invalidateTarget();
    if ( m_sockets->isConnected() )
    {
        m_stage = Stage::Disconnecting;
        m_timeout.start( 30000 );
        setStatus( tr( "サーバーとの切断を待っています。" ) );
        updateControls();
        m_communication->disconnectFromServer();
    }
    else startConnection();
}

void SimpleSettingWidget::startConnection()
{
    const auto& target = targets[m_requested_target];
    m_communication->configureRemoteVisualization( QString::fromUtf8( target.volumePath ),
                                                   QString::fromUtf8( target.transferFunctionPath ) );
    m_communication->setOperationInProgress( true );
    m_stage = Stage::Connecting;
    m_timeout.start( 30000 );
    setStatus( tr( "サーバーに接続しています。" ) );
    updateControls();
    m_communication->connectToServer();
    m_generation = m_sockets->generation();
}

void SimpleSettingWidget::finishInitialization( bool success, const QString& reason )
{
    m_external_initializing = false;
    updateControls();
    if ( m_stage != Stage::Initializing || m_generation != m_sockets->generation() ) return;
    if ( !success ) { fail( reason.isEmpty() ? tr( "初期化に失敗しました。" ) : reason ); return; }
    m_timeout.stop();
    m_applied_target = m_requested_target;
    m_requested_target = -1;
    m_pending_color = false;
    setStatus( tr( "%1を読み込みました。" ).arg( QString::fromUtf8( targets[m_applied_target].label ) ) );
    if ( m_editor->hasColorFunction( appliedColorFunction() ) )
        emit selectColorFunction( appliedColorFunction() );
    refreshColorMap();
    m_stage = Stage::WaitingForObjects;
    m_timeout.start( 300000 );
    setStatus( tr( "初期化しました。Object一覧とGlyph初期設定の受信を待っています。" ) );
    updateControls();
    applyInitialSettings();
}

void SimpleSettingWidget::applyInitialSettings()
{
    if ( m_stage != Stage::WaitingForObjects || m_applied_target < 0 ||
         m_generation != m_sockets->generation() || !m_sockets->isConnected() ||
         !m_objects->hasServerVolumeObject() || !m_glyphs->hasInitialSettings() ) return;
    const bool cityLBM = m_applied_target == 0 || m_applied_target == 1;
    const double scaleFactor = m_applied_target == 1 ? 10.0 : ( cityLBM ? 50.0 : 0.1 );
    if ( !m_glyphs->applySimpleSettings( cityLBM, scaleFactor ) )
    {
        fail( tr( "Glyph設定を適用できません。q1・q2・q3、既定カラーマップ、操作権限を確認してください。" ) );
        return;
    }
    if ( cityLBM && !m_objects->configureParticleLimitAndApply( 200000000 ) )
    {
        fail( tr( "CityLBMのParticle Limitを適用できませんでした。" ) );
        return;
    }
    if ( !cityLBM && !m_objects->configureParticleLimitAndApply( 60000000 ) )
    {
        fail( tr( "冷却対流のParticle Limitを適用できませんでした。" ) );
        return;
    }
    // RepetitionLevel recreates particle renderers; update PointSize afterwards.
    m_repetition->setRepetitionLevel( 4 );
    m_point_size->resetPointSize();
    if ( cityLBM ) startTerrainAddition();
    else
    {
        refreshColorMap();
        startAutomaticJump();
    }
}

void SimpleSettingWidget::startTerrainAddition()
{
    if ( m_stage != Stage::WaitingForObjects || !m_objects->hasServerVolumeObject() ) return;
    m_stage = Stage::AddingTerrain;
    m_terrain_uuid = QUuid::createUuid().toString( QUuid::WithoutBraces );
    m_timeout.start( 300000 );
    setStatus( tr( "%1の地形を追加しています。" ).arg( QString::fromUtf8( targets[m_applied_target].label ) ) );
    if ( !m_objects->addFile( QString::fromUtf8( targets[m_applied_target].terrainPath ), m_terrain_uuid ) )
        finishTerrainAddition( false, tr( "地形の追加要求を送信できませんでした。" ) );
}

void SimpleSettingWidget::finishTerrainAddition( bool success, const QString& reason )
{
    if ( success )
    {
        startAutomaticJump();
        return;
    }
    finishOperation();
    QString message = tr( "可視化データは初期化済みですが、地形の追加・設定に失敗しました。%1 対象Applyで再読み込みできます。" ).arg( reason );
    // Preserve the missing-color-function diagnostic if the loaded TFE lacks the fixed C number.
    if ( m_applied_target >= 0 && !m_editor->hasColorFunction( appliedColorFunction() ) )
        message += tr( " C%1が存在しないためColor Mapは適用できません。" ).arg( appliedColorFunction() );
    setStatus( message, true );
}

void SimpleSettingWidget::startAutomaticJump()
{
    if ( m_stage != Stage::WaitingForObjects && m_stage != Stage::AddingTerrain &&
         m_stage != Stage::ApplyingColorMap ) return;
    if ( m_generation != m_sockets->generation() || !m_sockets->isConnected() ||
         !m_communication->isOperator() || m_applied_target < 0 )
    {
        fail( tr( "接続または操作権限がないため、自動Jumpを開始できません。" ) );
        return;
    }
    const bool completingTargetLoad = m_stage != Stage::ApplyingColorMap;
    if ( completingTargetLoad )
    {
        kvs::Vec3 start( 0.0f, 0.0f, 0.0f );
        kvs::Vec3 end( 0.0f, 0.0f, 0.8f );
        if ( m_applied_target == 0 )
        {
            start = kvs::Vec3( -100.0f, -200.0f, 100.0f );
            end = kvs::Vec3( 400.0f, -200.0f, 100.0f );
        }
        else if ( m_applied_target == 1 )
        {
            start = kvs::Vec3( 15.0f, 150.0f, 35.0f );
            end = kvs::Vec3( 15.0f, 100.0f, 35.0f );
        }
        if ( !m_plot_over_line->configureCoordsAndApply( start, end ) )
        {
            fail( tr( "Plot Over Lineを適用できません。接続・操作権限・対象変数を確認してください。" ) );
            return;
        }
        if ( m_generation != m_sockets->generation() || !m_sockets->isConnected() ||
             ( m_stage != Stage::WaitingForObjects && m_stage != Stage::AddingTerrain ) ) return;
    }
    m_jump_completes_target_load = completingTargetLoad;
    m_stage = Stage::WaitingForJump;
    m_timeout.start( 300000 );
    setStatus( tr( "各設定を適用しました。データ要求中の場合は完了後に自動Jumpを実行します。" ) );
    updateControls();
    emit automaticJumpRequested();
}

void SimpleSettingWidget::onAutomaticJumpStarted()
{
    if ( m_stage != Stage::WaitingForJump || m_generation != m_sockets->generation() ||
         !m_sockets->isConnected() || !m_communication->isOperator() ) return;
    // Set the stage before dispatching Jump, including requests that finish synchronously.
    m_stage = Stage::WaitingForJumpData;
    m_timeout.start( 300000 );
    setStatus( tr( "自動Jumpを実行しています。データ要求完了を待っています。" ) );
}

void SimpleSettingWidget::onAutomaticJumpFailed()
{
    if ( m_stage == Stage::WaitingForJump || m_stage == Stage::WaitingForJumpData )
        fail( tr( "自動Jumpを開始できません。接続・操作権限・再生操作の状態を確認してください。" ) );
}

void SimpleSettingWidget::onDataRequestCompleted( int timeStep )
{
    if ( m_generation != m_sockets->generation() || !m_sockets->isConnected() ||
         !m_communication->isOperator() || m_applied_target < 0 ) return;
    if ( m_stage == Stage::WaitingForJump )
    {
        // Leave the preceding request's completion handler before starting another request.
        const quint64 generation = m_generation;
        QTimer::singleShot( 0, this, [this, generation]() {
            if ( m_stage == Stage::WaitingForJump && generation == m_generation &&
                 generation == m_sockets->generation() && m_sockets->isConnected() &&
                 m_communication->isOperator() )
                emit automaticJumpRequested();
        } );
        return;
    }
    if ( m_stage != Stage::WaitingForJumpData ) return;
    const QString target = QString::fromUtf8( targets[m_applied_target].label );
    const bool completedTargetLoad = m_jump_completes_target_load;
    finishOperation();
    setStatus( tr( "%1の各設定と自動Jumpが完了しました（Time Step=%2）。" ).arg( target ).arg( timeStep ) );
    refreshColorMap();
    if ( completedTargetLoad ) emit targetLoadCompleted();
}

void SimpleSettingWidget::beginOperation()
{
    m_communication->setOperationInProgress( true );
    m_editor->setEnabled( false );
    m_objects->setEnabled( false );
    m_glyphs->setEnabled( false );
    m_plot_over_line->setEnabled( false );
    m_point_size->setEnabled( false );
    m_repetition->setEnabled( false );
}

void SimpleSettingWidget::finishOperation()
{
    m_timeout.stop();
    m_stage = Stage::Idle;
    m_jump_completes_target_load = false;
    m_terrain_uuid.clear();
    m_communication->setOperationInProgress( false );
    m_editor->setEnabled( true );
    m_objects->setEnabled( true );
    m_glyphs->setEnabled( true );
    m_plot_over_line->setEnabled( true );
    m_point_size->setEnabled( true );
    m_repetition->setEnabled( true );
    updateControls();
}

void SimpleSettingWidget::fail( const QString& reason )
{
    finishOperation();
    m_requested_target = -1;
    invalidateTarget();
    setStatus( reason, true );
    updateControls();
}

void SimpleSettingWidget::invalidateTarget()
{
    m_applied_target = -1;
    m_terrain_uuid.clear();
    m_pending_color = false;
    m_selector.reject();
    m_preview->setColors( QVector<QColor>( 2, QColor( Qt::black ) ) );
}

void SimpleSettingWidget::refreshColorMap()
{
    if ( m_applied_target >= 0 )
    {
        const int number = appliedColorFunction();
        if ( !m_editor->hasColorFunction( number ) || m_editor->colorMap( number ).isEmpty() )
        {
            m_pending_color = false;
            m_selector.reject();
            m_preview->setColors( QVector<QColor>( 2, QColor( Qt::black ) ) );
            setStatus( tr( "伝達関数にC%1のColor Mapが存在しないため適用できません。" ).arg( number ), true );
        }
        else if ( !m_pending_color ) m_preview->setColors( m_editor->colorMap( number ) );
    }
    updateControls();
}

void SimpleSettingWidget::updateControls()
{
    const bool idle = m_stage == Stage::Idle && !m_external_initializing;
    const bool connected = m_sockets->isConnected();
    const bool allowed = !connected || m_communication->isOperator();
    const auto state = m_sockets->state();
    m_target_apply->setEnabled( idle && allowed &&
        ( state == WebSocketPair::State::Disconnected || state == WebSocketPair::State::Connected ) );
    for ( auto* button : m_targets->buttons() ) button->setEnabled( idle );
    const bool colorAllowed = idle && connected && allowed && m_applied_target >= 0 &&
        m_editor->hasColorFunction( appliedColorFunction() ) &&
        !m_editor->colorMap( appliedColorFunction() ).isEmpty();
    m_color_select->setEnabled( colorAllowed );
    m_color_apply->setEnabled( colorAllowed );
    m_preview->setEnabled( colorAllowed );
}

void SimpleSettingWidget::setStatus( const QString& message, bool error )
{
    emit updateStatusBarMessage( message );
    if ( error && message != m_last_error )
    {
        m_last_error = message;
        auto* dialog = new QMessageBox( QMessageBox::Warning, windowTitle(), message,
                                        QMessageBox::Ok, this );
        dialog->setAttribute( Qt::WA_DeleteOnClose );
        dialog->open();
    }
}

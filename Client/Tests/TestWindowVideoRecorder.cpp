#include "TestWindowVideoRecorder.h"

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QMediaFormat>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPixmap>
#include <QUrl>
#include <QWidget>

#include <algorithm>
#include <functional>

namespace
{
constexpr int k_frame_interval_ms = 100; // 10 fps
constexpr int k_start_timeout_ms = 5000;
constexpr int k_finish_timeout_ms = 15000;

// Qt Multimedia の準備・保存完了を待つ間も GUI イベントを処理する。
bool waitUntil( const std::function<bool()>& condition, int timeout_ms )
{
    if ( condition() ) { return true; }
    QEventLoop loop;
    QTimer poll_timer;
    QObject::connect( &poll_timer, &QTimer::timeout, &loop,
                      [&]() { if ( condition() ) { loop.quit(); } } );
    QTimer::singleShot( timeout_ms, &loop, &QEventLoop::quit );
    poll_timer.start( 10 );
    loop.exec();
    return condition();
}
}

namespace ClientTests
{

TestWindowVideoRecorder::TestWindowVideoRecorder()
{
    m_session.setVideoFrameInput( &m_input );
    m_session.setRecorder( &m_recorder );
    m_recorder.setAutoStop( true );
    m_timer.setInterval( k_frame_interval_ms );
    m_timer.setTimerType( Qt::PreciseTimer );
    connect( &m_timer, &QTimer::timeout, this, [this]() { captureFrame(); } );
    // sendVideoFrame 内の通知で再帰しないよう、再送はイベントループに戻して行う。
    connect( &m_input, &QVideoFrameInput::readyToSendVideoFrame,
             this, [this]() { sendPendingFrame(); }, Qt::QueuedConnection );
    connect( &m_recorder, &QMediaRecorder::recorderStateChanged, this,
             [this]( QMediaRecorder::RecorderState state )
             {
                 m_messages << QStringLiteral( "recorderState=%1" ).arg( int( state ) );
                 if ( state == QMediaRecorder::RecordingState ) { sendPendingFrame(); }
                 else if ( state == QMediaRecorder::StoppedState && m_started && !m_finishing )
                 {
                     fail( QStringLiteral( "停止要求前に動画エンコーダーが終了しました。" ) );
                 }
             } );
    connect( &m_recorder, &QMediaRecorder::errorOccurred, this,
             [this]( QMediaRecorder::Error error, const QString& message )
             {
                 fail( QStringLiteral( "Qt Multimedia エラー %1: %2" ).arg( int( error ) ).arg( message ) );
             } );
}

TestWindowVideoRecorder::~TestWindowVideoRecorder()
{
    if ( m_started && !m_finished ) { finish(); }
    disconnect( &m_timer, nullptr, this, nullptr );
    disconnect( &m_input, nullptr, this, nullptr );
    disconnect( &m_recorder, nullptr, this, nullptr );
    m_session.setVideoFrameInput( nullptr );
    m_session.setRecorder( nullptr );
}

void TestWindowVideoRecorder::fail( const QString& reason )
{
    if ( m_failure_reason.isEmpty() ) { m_failure_reason = reason; }
    m_messages << reason;
    m_timer.stop();
    qWarning().noquote() << reason;
}

bool TestWindowVideoRecorder::start( QWidget* window, const QString& file_path )
{
    // このオブジェクトは 1 回の録画に使用する。
    if ( m_started || m_finished ) { return false; }
    m_window = window;
    m_file_path = file_path;
    if ( !window || !window->isVisible() || window->width() < 2 || window->height() < 2 )
    {
        fail( QStringLiteral( "録画対象のウィンドウが表示されていないか、サイズが無効です。" ) );
        return false;
    }
    // H.264 用に偶数サイズへ揃え、Retina の物理ピクセル数による負荷増大を避ける。
    m_frame_size = QSize( window->width() & ~1, window->height() & ~1 );
    m_messages << QStringLiteral( "backend=%1, format=MP4/H.264, resolution=%2x%3, fps=10, DPR=%4" )
        .arg( qEnvironmentVariable( "QT_MEDIA_BACKEND" ) )
        .arg( m_frame_size.width() ).arg( m_frame_size.height() ).arg( window->devicePixelRatioF() );

    QMediaFormat format( QMediaFormat::MPEG4 );
    format.setVideoCodec( QMediaFormat::VideoCodec::H264 );
    format.setAudioCodec( QMediaFormat::AudioCodec::Unspecified );
    if ( !m_recorder.isAvailable() || !format.isSupported( QMediaFormat::Encode ) )
    {
        fail( QStringLiteral( "MP4/H.264 の動画保存を利用できません。Qt Multimedia の FFmpeg バックエンドとエンコーダーを確認してください。" ) );
        return false;
    }
    if ( QFileInfo::exists( file_path ) && !QFile::remove( file_path ) )
    {
        fail( QStringLiteral( "既存の動画ファイルを削除できません: %1" ).arg( file_path ) );
        return false;
    }
    m_recorder.setMediaFormat( format );
    m_recorder.setVideoResolution( m_frame_size );
    m_recorder.setVideoFrameRate( 1000.0 / k_frame_interval_ms );
    m_recorder.setQuality( QMediaRecorder::HighQuality );
    m_recorder.setOutputLocation( QUrl::fromLocalFile( file_path ) );
    m_started = true;
    m_clock.start();
    captureFrame(); // 最初のフレームを準備してからエンコーダーを起動する。
    if ( !m_failure_reason.isEmpty() ) { return false; }
    m_recorder.record();
    const bool ready = waitUntil(
        [this]() { return m_sent_frames > 0 || !m_failure_reason.isEmpty(); }, k_start_timeout_ms );
    if ( !ready ) { fail( QStringLiteral( "動画エンコーダーが制限時間内に最初のフレームを受け付けませんでした。" ) ); }
    if ( !m_failure_reason.isEmpty() ) { return false; }
    m_timer.start();
    return true;
}

void TestWindowVideoRecorder::captureFrame()
{
    if ( !m_failure_reason.isEmpty() ) { return; }
    if ( m_pending_frame.isValid() )
    {
        ++m_skipped_frames; // 未送信フレームは 1 枚まで。GUI を待たせない。
        return;
    }
    if ( !m_window )
    {
        fail( QStringLiteral( "録画中に対象ウィンドウが破棄されました。" ) );
        return;
    }
    QImage image = m_window->grab().toImage();
    if ( image.isNull() )
    {
        fail( QStringLiteral( "ウィンドウの UI 描画を取得できませんでした。" ) );
        return;
    }
    // QWidget::grab だけでは OpenGL 部分が取得できないため明示的に合成する。
    QPainter painter( &image );
    const auto gl_widgets = m_window->findChildren<QOpenGLWidget*>();
    int gl_count = 0;
    for ( auto* widget : gl_widgets )
    {
        if ( widget->window() != m_window->window() || !widget->isVisibleTo( m_window.data() ) ) { continue; }
        const QImage framebuffer = widget->grabFramebuffer();
        if ( framebuffer.isNull() )
        {
            painter.end();
            fail( QStringLiteral( "OpenGL 表示のフレームバッファーを取得できませんでした。" ) );
            return;
        }
        const QPoint position = widget->mapTo( m_window.data(), QPoint( 0, 0 ) );
        painter.save();
        painter.setClipRegion( widget->visibleRegion().translated( position ) );
        painter.drawImage( QRect( position, widget->size() ), framebuffer );
        painter.restore();
        ++gl_count;
    }
    if ( gl_count == 0 )
    {
        painter.end();
        fail( QStringLiteral( "録画対象内に表示中の OpenGL ウィジェットが見つかりませんでした。" ) );
        return;
    }

    // QDialog は親付きでも別ウィンドウなので、メインウィンドウの grab には含まれない。
    // 対象ウィンドウに属する Qt ダイアログだけを個別に取得する。
    QList<QDialog*> dialogs;
    for ( QWidget* widget : QApplication::topLevelWidgets() )
    {
        auto* dialog = qobject_cast<QDialog*>( widget );
        if ( dialog && dialog->isVisible() && !dialog->isMinimized() &&
             m_window->isAncestorOf( dialog ) )
        {
            dialogs.append( dialog );
        }
    }
    const auto parent_depth = []( const QWidget* widget )
    {
        int depth = 0;
        for ( auto* parent = widget->parentWidget(); parent; parent = parent->parentWidget() ) { ++depth; }
        return depth;
    };
    QWidget* active_modal = QApplication::activeModalWidget();
    std::stable_sort( dialogs.begin(), dialogs.end(),
                     [active_modal, &parent_depth]( const QDialog* left, const QDialog* right )
                     {
                         if ( left == right ) { return false; }
                         if ( left == active_modal ) { return false; }
                         if ( right == active_modal ) { return true; }
                         return parent_depth( left ) < parent_depth( right );
                     } );
    for ( QDialog* dialog : dialogs )
    {
        const QImage dialog_image = dialog->grab().toImage();
        if ( dialog_image.isNull() )
        {
            painter.end();
            fail( QStringLiteral( "ダイアログの描画を取得できませんでした: %1" ).arg( dialog->windowTitle() ) );
            return;
        }
        QSize size = dialog->size();
        if ( size.width() > m_window->width() || size.height() > m_window->height() )
        {
            size.scale( m_window->size(), Qt::KeepAspectRatio );
        }
        QPoint position = m_window->mapFromGlobal( dialog->mapToGlobal( QPoint( 0, 0 ) ) );
        // 外側へはみ出す場合は録画画像内に収める。実際のダイアログは移動しない。
        position.setX( qBound( 0, position.x(), m_window->width() - size.width() ) );
        position.setY( qBound( 0, position.y(), m_window->height() - size.height() ) );
        painter.drawImage( QRect( position, size ), dialog_image );
        const QString message = QStringLiteral( "合成対象ダイアログ: %1" ).arg( dialog->windowTitle() );
        if ( !m_messages.contains( message ) ) { m_messages << message; }
    }
    if ( !dialogs.isEmpty() ) { ++m_dialog_frames; }
    painter.end();
    image.setDevicePixelRatio( 1.0 );
    // 録画中にウィンドウサイズが変わっても動画の解像度を固定する。
    const QImage scaled = image.scaled( m_frame_size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
    QImage frame_image( m_frame_size, QImage::Format_RGBA8888 );
    frame_image.fill( Qt::black );
    QPainter frame_painter( &frame_image );
    frame_painter.drawImage( QPoint( ( m_frame_size.width() - scaled.width() ) / 2,
                                    ( m_frame_size.height() - scaled.height() ) / 2 ), scaled );
    frame_painter.end();
    m_last_image = frame_image;
    queueImage( frame_image );
}

void TestWindowVideoRecorder::queueImage( const QImage& image )
{
    m_pending_frame = QVideoFrame( image );
    if ( !m_pending_frame.isValid() )
    {
        fail( QStringLiteral( "取得画像を動画フレームに変換できませんでした。" ) );
        return;
    }
    const qint64 timestamp = m_sent_frames == 0 ? 0
        : qMax( m_last_timestamp_us + 1, m_clock.nsecsElapsed() / 1000 );
    m_pending_frame.setStartTime( timestamp );
    m_pending_frame.setEndTime( timestamp + k_frame_interval_ms * 1000 );
    sendPendingFrame();
}

void TestWindowVideoRecorder::sendPendingFrame()
{
    if ( !m_failure_reason.isEmpty() || m_recorder.recorderState() != QMediaRecorder::RecordingState ) { return; }
    if ( m_pending_frame.isValid() )
    {
        if ( !m_input.sendVideoFrame( m_pending_frame ) ) { return; }
        m_last_timestamp_us = m_pending_frame.startTime();
        m_pending_frame = QVideoFrame();
        ++m_sent_frames;
    }
    if ( m_finishing && !m_end_sent )
    {
        // 空フレームは入力終了。autoStop によりキューを処理してから保存を完了する。
        m_end_sent = m_input.sendVideoFrame( QVideoFrame() );
    }
}

bool TestWindowVideoRecorder::finish()
{
    if ( m_finished ) { return m_success; }
    m_timer.stop();
    if ( m_started && m_failure_reason.isEmpty() )
    {
        if ( !waitUntil( [this]() { return !m_pending_frame.isValid() || !m_failure_reason.isEmpty(); }, k_start_timeout_ms ) )
        {
            fail( QStringLiteral( "未送信の動画フレームを制限時間内に送信できませんでした。" ) );
        }
        // 最後の描画を停止時刻でも送信し、遅い取得環境でも動画の実時間を保つ。
        if ( m_failure_reason.isEmpty() && !m_last_image.isNull() ) { queueImage( m_last_image ); }
        m_finishing = true;
        sendPendingFrame();
    }
    else
    {
        m_finishing = true;
    }
    if ( !m_failure_reason.isEmpty() ) { m_recorder.stop(); }
    const bool stopped = waitUntil(
        [this]() { return m_recorder.recorderState() == QMediaRecorder::StoppedState; }, k_finish_timeout_ms );
    if ( !stopped )
    {
        fail( QStringLiteral( "動画ファイルの保存が制限時間内に完了しませんでした。" ) );
        m_recorder.stop();
        waitUntil( [this]() { return m_recorder.recorderState() == QMediaRecorder::StoppedState; }, k_start_timeout_ms );
    }
    const QFileInfo file_info( m_file_path );
    m_success = m_started && stopped && m_end_sent && m_sent_frames > 0 &&
                m_failure_reason.isEmpty() && file_info.exists() && file_info.size() > 0;
    if ( !m_success && m_failure_reason.isEmpty() )
    {
        fail( QStringLiteral( "録画を正常完了できなかったか、空でない動画ファイルが生成されませんでした。" ) );
    }
    m_finished = true;
    qInfo().noquote() << diagnostics();
    return m_success;
}

QString TestWindowVideoRecorder::diagnostics() const
{
    return QStringLiteral( "方式: アプリ内描画 + Qt Multimedia（音声なし）\n"
                           "送信フレーム数: %1\n取得見送り回数: %2\n動画時間: %3 ms\n"
                           "保存完了: %4\n保存先: %5\n%6" )
        .arg( m_sent_frames ).arg( m_skipped_frames ).arg( m_recorder.duration() )
        .arg( m_success ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) )
        .arg( m_file_path, m_messages.join( QLatin1Char( '\n' ) ) )
        + QStringLiteral( "\nダイアログを含むフレーム取得数: %1" ).arg( m_dialog_frames );
}

} // namespace ClientTests

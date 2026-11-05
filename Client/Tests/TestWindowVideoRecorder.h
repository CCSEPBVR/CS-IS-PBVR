#ifndef TESTWINDOWVIDEORECORDER_H
#define TESTWINDOWVIDEORECORDER_H

#include <QElapsedTimer>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QTimer>
#include <QVideoFrame>
#include <QVideoFrameInput>

class QWidget;

namespace ClientTests
{

// 自アプリの描画だけを取得する。OS の画面収録 API は使用しない。
class TestWindowVideoRecorder : public QObject
{
public:
    TestWindowVideoRecorder();
    ~TestWindowVideoRecorder() override;

    bool start( QWidget* window, const QString& file_path );
    bool finish();
    QString failureReason() const { return m_failure_reason; }
    QString diagnostics() const;

private:
    void captureFrame();
    void queueImage( const QImage& image );
    void sendPendingFrame();
    void fail( const QString& reason );

    QMediaCaptureSession m_session;
    QMediaRecorder m_recorder;
    QVideoFrameInput m_input;
    QTimer m_timer;
    QPointer<QWidget> m_window;
    QElapsedTimer m_clock;
    QImage m_last_image;
    QVideoFrame m_pending_frame;
    QSize m_frame_size;
    QString m_file_path;
    QString m_failure_reason;
    QStringList m_messages;
    qint64 m_last_timestamp_us = -1;
    int m_sent_frames = 0;
    int m_skipped_frames = 0;
    int m_dialog_frames = 0;
    bool m_started = false;
    bool m_finishing = false;
    bool m_end_sent = false;
    bool m_finished = false;
    bool m_success = false;
};

} // namespace ClientTests

#endif // TESTWINDOWVIDEORECORDER_H

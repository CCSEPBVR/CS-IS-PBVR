#ifndef PLAYBACKCONTROLTOOLBARTEST_H
#define PLAYBACKCONTROLTOOLBARTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <memory>

class QFileDialog;
class QMainWindow;
class QPushButton;
class QSpinBox;
class QToolBar;
class QWidget;

namespace ClientTests
{
class TestWindowVideoRecorder;
class PlayBackControlToolBarTest : public QObject
{
    Q_OBJECT

public:
    PlayBackControlToolBarTest();
    ~PlayBackControlToolBarTest() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_playback_control_toolbar_scenario();

private:
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path ) const;
    void clickButtonAndWait( QPushButton* button, int wait_ms ) const;
    QSpinBox* findNextTimeStepSpinBox( QToolBar* tool_bar ) const;
    void startVideoRecording( QWidget* target_window );
    bool stopVideoRecording();
    void writeMarkdownReport() const;
    QProcess m_recording_process;
#ifdef Q_OS_MACOS
    std::unique_ptr<TestWindowVideoRecorder> m_window_recorder;
#endif
    QString m_client_executable;
    QString m_object_file_path;
    QString m_output_dir_path;
    QString m_video_file_path;
    QString m_report_path;
    QString m_recording_failure_reason;
    QString m_recorder_output;
    QString m_recorder_exit_details;
    bool m_recording_attempted = false;
    bool m_recording_finalized = false;
    bool m_video_created = false;
    bool m_video_recording_available = false;
    bool m_test_succeeded = false;
};
}

#endif // PLAYBACKCONTROLTOOLBARTEST_H

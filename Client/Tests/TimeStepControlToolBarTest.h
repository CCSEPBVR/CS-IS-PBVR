#ifndef TIMESTEPCONTROLTOOLBARTEST_H
#define TIMESTEPCONTROLTOOLBARTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <memory>

class QFileDialog;
class QPushButton;
class QSpinBox;
class QToolBar;
class QWidget;

namespace ClientTests
{
class TestWindowVideoRecorder;
class TimeStepControlToolBarTest : public QObject
{
    Q_OBJECT

public:
    TimeStepControlToolBarTest();
    ~TimeStepControlToolBarTest() override;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_time_step_control_toolbar_scenario();

private:
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path ) const;
    void clickButtonAndWait( QPushButton* button, int wait_ms ) const;
    QList<QSpinBox*> findSpinBoxes( QToolBar* tool_bar ) const;
    void startVideoRecording( QWidget* target_window );
    void stopVideoRecording();

#ifdef Q_OS_MACOS
    void writeMarkdownReport() const;
    std::unique_ptr<TestWindowVideoRecorder> m_window_recorder;
    QString m_report_path;
    QString m_recording_failure_reason;
    QString m_recording_diagnostics;
    bool m_recording_attempted = false;
    bool m_recording_finalized = false;
    bool m_video_created = false;
    bool m_test_succeeded = false;
#endif
    QProcess m_recording_process;
    QString m_client_executable;
    QString m_object_file_path;
    QString m_output_dir_path;
    QString m_video_file_path;
    bool m_video_recording_available = false;
};
}

#endif // TIMESTEPCONTROLTOOLBARTEST_H

#ifndef TIMESTEPCONTROLTOOLBARTEST_H
#define TIMESTEPCONTROLTOOLBARTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>

class QFileDialog;
class QPushButton;
class QSpinBox;
class QToolBar;
class QWidget;

namespace ClientTests
{
class TimeStepControlToolBarTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_time_step_control_toolbar_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path ) const;
    void clickButtonAndWait( QPushButton* button, int wait_ms ) const;
    QList<QSpinBox*> findSpinBoxes( QToolBar* tool_bar ) const;
    void startVideoRecording( QWidget* target_window );
    void stopVideoRecording();

    QProcess m_recording_process;
    QString m_client_executable;
    QString m_object_file_path;
    QString m_output_dir_path;
    QString m_video_file_path;
    bool m_video_recording_available = false;
};
}

#endif // TIMESTEPCONTROLTOOLBARTEST_H

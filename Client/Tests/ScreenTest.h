#ifndef SCREENTEST_H
#define SCREENTEST_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QString>
#include <functional>

class QWidget;
class QMainWindow;

class ScreenTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_screen_interaction_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString defaultServerExecutablePath() const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void dragMouse( QWidget* widget, Qt::MouseButton button, const QPoint& start, const QPoint& end, int steps = 12 ) const;
    void saveScreenshot( QMainWindow& window, const QString& file_name ) const;
    void writeMarkdownReport() const;

    QProcess m_server_process;
    QString m_server_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QStringList m_saved_screenshot_paths;
    bool m_test_succeeded = false;
};

#endif // SCREENTEST_H

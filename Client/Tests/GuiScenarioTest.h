#ifndef GUISCENARIOTEST_H
#define GUISCENARIOTEST_H

#include <QDir>
#include <QProcess>
#include <QObject>
#include <functional>

class GuiScenarioTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_remote_viz_jump_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;

    QProcess m_server_process;
    QString m_server_executable;
    QString m_volume_data_path;
    QString m_screenshot_path;
};

#endif // GUISCENARIOTEST_H

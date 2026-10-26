#ifndef MENUBARTEST_H
#define MENUBARTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QVector>

class QMainWindow;
class QMenu;
class QAction;
class QWidget;

class MenuBarTest : public QObject
{
    Q_OBJECT

private:
    struct ScreenshotEntry
    {
        QString file_name;
        QString caption;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_menu_bar_scenario();

private:
    QAction* findActionByText( QWidget* root, const QString& text ) const;
    void openMenu( QMainWindow& window, QMenu* menu ) const;
    void triggerAction( QAction* action, int settle_ms = 400 ) const;
    void saveScreenshot( const QString& file_name, const QString& caption ) const;
    void writeMarkdownReport() const;

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_transfer_function_path;
    QString m_report_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QVector<ScreenshotEntry> m_screenshots;
    bool m_test_succeeded = false;
};

#endif // MENUBARTEST_H

#ifndef SCREENTEST_H
#define SCREENTEST_H

#include <QObject>
#include <QPoint>
#include <QProcess>
#include <QVector>
#include <QString>
#include <Qt>

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
    struct ScreenshotEntry
    {
        QString path;
        QString caption;
    };

    void dragMouse(
        QWidget* widget,
        Qt::MouseButton button,
        const QPoint& start,
        const QPoint& end,
        Qt::KeyboardModifiers modifiers = Qt::NoModifier,
        int steps = 12 ) const;
    void saveScreenshot( QMainWindow& window, const QString& file_name, const QString& caption ) const;
    void writeMarkdownReport() const;

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QVector<ScreenshotEntry> m_screenshots;
    bool m_test_succeeded = false;
};

#endif // SCREENTEST_H

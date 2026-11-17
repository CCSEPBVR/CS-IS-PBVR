#ifndef PLOTOVERTIMEEDITORTEST_H
#define PLOTOVERTIMEEDITORTEST_H

#include <QObject>
#include <QString>
#include <QVector>

namespace ClientTests
{
class PlotOverTimeEditorTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_plot_over_time_editor_cs_scenario();

private:
    struct StepEntry
    {
        QString description;
        QString result;
    };
    struct ScreenshotEntry
    {
        QString file_name;
        QString caption;
    };

    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;

    QString m_volume_data_path;
    QString m_transfer_function_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QVector<StepEntry> m_steps;
    QVector<ScreenshotEntry> m_screenshots;
    bool m_test_succeeded = false;
};
}

#endif // PLOTOVERTIMEEDITORTEST_H

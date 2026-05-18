#ifndef PREFERENCETEST_H
#define PREFERENCETEST_H

#include <QObject>
#include <QString>
#include <functional>
#include <vector>

class QComboBox;
class QColor;
class QColorDialog;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QWidget;

namespace ClientTests
{
class PreferenceTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_preference_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void clickButtonAndWait( QPushButton* button, int wait_ms ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setSpinBoxValue( QSpinBox* spin_box, int value ) const;
    void setGroupBoxChecked( QGroupBox* group_box, bool checked ) const;
    void selectComboBoxItem( QComboBox* combo_box, int index ) const;
    QColorDialog* waitForColorDialog( int timeout_ms ) const;
    void selectColor( QWidget* target, const QColor& color ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    void recordCheck( const QString& description, bool passed );

    struct StepEntry
    {
        QString description;
        bool completed = false;
    };

    struct CheckEntry
    {
        QString description;
        bool passed = false;
    };

    struct ScreenshotEntry
    {
        QString file_name;
        QString caption;
    };

    QString m_client_executable;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    std::vector<StepEntry> m_steps;
    std::vector<CheckEntry> m_checks;
    std::vector<QString> m_visual_checks;
    std::vector<ScreenshotEntry> m_screenshots;
    bool m_test_succeeded = false;
};
}

#endif // PREFERENCETEST_H

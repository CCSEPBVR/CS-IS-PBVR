#ifndef PREFERENCETEST_H
#define PREFERENCETEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>

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
    void startVideoRecording();
    void stopVideoRecording();
    void clickButtonAndWait( QPushButton* button, int wait_ms ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setSpinBoxValue( QSpinBox* spin_box, int value ) const;
    void setGroupBoxChecked( QGroupBox* group_box, bool checked ) const;
    void selectComboBoxItem( QComboBox* combo_box, int index ) const;
    QColorDialog* waitForColorDialog( int timeout_ms ) const;
    void selectColor( QWidget* target, const QColor& color ) const;

    QProcess m_recording_process;
    QString m_client_executable;
    QString m_output_dir_path;
    QString m_video_file_path;
};
}

#endif // PREFERENCETEST_H

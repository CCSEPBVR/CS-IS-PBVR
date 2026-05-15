#ifndef COMMUNICATIONSHAREVIEWTEST_H
#define COMMUNICATIONSHAREVIEWTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QLineEdit;
class QListView;
class QPushButton;
class QRadioButton;
class QTabWidget;
class QWidget;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;

namespace ClientTests
{
class CommunicationShareViewTest : public QObject
{
    Q_OBJECT

private:
    struct ScreenshotEntry
    {
        QString file_name;
        QString caption;
    };

    struct StepEntry
    {
        QString description;
        bool completed = false;
    };

    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        QWidget* screen_widget = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* share_view_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QTabWidget* tab_widget = nullptr;
        QWidget* share_view_tab = nullptr;
        QListView* share_list_view = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_communication_share_view_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void startVideoRecording();
    void stopVideoRecording();
    void bringWindowToFront( MainWindow* window ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    bool ensureConnected( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void selectShareViewTab( const ClientHandles& client ) const;
    void dragScreenRightButton( QWidget* widget ) const;

    QProcess m_server_process;
    QProcess m_recording_process;
    QString m_operator_client_executable;
    QString m_guest_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QString m_video_file_path;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<StepEntry> m_steps;
    bool m_test_succeeded = false;
};
}

#endif // COMMUNICATIONSHAREVIEWTEST_H

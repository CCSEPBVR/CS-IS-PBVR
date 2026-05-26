#ifndef COMMUNICATIONTEST_H
#define COMMUNICATIONTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QAction;
class QFileDialog;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListView;
class QMenu;
class QPushButton;
class QRadioButton;
class QTabWidget;
class QTextBrowser;
class QTreeView;
class QWidget;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class TotalParticlesToolBar;
class VolumeTransform;

namespace ClientTests
{
class CommunicationTest : public QObject
{
    Q_OBJECT

public:
    explicit CommunicationTest( QObject* parent = nullptr );

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
        TotalParticlesToolBar* total_particles_tool_bar = nullptr;
        VolumeTransform* volume_transform = nullptr;
        QWidget* screen_widget = nullptr;
        QMenu* tools_menu = nullptr;
        QAction* glyph_editor_action = nullptr;
        QAction* plot_over_line_editor_action = nullptr;
        QAction* plot_over_time_editor_action = nullptr;
        QAction* transfer_function_editor_action = nullptr;
        QAction* volume_transform_action = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* transfer_operator_apply_button = nullptr;
        QPushButton* volume_data_path_button = nullptr;
        QPushButton* transfer_function_path_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* share_view_button = nullptr;
        QPushButton* volume_transform_apply_button = nullptr;
        QRadioButton* local_viz_radio = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QRadioButton* uniform_radio = nullptr;
        QRadioButton* metropolis_radio = nullptr;
        QRadioButton* rejection_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* transfer_function_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* is_operator_line_edit = nullptr;
        QLineEdit* transfer_operator_id_line_edit = nullptr;
        QLineEdit* chat_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QLabel* total_particles_display_label = nullptr;
        QTextBrowser* text_browser = nullptr;
        QTreeView* object_tree_view = nullptr;
        QTabWidget* tab_widget = nullptr;
        QWidget* share_view_tab = nullptr;
        QListView* share_list_view = nullptr;
        QDoubleSpinBox* rotation_x_axis_spin_box = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_communication_scenario();

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
    void ensureConnected( const ClientHandles& client ) const;
    void ensureDisconnected( const ClientHandles& client ) const;
    void connectGuestClient( const ClientHandles& guest ) const;
    void selectRadioButton( QRadioButton* radio_button, const char* object_name ) const;
    void setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const;
    void configureVisualization(
        const ClientHandles& client,
        QRadioButton* visualization_radio,
        const QString& volume_path,
        const QString& transfer_function_path,
        QRadioButton* sampling_radio = nullptr ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void generateObject(
        const ClientHandles& client,
        QRadioButton* visualization_radio,
        const QString& volume_path,
        const QString& transfer_function_path,
        QRadioButton* sampling_radio = nullptr ) const;
    void openToolsMenuAndCapture(
        const ClientHandles& client,
        const QString& file_name,
        const QString& caption,
        bool expected_enabled );
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromBrowseDialog(
        const ClientHandles& client,
        QPushButton* browse_button,
        const QString& file_path,
        const QString& screenshot_file_name,
        const QString& caption );
    void uncheckSecondObjectDisplay( const ClientHandles& client ) const;
    void openVolumeTransformAndApplyRotationX( const ClientHandles& client, double value ) const;
    void selectShareViewTab( const ClientHandles& client ) const;
    void dragScreenLeftButton( QWidget* widget ) const;

    QProcess m_recording_process;
    QString m_operator_client_executable;
    QString m_guest_client_executable;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QString m_video_file_path;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<StepEntry> m_steps;
    bool m_test_succeeded = false;
};
}

#endif // COMMUNICATIONTEST_H

#ifndef TOTALPARTICLESTOOLBARTEST_H
#define TOTALPARTICLESTOOLBARTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QStandardItemModel;
class QTreeView;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class TotalParticlesToolBar;

namespace ClientTests
{
class TotalParticlesToolBarTest : public QObject
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
        TotalParticlesToolBar* total_particles_tool_bar = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QLabel* total_particles_display_label = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void total_particles_display_updates();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    QStandardItemModel* waitForObjectModel( const ClientHandles& client ) const;
    void applyObjectEditor( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void setDisplayItemChecked( const ClientHandles& client, bool checked ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void waitForTotalParticlesNumber( const ClientHandles& client ) const;
    void waitForNoPointObjects( const ClientHandles& client ) const;

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<StepEntry> m_steps;
    bool m_test_succeeded = false;
};
}

#endif // TOTALPARTICLESTOOLBARTEST_H

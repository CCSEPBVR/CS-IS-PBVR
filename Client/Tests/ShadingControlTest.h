#ifndef SHADINGCONTROLTEST_H
#define SHADINGCONTROLTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QRadioButton;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class ShadingControl;

namespace ClientTests
{
class ShadingControlTest : public QObject
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
        ShadingControl* shading_control = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QRadioButton* none_radio_button = nullptr;
        QRadioButton* phong_radio_button = nullptr;
        QRadioButton* blinn_phong_radio_button = nullptr;
        QRadioButton* lambert_radio_button = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QDoubleSpinBox* ambient_double_spin_box = nullptr;
        QDoubleSpinBox* diffuse_double_spin_box = nullptr;
        QDoubleSpinBox* specular_double_spin_box = nullptr;
        QDoubleSpinBox* shininess_double_spin_box = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_shading_control_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringShadingControlToFront( ShadingControl* control ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void selectShadingRadioButton( const ClientHandles& client, QRadioButton* radio_button, const QString& step_description );
    void captureShadingState( const QString& file_name, const QString& caption );

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

#endif // SHADINGCONTROLTEST_H

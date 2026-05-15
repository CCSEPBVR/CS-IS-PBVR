#ifndef VOLUMETRANSFORMTEST_H
#define VOLUMETRANSFORMTEST_H

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
class VolumeTransform;

namespace ClientTests
{
class VolumeTransformTest : public QObject
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
        VolumeTransform* volume_transform = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* volume_transform_apply_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QDoubleSpinBox* translation_x_axis_spin_box = nullptr;
        QDoubleSpinBox* translation_y_axis_spin_box = nullptr;
        QDoubleSpinBox* translation_z_axis_spin_box = nullptr;
        QDoubleSpinBox* scale_spin_box = nullptr;
        QDoubleSpinBox* rotation_x_axis_spin_box = nullptr;
        QDoubleSpinBox* rotation_y_axis_spin_box = nullptr;
        QDoubleSpinBox* rotation_z_axis_spin_box = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_volume_transform_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringVolumeTransformToFront( VolumeTransform* control ) const;
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
    void applyVolumeTransform( const ClientHandles& client, const QString& step_description );
    void captureVolumeTransformState( const QString& file_name, const QString& caption );

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

#endif // VOLUMETRANSFORMTEST_H

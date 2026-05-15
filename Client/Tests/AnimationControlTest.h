#ifndef ANIMATIONCONTROLTEST_H
#define ANIMATIONCONTROLTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>

class QFileDialog;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QWidget;

class MainWindow;
class AnimationControl;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class TotalParticlesToolBar;

namespace ClientTests
{
class AnimationControlTest : public QObject
{
    Q_OBJECT

private:
    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        TotalParticlesToolBar* total_particles_tool_bar = nullptr;
        AnimationControl* animation_control = nullptr;
        QWidget* screen_widget = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QComboBox* capture_combo_box = nullptr;
        QLineEdit* image_file_line_edit = nullptr;
        QSpinBox* interpolation_spin_box = nullptr;
        QLabel* total_key_frames_display_label = nullptr;
        QLabel* total_particles_display_label = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_animation_control_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path, bool require_existing_file ) const;
    void startVideoRecording();
    void stopVideoRecording();
    void bringWindowToFront( MainWindow* window ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureConnected( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void waitForVisibleObject( const ClientHandles& client ) const;
    void focusScreen( QWidget* widget ) const;
    void dragScreenLeftButton( QWidget* widget ) const;
    void pressScreenKey( QWidget* widget, Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier ) const;
    int screenshotCount() const;
    void removeExistingScreenshots() const;

    QProcess m_server_process;
    QProcess m_recording_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_video_file_path;
    QString m_anim_file_path;
    QString m_capture_base_name;
    QString m_capture_dir_path;
    QString m_original_current_path;
};
}

#endif // ANIMATIONCONTROLTEST_H

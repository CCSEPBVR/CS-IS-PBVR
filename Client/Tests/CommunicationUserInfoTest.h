#ifndef COMMUNICATIONUSERINFOTEST_H
#define COMMUNICATIONUSERINFOTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>

class QLineEdit;
class QPushButton;
class QRadioButton;
class QTextBrowser;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;

namespace ClientTests
{
class CommunicationUserInfoTest : public QObject
{
    Q_OBJECT

private:
    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* transfer_operator_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* transfer_operator_id_line_edit = nullptr;
        QLineEdit* chat_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QTextBrowser* text_browser = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_communication_user_info_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void startVideoRecording();
    void stopVideoRecording();
    void bringWindowToFront( MainWindow* window ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureConnected( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;

    QProcess m_server_process;
    QProcess m_recording_process;
    QString m_operator_client_executable;
    QString m_guest_client_executable;
    QString m_server_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_video_file_path;
};
}

#endif // COMMUNICATIONUSERINFOTEST_H

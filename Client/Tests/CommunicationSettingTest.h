#ifndef COMMUNICATIONSETTINGTEST_H
#define COMMUNICATIONSETTINGTEST_H

#include <QObject>
#include <QProcess>
#include <QString>

class QLineEdit;
class QPushButton;
class QRadioButton;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;

namespace ClientTests
{
class CommunicationSettingTest : public QObject
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
        QRadioButton* local_viz_radio = nullptr;
        QRadioButton* uniform_radio = nullptr;
        QRadioButton* metropolis_radio = nullptr;
        QRadioButton* rejection_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* transfer_function_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_communication_setting_scenario();

private:
    void startVideoRecording();
    void stopVideoRecording();
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureConnected( const ClientHandles& client ) const;
    void ensureDisconnected( const ClientHandles& client ) const;
    void selectRadioButton( QRadioButton* radio_button, const char* object_name ) const;
    void configureLocalSampling( const ClientHandles& client, QRadioButton* sampling_radio ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void writeSummaryReport() const;

    QProcess m_recording_process;
    QString m_client_executable;
    QString m_volume_data_path;
    QString m_transfer_function_path;
    QString m_output_dir_path;
    QString m_video_file_path;
    QString m_summary_file_path;
    bool m_test_succeeded = false;
};
}

#endif // COMMUNICATIONSETTINGTEST_H

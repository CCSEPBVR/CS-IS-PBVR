#ifndef TRANSFERFUNCTIONEDITORCOLORFUNCTIONVARIABLETEST_H
#define TRANSFERFUNCTIONEDITORCOLORFUNCTIONVARIABLETEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QComboBox;
class QDialog;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class TransferFunctionEditor;

namespace TransferFunctionEditorTest
{
class ColorFunctionVariableTest : public QObject
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
        TransferFunctionEditor* transfer_function_editor = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QSpinBox* number_of_transfer_function_spin_box = nullptr;
        QComboBox* color_function_combo_box = nullptr;
        QPushButton* color_function_variable_editor_button = nullptr;
        QLineEdit* color_function_variable_line_edit = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void color_function_variable();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void closeUnexpectedDialogs( TransferFunctionEditor* editor ) const;
    void setTransferFunctionCount( const ClientHandles& client, int value ) const;
    void setComboBoxIndex( QComboBox* combo_box, int index, const char* widget_name ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    QDialog* findVisibleVariableEditor() const;
    void setVariableEditorRows( QDialog* dialog, const QStringList& variables ) const;
    void editColorVariablesWithDialog(
        const ClientHandles& client,
        const QStringList& variables,
        const QString& file_name,
        const QString& caption,
        bool accept_dialog );
    void captureTransferFunctionEditorState(
        const ClientHandles& client,
        const QString& file_name,
        const QString& caption,
        int color_combo_index = -1 );

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

#endif // TRANSFERFUNCTIONEDITORCOLORFUNCTIONVARIABLETEST_H

#ifndef TRANSFERFUNCTIONEDITORIMPORTEXPORTTEST_H
#define TRANSFERFUNCTIONEDITORIMPORTEXPORTTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

#include "../Widgets/TransferFunctionEditor.h"

class QFileDialog;
class QComboBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QTableWidget;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;

namespace TransferFunctionEditorTest
{
class ImportExportTest : public QObject
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
        QPushButton* tf_import_button = nullptr;
        QPushButton* tf_export_button = nullptr;
        QPushButton* tf_apply_button = nullptr;
        QPushButton* tf_color_map_edit_button = nullptr;
        QComboBox* tf_color_function_combo_box = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_import_export_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const;
    void closeTransferFunctionEditor( TransferFunctionEditor* editor ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path, bool require_existing_file ) const;
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void disconnectClient( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void importTransferFunction( const ClientHandles& client, const QString& file_path );
    void exportTransferFunction( const ClientHandles& client, const QString& file_path );
    void selectThirdColorFunction( const ClientHandles& client ) const;
    void applyCoolToWarmColorMap( const ClientHandles& client );
    bool findPresetCell( QTableWidget* table, const QString& preset_name, int* row, int* column ) const;

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_import_tfe_path;
    QString m_export_tfe_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<StepEntry> m_steps;
    bool m_test_succeeded = false;
};
}

#endif // TRANSFERFUNCTIONEDITORIMPORTEXPORTTEST_H

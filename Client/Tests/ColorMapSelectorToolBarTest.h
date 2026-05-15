#ifndef COLORMAPSELECTORTOOLBARTEST_H
#define COLORMAPSELECTORTOOLBARTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QDialog;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class TransferFunctionEditor;
class ColorMapSelectorToolBar;

namespace ClientTests
{
class ColorMapSelectorToolBarTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_color_map_selector_toolbar_scenario();

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
        ColorMapSelectorToolBar* color_map_selector_tool_bar = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* tf_apply_button = nullptr;
        QPushButton* tf_color_map_edit_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QRadioButton* color_user_defined_min_max_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QDoubleSpinBox* color_user_defined_min_spin_box = nullptr;
        QDoubleSpinBox* color_user_defined_max_spin_box = nullptr;
        QSpinBox* number_of_transfer_function_spin_box = nullptr;
        QComboBox* tf_color_function_combo_box = nullptr;
        QComboBox* selector_color_function_combo_box = nullptr;
    };

    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const;
    void bringDialogToFront( QDialog* dialog ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value, const char* widget_name ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void applyTransferFunction( const ClientHandles& client ) const;
    void selectComboBoxIndex( QComboBox* combo_box, int index, const char* widget_name ) const;
    QDialog* waitForColorMapEditor() const;
    void doubleClickPresetColorMap( QDialog* dialog, const QString& preset_name ) const;
    void applyPresetColorMapFromEditor( const ClientHandles& client, const QString& preset_name ) const;

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

#endif // COLORMAPSELECTORTOOLBARTEST_H

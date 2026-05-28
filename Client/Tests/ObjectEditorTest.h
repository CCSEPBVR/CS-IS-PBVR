#ifndef OBJECTEDITORTEST_H
#define OBJECTEDITORTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>
#include <vector>

class QLineEdit;
class QLabel;
class QCheckBox;
class QColor;
class QDialog;
class QDoubleSpinBox;
class QFileDialog;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStandardItemModel;
class QTreeView;
class QWidget;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class TimeStepControlToolBar;

namespace ClientTests
{
class ObjectEditorTest : public QObject
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
        TimeStepControlToolBar* time_step_tool_bar = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* first_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* next_button = nullptr;
        QPushButton* play_button = nullptr;
        QPushButton* loop_button = nullptr;
        QPushButton* browse_button = nullptr;
        QPushButton* delete_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QCheckBox* focus_check_box = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QLineEdit* coordinate_x_line_edit = nullptr;
        QLineEdit* coordinate_y_line_edit = nullptr;
        QLineEdit* coordinate_z_line_edit = nullptr;
        QSpinBox* min_limit_time_step_spin_box = nullptr;
        QSpinBox* max_limit_time_step_spin_box = nullptr;
        QSpinBox* particle_limit_spin_box = nullptr;
        QDoubleSpinBox* opacity_double_spin_box = nullptr;
        QWidget* color_clickable_label = nullptr;
        QTreeView* object_tree_view = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_object_editor_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void addStep( const QString& description );
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureConnected( const ClientHandles& client );
    void ensureDisconnected( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    void configureRemoteVisualizationWithoutVolume( const ClientHandles& client ) const;
    void applyObjectEditor( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void clickNextAndWaitForCompletion( const ClientHandles& client ) const;
    QStandardItemModel* waitForObjectModel( const ClientHandles& client ) const;
    void selectObjectRow( const ClientHandles& client, int row ) const;
    void setModelCheckState( const ClientHandles& client, int row, int column, bool checked, const char* item_name ) const;
    void setDisplayItemChecked( const ClientHandles& client, int row, bool checked ) const;
    void setKeepInitialChecked( const ClientHandles& client, int row, bool checked ) const;
    void setKeepFinalChecked( const ClientHandles& client, int row, bool checked ) const;
    void setSpinBoxValue( QSpinBox* spin_box, int value, const char* object_name ) const;
    void setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value, const char* object_name ) const;
    void setCheckBoxState( QCheckBox* check_box, bool checked, const char* object_name ) const;
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    QDialog* waitFor3dDataDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path ) const;
    void selectFileFromRemoteDialog( QDialog* dialog, const QString& file_path ) const;
    void chooseColorFromDialog( const QColor& color ) const;
    void browseAndLoadObject( const ClientHandles& client, const QString& file_path, int expected_row_count ) const;
    void clickFirstAndWait( const ClientHandles& client ) const;
    void advanceTimeSteps( const ClientHandles& client, int repeat_count ) const;
    QString serverProcessSummary();

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_point_data_path;
    QString m_object_data_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<StepEntry> m_steps;
    bool m_test_succeeded = false;
};
}

#endif // OBJECTEDITORTEST_H

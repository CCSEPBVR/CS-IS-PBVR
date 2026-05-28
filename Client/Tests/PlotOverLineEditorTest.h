#ifndef PLOTOVERLINEEDITORTEST_H
#define PLOTOVERLINEEDITORTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QVector>
#include <functional>

class QAction;
class QComboBox;
class QDoubleSpinBox;
class QGridLayout;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStandardItemModel;
class QTreeView;

class MainWindow;
class Communication;
class GlyphEditor;
class ObjectEditor;
class PlayBackControlToolBar;
class PlotOverLineEditor;
class VolumeTransform;

namespace ClientTests
{
class PlotOverLineEditorTest : public QObject
{
    Q_OBJECT

private:
    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        GlyphEditor* glyph_editor = nullptr;
        PlotOverLineEditor* plot_over_line_editor = nullptr;
        VolumeTransform* volume_transform = nullptr;
        QAction* glyph_editor_action = nullptr;
        QAction* plot_over_line_editor_action = nullptr;
        QAction* volume_transform_action = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* glyph_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* plot_over_line_apply_button = nullptr;
        QPushButton* volume_transform_apply_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* transfer_function_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* is_operator_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QRadioButton* size_variable_array_radio = nullptr;
        QSpinBox* size_number_of_variables_spin_box = nullptr;
        QGridLayout* size_variable_grid_layout = nullptr;
        QRadioButton* color_data_variable_array_radio = nullptr;
        QSpinBox* color_data_number_of_variables_spin_box = nullptr;
        QGridLayout* color_data_variable_grid_layout = nullptr;
        QDoubleSpinBox* scale_factor_spin_box = nullptr;
        QGroupBox* plot_over_line_group_box = nullptr;
        QComboBox* target_combo_box = nullptr;
        QSpinBox* resolution_spin_box = nullptr;
        QDoubleSpinBox* start_coords_x_spin_box = nullptr;
        QDoubleSpinBox* start_coords_y_spin_box = nullptr;
        QDoubleSpinBox* start_coords_z_spin_box = nullptr;
        QDoubleSpinBox* end_coords_x_spin_box = nullptr;
        QDoubleSpinBox* end_coords_y_spin_box = nullptr;
        QDoubleSpinBox* end_coords_z_spin_box = nullptr;
        QDoubleSpinBox* rotation_x_axis_spin_box = nullptr;
    };

    struct StepEntry
    {
        QString description;
        bool completed = false;
    };

    struct ScreenshotEntry
    {
        QString file_name;
        QString caption;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_plot_over_line_editor_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    QString serverProcessSummary() const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringGlyphEditorToFront( GlyphEditor* editor ) const;
    void bringPlotOverLineEditorToFront( PlotOverLineEditor* editor ) const;
    void bringVolumeTransformToFront( VolumeTransform* editor ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const;
    void setSpinBoxValue( QSpinBox* spin_box, int value ) const;
    void setGroupBoxChecked( QGroupBox* group_box, bool checked, const char* object_name ) const;
    void selectRadioButton( QRadioButton* radio_button, const char* object_name ) const;
    void selectComboBoxItem( QComboBox* combo_box, int index ) const;
    void saveScreenshot( const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    void addStep( const QString& description );
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureConnected( const ClientHandles& client ) const;
    void ensureDisconnected( const ClientHandles& client ) const;
    void waitForOperatorPrivileges( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client, const QString& volume_path, const QString& transfer_function_path ) const;
    QStandardItemModel* waitForObjectModel( const ClientHandles& client ) const;
    void setObjectDisplayItemChecked( const ClientHandles& client, int row, bool checked ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    QComboBox* comboBoxAtGridRow( QGridLayout* grid_layout, int row ) const;
    void waitForGlyphEditorReady( const ClientHandles& client ) const;
    void openGlyphEditor( const ClientHandles& client ) const;
    void configureGlyphEditor( const ClientHandles& client ) const;
    void openVolumeTransform( const ClientHandles& client ) const;
    void configureVolumeTransform( const ClientHandles& client ) const;
    void waitForPlotOverLineEditorReady( const ClientHandles& client, int minimum_target_count ) const;
    void openPlotOverLineEditor( const ClientHandles& client, int minimum_target_count ) const;
    void configurePlotOverLine(
        const ClientHandles& client,
        double start_x,
        double start_y,
        double start_z,
        double end_x,
        double end_y,
        double end_z,
        int target_index = -1 ) const;
    void applyPlotOverLineResolution( const ClientHandles& client, int resolution ) const;
    void prepareSecondDatasetConnection( const ClientHandles& client ) const;

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_structured_volume_data_path;
    QString m_unstructured_volume_data_path;
    QString m_transfer_function_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QVector<StepEntry> m_steps;
    QVector<ScreenshotEntry> m_screenshots;
    bool m_test_succeeded = false;
};
}

#endif // PLOTOVERLINEEDITORTEST_H

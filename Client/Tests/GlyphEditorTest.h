#ifndef GLYPHEDITORTEST_H
#define GLYPHEDITORTEST_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <functional>

class QAction;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QDialog;
class QGridLayout;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTableWidget;
class QTreeView;
class QLabel;

class MainWindow;
class Communication;
class GlyphEditor;
class ObjectEditor;
class PlayBackControlToolBar;
class TotalParticlesToolBar;

namespace ClientTests
{
class GlyphEditorTest : public QObject
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
        GlyphEditor* glyph_editor = nullptr;
        QAction* glyph_editor_action = nullptr;
        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* transfer_function_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* is_operator_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QCheckBox* focus_check_box = nullptr;
        QLabel* total_particles_display_label = nullptr;
        QDoubleSpinBox* scale_factor_spin_box = nullptr;
        QComboBox* direction3_combo_box = nullptr;
        QRadioButton* size_variable_array_radio = nullptr;
        QSpinBox* size_number_of_variables_spin_box = nullptr;
        QGridLayout* size_variable_grid_layout = nullptr;
        QRadioButton* color_data_constant_radio = nullptr;
        QRadioButton* color_data_variable_array_radio = nullptr;
        QSpinBox* color_data_number_of_variables_spin_box = nullptr;
        QGridLayout* color_data_variable_grid_layout = nullptr;
        QSpinBox* number_of_sample_points_spin_box = nullptr;
        QSpinBox* seed_spin_box = nullptr;
        QRadioButton* all_points_radio = nullptr;
        QRadioButton* every_nth_point_radio = nullptr;
        QSpinBox* stride_spin_box = nullptr;
        QPushButton* edit_color_map_button = nullptr;
        QPushButton* glyph_apply_button = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_glyph_editor_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    QString sourceTreePath( const QString& relative_path_from_repo_root ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void startVideoRecording();
    void stopVideoRecording();
    void bringWindowToFront( MainWindow* window ) const;
    void bringGlyphEditorToFront( GlyphEditor* glyph_editor ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setSpinBoxValue( QSpinBox* spin_box, int value ) const;
    void setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value ) const;
    void selectRadioButton( QRadioButton* radio_button, const char* object_name ) const;
    void selectComboBoxItem( QComboBox* combo_box, int index ) const;
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureConnected( const ClientHandles& client ) const;
    void ensureDisconnected( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client, const QString& volume_path, const QString& transfer_function_path ) const;
    void waitForOperatorPrivileges( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void waitForVisiblePointObject( const ClientHandles& client ) const;
    void waitForGlyphEditorReady( const ClientHandles& client ) const;
    void openGlyphEditor( const ClientHandles& client ) const;
    void moveGlyphEditorRight( const ClientHandles& client ) const;
    QComboBox* comboBoxAtGridRow( QGridLayout* grid_layout, int row ) const;
    void configureVariableArraySections( const ClientHandles& client ) const;
    void applyXRayColorMap( const ClientHandles& client ) const;
    QString comboBoxItemsText( const QComboBox* combo_box ) const;

    QProcess m_server_process;
    QProcess m_recording_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_structured_volume_data_path;
    QString m_unstructured_volume_data_path;
    QString m_transfer_function_path;
    QString m_output_dir_path;
    QString m_video_file_path;
};
}

#endif // GLYPHEDITORTEST_H

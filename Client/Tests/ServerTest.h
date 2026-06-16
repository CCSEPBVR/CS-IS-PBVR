#ifndef SERVERTEST_H
#define SERVERTEST_H

#include <QObject>
#include <QString>
#include <functional>
#include <vector>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTreeView;

class ColorMapSelectorToolBar;
class Communication;
class MainWindow;
class ObjectEditor;
class PlayBackControlToolBar;
class RepetitionLevelControl;
class TimeStepControlToolBar;
class TransferFunctionEditor;

namespace ClientTests
{
class ServerTest : public QObject
{
    Q_OBJECT

public:
    explicit ServerTest( QObject* parent = nullptr );

    enum class SamplingMode
    {
        Uniform,
        Metropolis,
        Rejection,
    };

private:
    struct Dataset
    {
        QString key;
        QString path;
    };

    struct ScreenshotEntry
    {
        QString case_id;
        QString file_name;
        QString caption;
    };

    struct CaseEntry
    {
        QString case_id;
        QString description;
        QString data_key;
        QString data_path;
        QString status = QStringLiteral( "未実行" );
    };

    struct CaseOptions
    {
        CaseOptions() = default;
        explicit CaseOptions( int repetition_level_ )
            : repetition_level( repetition_level_ )
        {
        }
        CaseOptions( int repetition_level_, bool hide_glyph_, const QString& transfer_function_path_ )
            : repetition_level( repetition_level_ )
            , hide_glyph( hide_glyph_ )
            , transfer_function_path( transfer_function_path_ )
        {
        }

        int repetition_level = -1;
        bool hide_glyph = false;
        QString transfer_function_path;
    };

    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        TimeStepControlToolBar* time_step_tool_bar = nullptr;
        ColorMapSelectorToolBar* color_map_selector_tool_bar = nullptr;
        TransferFunctionEditor* transfer_function_editor = nullptr;
        RepetitionLevelControl* repetition_level_control = nullptr;

        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QRadioButton* uniform_radio = nullptr;
        QRadioButton* metropolis_radio = nullptr;
        QRadioButton* rejection_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* transfer_function_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;

        QLineEdit* object_name_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QCheckBox* focus_check_box = nullptr;
        QSpinBox* particle_limit_spin_box = nullptr;
        QPushButton* object_apply_button = nullptr;

        QPushButton* jump_button = nullptr;
        QSpinBox* next_time_step_spin_box = nullptr;
        QComboBox* selector_color_function_combo_box = nullptr;

        QSpinBox* number_of_transfer_function_spin_box = nullptr;
        QLineEdit* color_synthesizer_line_edit = nullptr;
        QLineEdit* opacity_synthesizer_line_edit = nullptr;
        QPushButton* transfer_function_apply_button = nullptr;

        QSpinBox* new_repetition_level_spin_box = nullptr;
        QPushButton* repetition_apply_button = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_server_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    bool configuredPathExists( const QString& path ) const;
    Dataset dataset( const QString& key ) const;
    void verifyDatasets() const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const;
    void bringRepetitionLevelControlToFront( RepetitionLevelControl* control ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void setSpinBoxValue( QSpinBox* spin_box, int value, const char* widget_name ) const;
    void selectRadioButton( QRadioButton* radio_button, const char* object_name ) const;
    void selectComboBoxIndex( QComboBox* combo_box, int index, const char* widget_name ) const;
    void saveScreenshot( const QString& case_id, const QString& file_name, const QString& caption );
    void writeMarkdownReport() const;
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void ensureDisconnected( const ClientHandles& client ) const;
    void ensureConnected( const ClientHandles& client ) const;
    void selectSamplingMode( const ClientHandles& client, SamplingMode sampling_mode ) const;
    void loadDataset(
        const ClientHandles& client,
        const Dataset& data,
        SamplingMode sampling_mode,
        const QString& transfer_function_path = QString() );
    void waitForObjectAndApply( const ClientHandles& client, bool hide_glyph = false );
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    void applyRepetitionLevel( const ClientHandles& client, int repetition_level ) const;
    void setTimeStepAndJump( const ClientHandles& client, int time_step ) const;
    void applyTransferFunctionSynthesizer(
        const ClientHandles& client,
        int required_transfer_function_count,
        const QString& color_synthesizer,
        const QString& opacity_synthesizer ) const;
    void selectColorFunction( const ClientHandles& client, int one_based_index ) const;
    void setParticleLimitAndApply( const ClientHandles& client, int particle_limit ) const;
    void runCase(
        const QString& case_id,
        const QString& description,
        const QString& data_key,
        const QString& data_path,
        const std::function<void()>& body );
    void captureCase(
        const QString& case_id,
        const QString& caption,
        int repetition_level = -1,
        const ClientHandles* client = nullptr );

    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    std::vector<Dataset> m_datasets;
    QString m_mej_transfer_function_path;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<CaseEntry> m_cases;
    bool m_test_succeeded = false;
    bool m_has_connected_once = false;
    bool m_uses_remote_data_paths = false;
};
}

#endif // SERVERTEST_H

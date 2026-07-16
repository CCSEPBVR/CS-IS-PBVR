#ifndef ENSEMBLETRANSFERFUNCTIONTEST_H
#define ENSEMBLETRANSFERFUNCTIONTEST_H

#include <QColor>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVector>

#include <array>
#include <functional>
#include <vector>

class ColorMap;
class Communication;
class EnsembleTransferFunctionEditor;
class MainWindow;
class ObjectEditor;
class OpacityMap;
class PlayBackControlToolBar;
class QComboBox;
class QDoubleSpinBox;
class QEvent;
class QFileDialog;
class QJsonObject;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSignalSpy;
class QTextStream;
class QTreeView;
class QWidget;
class TimeStepControlToolBar;
class TotalParticlesToolBar;
class VolumeTransform;

namespace ClientTests
{
class EnsembleTransferFunctionTest : public QObject
{
    Q_OBJECT

public:
    explicit EnsembleTransferFunctionTest( QObject* parent = nullptr );

private:
    struct StatisticState
    {
        QString statistic;
        bool use_user_range = true;
        double user_min = 0.0;
        double user_max = 1.0;
        double server_min = 0.0;
        double server_max = 1.0;
        QVector<QColor> colors;
        QVector<float> opacities;
    };

    struct ScreenshotPair
    {
        QString title;
        QString change_description;
        QString particle_file_name;
        QString editor_file_name;
        int request_time_step = -1;
        int request_count = 0;
        QByteArray before_fingerprint;
        QByteArray after_fingerprint;
        bool paint_completed = false;
    };

    struct ParticleUpdateResult
    {
        int request_time_step = -1;
        int request_count = 0;
        QByteArray before_fingerprint;
        QByteArray after_fingerprint;
        bool paint_completed = false;
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
        TotalParticlesToolBar* total_particles_tool_bar = nullptr;
        EnsembleTransferFunctionEditor* ensemble_editor = nullptr;
        VolumeTransform* volume_transform = nullptr;
        QWidget* render_screen = nullptr;

        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* keep_last_button = nullptr;
        QPushButton* ensemble_apply_button = nullptr;
        QPushButton* ensemble_import_button = nullptr;
        QPushButton* ensemble_export_button = nullptr;
        QRadioButton* remote_viz_insitu_radio = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QLabel* total_particles_display = nullptr;
        QLineEdit* variable_line_edit = nullptr;
        QComboBox* statistic_combo_box = nullptr;
        QRadioButton* user_min_max_radio = nullptr;
        QRadioButton* server_min_max_radio = nullptr;
        QDoubleSpinBox* user_min_spin_box = nullptr;
        QDoubleSpinBox* user_max_spin_box = nullptr;
        QDoubleSpinBox* server_min_spin_box = nullptr;
        QDoubleSpinBox* server_max_spin_box = nullptr;
        QDoubleSpinBox* rotation_x_axis_spin_box = nullptr;
        QPushButton* volume_transform_apply_button = nullptr;
        ColorMap* color_map = nullptr;
        OpacityMap* opacity_map = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_ensemble_transfer_function_scenario();

private:
    bool eventFilter( QObject* watched, QEvent* event ) override;
    QString repoRootPath() const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringEditorToFront( EnsembleTransferFunctionEditor* editor ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void saveWindowScreenshot( QWidget* window, const QString& file_name ) const;
    void saveScreenshotPair(
        const ClientHandles& client,
        const QString& file_name_prefix,
        const QString& title,
        const QString& change_description,
        const ParticleUpdateResult& update_result );
    void writeMarkdownReport() const;
    void writeImportedSettings( QTextStream& stream ) const;
    void markStepCompleted( const QString& description );
    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureInSituVisualization( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void generateInitialParticles( const ClientHandles& client, QSignalSpy& completion_spy ) const;
    void enableKeepLast( const ClientHandles& client ) const;
    void applyVolumeRotationX( const ClientHandles& client, double value ) const;
    void waitForPositiveParticleCount( const ClientHandles& client ) const;
    void selectStatistic( const ClientHandles& client, int index ) const;
    void setUserRange( const ClientHandles& client, double min_value, double max_value ) const;
    void setServerRange( const ClientHandles& client ) const;
    void applyColorPreset( const ClientHandles& client, const QString& preset_name ) const;
    void applyOpacityExpression( const ClientHandles& client, const QString& expression ) const;
    StatisticState captureStatisticState( const ClientHandles& client, int index ) const;
    std::array<StatisticState, 3> captureAllStatisticStates( const ClientHandles& client ) const;
    bool extractSettingsFromPayload(
        const QJsonObject& payload,
        QString* variable,
        std::array<StatisticState, 3>* states ) const;
    bool waitForInitialSettings(
        QSignalSpy& parameter_spy,
        QString* variable,
        std::array<StatisticState, 3>* states ) const;
    void compareStatisticState( const ClientHandles& client, int index, const StatisticState& expected ) const;
    void restoreSettings(
        const ClientHandles& client,
        const QString& variable,
        const std::array<StatisticState, 3>& states ) const;
    QByteArray particleFingerprint( const ClientHandles& client ) const;
    bool responseMatchesExpectedSettings(
        const QJsonObject& payload,
        const std::array<StatisticState, 3>& expected,
        const QString& expected_variable ) const;
    bool waitForAppliedParticles(
        const ClientHandles& client,
        QSignalSpy& request_spy,
        QSignalSpy& completion_spy,
        QSignalSpy& request_parameter_spy,
        const std::array<StatisticState, 3>& expected,
        const QString& expected_variable,
        bool require_particle_change,
        ParticleUpdateResult* result );
    bool applyAndCaptureCase(
        const ClientHandles& client,
        QSignalSpy& request_spy,
        QSignalSpy& completion_spy,
        QSignalSpy& request_parameter_spy,
        int statistic_index,
        const QString& variable,
        const QString& file_name_prefix,
        const QString& title,
        const QString& change_description,
        bool require_particle_change = true );
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path, bool require_existing_file ) const;
    void exportSettings( const ClientHandles& client, const QString& file_path ) const;
    void importSettings( const ClientHandles& client, const QString& file_path ) const;
    void validateExportedJson(
        const QString& file_path,
        const QString& expected_variable,
        const std::array<StatisticState, 3>& expected ) const;

    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_report_path;
    QString m_export_path;
    std::vector<ScreenshotPair> m_screenshot_pairs;
    std::vector<StepEntry> m_steps;
    QString m_import_variable;
    std::array<StatisticState, 3> m_import_states;
    bool m_has_import_settings = false;
    QString m_baseline_variable;
    std::array<StatisticState, 3> m_baseline_states;
    bool m_has_baseline_settings = false;
    bool m_test_succeeded = false;
    QObject* m_render_screen_object = nullptr;
    int m_render_paint_count = 0;
};
}

#endif // ENSEMBLETRANSFERFUNCTIONTEST_H

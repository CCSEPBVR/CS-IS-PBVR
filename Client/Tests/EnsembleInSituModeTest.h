#ifndef ENSEMBLEINSITUMODETEST_H
#define ENSEMBLEINSITUMODETEST_H

#include <QByteArray>
#include <QColor>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include <array>
#include <functional>
#include <vector>

class QAction;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSignalSpy;
class QSpinBox;
class QTextStream;
class QTreeView;
class QWidget;

class ColorMap;
class Communication;
class EnsembleTransferFunctionEditor;
class MainWindow;
class ObjectEditor;
class OpacityMap;
class PlayBackControlToolBar;
class TimeStepControlToolBar;
class TotalParticlesToolBar;

namespace ClientTests
{
// Server シート ISPBVR 33 / 35(S-IS-33 / S-IS-35)。
// Part A(S-IS-33): アンサンブルシミュレーションの動作確認
// Part C(S-IS-35): default_XXXXX.json にアンサンブルセクションが無いステップの動作確認
// 既存の InSituModeTest は変更せず、本クラスを追加する。
class EnsembleInSituModeTest : public QObject
{
    Q_OBJECT

public:
    explicit EnsembleInSituModeTest( QObject* parent = nullptr );

private:
    struct StepEntry
    {
        QString description;
        QString status; // PASS / FAIL / SKIP
    };

    struct ItemResult
    {
        QString id;
        QString title;
        QString status = QStringLiteral( "NOT RUN" ); // PASS / FAIL / SKIP
        QStringList messages;
    };

    struct ScreenshotEntry
    {
        QString item_id;
        int seq = 0;
        QString file_name;        // img/ 直下のファイル名
        QString caption;
        QString manual_rel_path;  // img/manual/ からの相対パス
        bool captured = false;
        bool copied = false;
        int time_step = -1;
    };

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

    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        TimeStepControlToolBar* time_step_tool_bar = nullptr;
        TotalParticlesToolBar* total_particles_tool_bar = nullptr;
        EnsembleTransferFunctionEditor* ensemble_editor = nullptr;

        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QRadioButton* remote_viz_insitu_radio = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* keep_last_button = nullptr;
        QSpinBox* next_time_step_spin_box = nullptr;
        QLabel* total_particles_display = nullptr;

        QPushButton* ensemble_apply_button = nullptr;
        QLineEdit* variable_line_edit = nullptr;
        QComboBox* statistic_combo_box = nullptr;
        QRadioButton* user_min_max_radio = nullptr;
        QRadioButton* server_min_max_radio = nullptr;
        QDoubleSpinBox* user_min_spin_box = nullptr;
        QDoubleSpinBox* user_max_spin_box = nullptr;
        QDoubleSpinBox* server_min_spin_box = nullptr;
        QDoubleSpinBox* server_max_spin_box = nullptr;
        ColorMap* color_map = nullptr;
        OpacityMap* opacity_map = nullptr;
    };

    struct Spies
    {
        QSignalSpy* request_spy = nullptr;           // TimeStepControlToolBar::requestDataAt
        QSignalSpy* completion_spy = nullptr;        // TimeStepControlToolBar::dataRequestCompleted
        QSignalSpy* parameter_spy = nullptr;         // Communication::receiveEnsembleStatisticsParameter
        QSignalSpy* request_parameter_spy = nullptr; // Communication::receiveRequestDataAtEnsembleStatisticsParameter
        QSignalSpy* request_tf_spy = nullptr;        // Communication::receiveRequestDataAtTransferFunctionParameter
        QSignalSpy* selected_file_spy = nullptr;     // Communication::receiveSelectedFile(Initialize の ObjectInfo、IsEnsemble を含む)
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_ensemble_in_situ_mode_scenario();

private:
    QString repoRootPath() const;
    QString configuredValue( const char* name ) const;
    QString configuredPathOrEmpty( const char* name ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringEditorToFront( EnsembleTransferFunctionEditor* editor ) const;
    void markStep( const QString& item_id, const QString& description, const QString& status = QStringLiteral( "PASS" ) );
    bool check( ItemResult& result, bool condition, const QString& message );
    void skipItem( ItemResult& result, const QString& reason );
    void finishItem( ItemResult& result );
    void saveScreenshot(
        ItemResult& result,
        int seq,
        const QString& content,
        const QString& caption,
        const QString& manual_rel_path,
        int time_step );
    void writeMarkdownReport() const;

    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureInSituVisualization( const ClientHandles& client ) const;
    bool waitForInitialStatistics( QSignalSpy& parameter_spy, QStringList* statistics ) const;
    bool waitForIsEnsemble( QSignalSpy& selected_file_spy ) const;
    void waitForObjectRows( const ClientHandles& client ) const;
    void selectFirstObjectAndApply( const ClientHandles& client ) const;
    QAction* findEnsembleEditorAction( MainWindow* window ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client, QSignalSpy& completion_spy ) const;
    void setKeepLast( const ClientHandles& client, bool enabled ) const;
    void waitForPositiveParticleCount( const ClientHandles& client ) const;
    int totalParticles( const ClientHandles& client ) const;
    int latestCompletedTimeStep( const Spies& spies ) const;
    bool waitForNextTimeStep( const Spies& spies, int* time_step ) const;
    bool waitForQuietRequests( const Spies& spies, int quiet_ms ) const;
    QByteArray particleFingerprint( const ClientHandles& client ) const;
    void selectStatistic( const ClientHandles& client, int index ) const;
    StatisticState captureStatisticState( const ClientHandles& client, int index ) const;
    std::array<StatisticState, 3> captureAllStatisticStates( const ClientHandles& client ) const;
    bool statesEqual( const StatisticState& a, const StatisticState& b, QString* why ) const;
    bool applyStatisticAndWaitForChange(
        ItemResult& result,
        const ClientHandles& client,
        const Spies& spies,
        int statistic_index,
        const QString& statistic_name,
        const QByteArray& previous_fingerprint,
        int* time_step,
        QByteArray* new_fingerprint );
    bool waitForStatisticEcho( QSignalSpy& parameter_spy, int start_index, const QString& statistic_name ) const;
    QJsonObject readJsonObject( const QString& path, bool* ok ) const;
    bool writeJsonObject( const QString& path, const QJsonObject& root ) const;
    QString stepJsonPath( int time_step ) const;
    void checkServerSideFiles( ItemResult& result );

    void runPartA( ItemResult& result, const ClientHandles& client, const Spies& spies, MainWindow& window );
    void runPartC( ItemResult& result, const ClientHandles& client, const Spies& spies );

    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_manual_dir_path;
    QString m_report_path;
    QString m_vis_param_dir;
    QString m_particle_dir;
    QString m_tf_name;
    QString m_server_stderr_log;
    std::vector<StepEntry> m_steps;
    std::vector<ItemResult> m_item_results;
    std::vector<ScreenshotEntry> m_screenshots;
    QStringList m_checked_files;
    QStringList m_initial_statistics;
    int m_part_c_time_step = -1;
    QString m_part_c_backup_path;
    QString m_part_c_target_path;
    bool m_test_succeeded = false;
};
}

#endif // ENSEMBLEINSITUMODETEST_H

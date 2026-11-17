#ifndef REPETITIONLEVELCONTROLINSITUTEST_H
#define REPETITIONLEVELCONTROLINSITUTEST_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>
#include <vector>

class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSignalSpy;
class QSpinBox;
class QTreeView;
class QWidget;

class MainWindow;
class Communication;
class ObjectEditor;
class PlayBackControlToolBar;
class RepetitionLevelControl;
class TimeStepControlToolBar;
class TotalParticlesToolBar;

namespace ClientTests
{
// C-RLC-03: IS モードで Next Repetition Level を操作し、サーバ側の粒子生成数が変わることを確認する。
// 既存の RepetitionLevelControlTest(CS モード)は変更せず、本クラスを追加する。
class RepetitionLevelControlInSituTest : public QObject
{
    Q_OBJECT

public:
    explicit RepetitionLevelControlInSituTest( QObject* parent = nullptr );

private:
    struct StepEntry
    {
        QString description;
        QString status; // PASS / FAIL / SKIP
    };

    struct ScreenshotEntry
    {
        int seq = 0;
        QString file_name;        // img/ 直下のファイル名
        QString caption;
        QString manual_rel_path;  // img/manual/ からの相対パス
        bool captured = false;
        bool copied = false;
        int time_step = -1;
    };

    struct LevelRecord
    {
        QString label;            // 初期値 / 1 / 8 / 16
        int level = -1;           // 設定した Repetition Level
        int echoed_level = -1;    // サーバから再配信された RepeatLevel
        int total_particles = -1; // Total Particles
        int time_step = -1;       // 粒子を受信(記録)したタイムステップ
        int max_step_before_apply = -1; // Apply 前にサーバから通知されていた最新ステップ(LatestTimeStep、参考)
        int max_step_after_apply = -1;  // Apply 後に最初に通知された最新ステップ M'
        int threshold_step = -1;        // 記録対象ステップの下限(M' + 2)
        int updates_waited = 0;         // 記録確定までに受信した RequestDataAt 完了の回数
        int prev_count = -1;            // 変化判定の基準にした前ケースの粒子数(C_prev)
        QString change_note;            // 変化・安定判定の経緯
        QString default_json_check; // 記録ステップの default_XXXXX.json の repeat_level 突合結果
    };

    struct ClientHandles
    {
        MainWindow* main_window = nullptr;
        Communication* communication = nullptr;
        ObjectEditor* object_editor = nullptr;
        PlayBackControlToolBar* playback_tool_bar = nullptr;
        TimeStepControlToolBar* time_step_tool_bar = nullptr;
        TotalParticlesToolBar* total_particles_tool_bar = nullptr;
        RepetitionLevelControl* repetition_level_control = nullptr;

        QPushButton* connect_button = nullptr;
        QPushButton* disconnect_button = nullptr;
        QPushButton* setting_apply_button = nullptr;
        QRadioButton* remote_viz_insitu_radio = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QTreeView* object_tree_view = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;
        QPushButton* keep_last_button = nullptr;
        QLabel* total_particles_display = nullptr;
        QLabel* current_repetition_level_label = nullptr;
        QSpinBox* new_repetition_level_spin_box = nullptr;
        QPushButton* repetition_apply_button = nullptr;
    };

    struct Spies
    {
        QSignalSpy* repetition_spy = nullptr;  // Communication::receiveRepetitionLevelParameter
        QSignalSpy* request_spy = nullptr;     // TimeStepControlToolBar::requestDataAt
        QSignalSpy* completion_spy = nullptr;  // TimeStepControlToolBar::dataRequestCompleted
        QSignalSpy* latest_step_spy = nullptr; // Communication::updateObjectLatestTimeStep(LatestTimeStep イベント: UpdateMaxTimeStep)
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_repetition_level_control_in_situ_scenario();

private:
    QString repoRootPath() const;
    QString configuredValue( const char* name ) const;
    QString configuredPathOrEmpty( const char* name ) const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringRepetitionLevelControlToFront( RepetitionLevelControl* control ) const;
    void markStep( const QString& description, const QString& status = QStringLiteral( "PASS" ) );
    bool check( bool condition, const QString& message );
    void saveScreenshot( int seq, const QString& content, const QString& caption, const QString& manual_rel_path, int time_step );
    void writeMarkdownReport() const;

    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void configureInSituVisualization( const ClientHandles& client ) const;
    bool waitForRepeatLevel( QSignalSpy& spy, int start_index, int expected_level, int timeout_ms, int* received_level ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client, QSignalSpy& completion_spy ) const;
    void enableKeepLast( const ClientHandles& client ) const;
    void waitForPositiveParticleCount( const ClientHandles& client ) const;
    int totalParticles( const ClientHandles& client ) const;
    bool waitForNextTimeStep( const Spies& spies, int* time_step ) const;
    int latestCompletedTimeStep( const Spies& spies ) const;
    int notifiedMaxTimeStepAt( const Spies& spies, int index ) const;
    int latestNotifiedMaxTimeStep( const Spies& spies ) const;
    bool waitForNotifiedMaxTimeStep( const Spies& spies, int start_index, int timeout_ms, int* value ) const;
    bool waitForTimeStepAtOrAfter( const Spies& spies, int completion_start, int threshold, int timeout_ms, int* time_step, int* updates );
    bool waitForStableChangedParticleCount(
        const ClientHandles& client,
        const Spies& spies,
        int threshold_floor,
        int prev_count,
        int timeout_ms,
        int* time_step,
        int* total_particles,
        int* updates,
        QString* note );
    bool applyRepetitionLevel(
        const ClientHandles& client,
        const Spies& spies,
        int level,
        int* echoed_level,
        int* latest_step_start_index = nullptr,
        int* completion_start_index = nullptr );
    QString checkDefaultJsonRepeatLevel( int expected_level, int* particle_limit, bool strict = true );
    QString checkStepJsonRepeatLevel( int time_step, int expected_level );
    QString defaultJsonPath() const;
    QString stepJsonPath( int time_step ) const;
    void runLevelCase( const ClientHandles& client, const Spies& spies, int level, int seq, const QString& manual_rel_path );
    void judgeParticleCounts();

    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_manual_dir_path;
    QString m_report_path;
    QString m_vis_param_dir;
    QString m_tf_name;
    std::vector<StepEntry> m_steps;
    std::vector<ScreenshotEntry> m_screenshots;
    std::vector<LevelRecord> m_records;
    QStringList m_judgements;
    QStringList m_failures;
    int m_initial_level = -1;
    int m_particle_limit = -1;
    bool m_scenario_completed = false;
    bool m_test_succeeded = false;
};
}

#endif // REPETITIONLEVELCONTROLINSITUTEST_H

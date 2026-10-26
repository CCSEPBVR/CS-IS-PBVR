#ifndef TRANSFERFUNCTIONEDITORIMPORTINVALIDFILETEST_H
#define TRANSFERFUNCTIONEDITORIMPORTINVALIDFILETEST_H

#include <QColor>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
#include <vector>

class QComboBox;
class QDoubleSpinBox;
class QFileDialog;
class QLineEdit;
class QMessageBox;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTextStream;
class QWidget;

class ColorMap;
class Communication;
class MainWindow;
class ObjectEditor;
class OpacityMap;
class PlayBackControlToolBar;
class TransferFunctionEditor;

namespace TransferFunctionEditorTest
{
// Client シート TransferFunctionEditor 57 / 58(C-TFE-57 / C-TFE-58)
// 通常モード(CS)の Transfer Function Editor の Import 異常系。
// 既存の ImportExportTest(接続、ファイルダイアログ操作、レポート出力)を流用する。
class ImportInvalidFileTest : public QObject
{
    Q_OBJECT

public:
    explicit ImportInvalidFileTest( QObject* parent = nullptr );

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
        QString manual_rel_path;  // img/manual/ からの相対パス(空ならコピーしない)
        bool captured = false;
        bool copied = false;
    };

    struct RangeEntry
    {
        bool user_range = true;
        double min = 0.0;
        double max = 1.0;
        QVector<QColor> colors;      // Color 側のみ
        QVector<float> opacities;    // Opacity 側のみ
    };

    struct EditorState
    {
        int count = 0;
        QString color_synthesizer;
        QString opacity_synthesizer;
        QVector<RangeEntry> colors;    // C1, C2, ...
        QVector<RangeEntry> opacities; // O1, O2, ...
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
        QRadioButton* remote_viz_client_server_radio = nullptr;
        QLineEdit* volume_data_path_line_edit = nullptr;
        QLineEdit* id_line_edit = nullptr;
        QLineEdit* object_name_line_edit = nullptr;
        QPushButton* object_apply_button = nullptr;
        QPushButton* jump_button = nullptr;

        QPushButton* tf_import_button = nullptr;
        QPushButton* tf_export_button = nullptr;
        QPushButton* tf_apply_button = nullptr;
        QSpinBox* tf_number_spin_box = nullptr;
        QLineEdit* tf_color_synthesizer_line_edit = nullptr;
        QLineEdit* tf_opacity_synthesizer_line_edit = nullptr;
        QComboBox* tf_color_function_combo_box = nullptr;
        QComboBox* tf_opacity_function_combo_box = nullptr;
        QRadioButton* tf_color_user_min_max_radio = nullptr;
        QDoubleSpinBox* tf_color_user_min_spin_box = nullptr;
        QDoubleSpinBox* tf_color_user_max_spin_box = nullptr;
        QRadioButton* tf_opacity_user_min_max_radio = nullptr;
        QDoubleSpinBox* tf_opacity_user_min_spin_box = nullptr;
        QDoubleSpinBox* tf_opacity_user_max_spin_box = nullptr;
        ColorMap* tf_color_map = nullptr;
        OpacityMap* tf_opacity_map = nullptr;
    };

private slots:
    void initTestCase();
    void cleanupTestCase();
    void performs_import_invalid_file_scenario();

private:
    QString envOrDefault( const char* name, const QString& fallback ) const;
    QString repoRootPath() const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void bringTransferFunctionEditorToFront( TransferFunctionEditor* editor ) const;
    void setLineEditText( QLineEdit* line_edit, const QString& text ) const;
    void markStep( const QString& item_id, const QString& description, const QString& status = QStringLiteral( "PASS" ) );
    bool check( ItemResult& result, bool condition, const QString& message );
    void skipItem( ItemResult& result, const QString& reason );
    void finishItem( ItemResult& result );
    void saveScreenshot( ItemResult& result, int seq, const QString& content, const QString& caption, const QString& manual_rel_path );
    void writeMarkdownReport() const;
    void writeStateTable( QTextStream& stream, const QString& title, const EditorState& state ) const;

    ClientHandles resolveClientHandles( MainWindow& window ) const;
    void connectClient( const ClientHandles& client ) const;
    void disconnectClient( const ClientHandles& client ) const;
    void configureRemoteVisualization( const ClientHandles& client ) const;
    void waitForObjectAndApply( const ClientHandles& client ) const;
    void clickJumpAndWaitForCompletion( const ClientHandles& client ) const;
    QFileDialog* waitForFileDialog( int timeout_ms ) const;
    void selectFileFromDialog( const QString& file_path, bool require_existing_file ) const;
    void exportTransferFunction( const ClientHandles& client, const QString& file_path );
    void importTransferFunctionWithoutApply( const ClientHandles& client, const QString& file_path );
    QMessageBox* findVisibleMessageBox() const;
    bool verifyNoMessageBox( ItemResult& result, int wait_ms );

    void selectColorFunction( const ClientHandles& client, int index ) const;
    void selectOpacityFunction( const ClientHandles& client, int index ) const;
    void setDoubleSpinBoxValue( QDoubleSpinBox* spin_box, double value, const char* widget_name ) const;
    EditorState captureEditorState( const ClientHandles& client ) const;
    bool statesEqual( const EditorState& a, const EditorState& b, QStringList* differences ) const;
    bool rangeEntryEqual( const RangeEntry& a, const RangeEntry& b, const QString& label, QStringList* differences ) const;
    bool generateTestFiles( ItemResult& result );

    void runItem57UnreadableFile( ItemResult& result, const ClientHandles& client, const EditorState& baseline );
    void runItem58NoTfNumber( ItemResult& result, const ClientHandles& client, const EditorState& baseline );

    QProcess m_server_process;
    QString m_client_executable;
    QString m_server_executable;
    QString m_server_target_wrapper_executable;
    QString m_volume_data_path;
    QString m_output_dir_path;
    QString m_screenshot_dir_path;
    QString m_manual_dir_path;
    QString m_report_path;
    QString m_base_tfe_path;
    QString m_unreadable_tfe_path;
    QString m_no_tf_number_tfe_path;
    std::vector<StepEntry> m_steps;
    std::vector<ItemResult> m_item_results;
    std::vector<ScreenshotEntry> m_screenshots;
    EditorState m_baseline_state;
    EditorState m_state_after_57;
    EditorState m_state_after_58;
    bool m_has_baseline = false;
    bool m_has_state_57 = false;
    bool m_has_state_58 = false;
    bool m_test_succeeded = false;
};
}

#endif // TRANSFERFUNCTIONEDITORIMPORTINVALIDFILETEST_H

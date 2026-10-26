#ifndef TESTCOMMON_H
#define TESTCOMMON_H

#include <QString>

#include <functional>

class QLineEdit;
class QWidget;

namespace ClientTests
{

/**
 * @brief 環境変数またはテスト設定ファイルからパスを取得します。
 * @param name 取得する環境変数および設定項目の名前。
 * @param fallback 環境変数と設定項目のどちらも空の場合に使用する値。
 * @return 設定されたパスまたはフォールバック値。
 */
QString envOrDefault( const char* name, const QString& fallback = QString() );

/**
 * @brief 実行環境に依存せずにリポジトリのルートディレクトリを取得します。
 * @return Client、Server、.gitを含むディレクトリのパス。
 */
QString repoRootPath();

/**
 * @brief Qtイベントを処理しながら条件が成立するまで待機します。
 * @param condition 繰り返し評価する条件。
 * @param timeout_ms 最大待機時間（ミリ秒）。
 * @param interval_ms 条件を評価する間隔（ミリ秒）。
 * @return 指定時間内に条件が成立した場合はtrue、それ以外はfalse。
 */
bool waitForCondition(
    const std::function<bool()>& condition,
    int timeout_ms,
    int interval_ms = 50 );

/**
 * @brief ウィジェットを表示して前面へ移動します。
 * @param window 前面へ移動するウィジェット。
 * @param settle_ms ウィンドウ操作後にイベント処理を待つ時間（ミリ秒）。
 */
void bringWindowToFront( QWidget* window, int settle_ms = 500 );

/**
 * @brief テスト用のキー入力でQLineEditへ文字列を設定します。
 * @param line_edit 文字列を設定する入力欄。
 * @param text 設定する文字列。
 */
void setLineEditText( QLineEdit* line_edit, const QString& text );

/**
 * @brief テストシナリオの進行状況をログへ出力します。
 * @param message 出力するメッセージ。
 */
void logStep( const QString& message );

} // namespace ClientTests

#endif // TESTCOMMON_H

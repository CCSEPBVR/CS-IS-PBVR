#include "TestCommon.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLineEdit>
#include <QTest>
#include <QtGlobal>
#include <QWidget>

#include "TestOutputPaths.h"

namespace ClientTests
{

namespace
{

// 指定した開始位置から、テスト実行に必要なリポジトリルートを探します。
QString findRepositoryRoot( const QString& start_path )
{
    QDir directory( start_path );
    while ( directory.exists() )
    {
        if ( directory.exists( QStringLiteral( ".git" ) ) &&
             directory.exists( QStringLiteral( "Client" ) ) &&
             directory.exists( QStringLiteral( "Server" ) ) )
        {
            return directory.absolutePath();
        }

        if ( !directory.cdUp() ) { break; }
    }

    return QString();
}

} // namespace

QString envOrDefault( const char* name, const QString& fallback )
{
    // コマンドラインからの環境変数を優先し、未設定時だけINIを参照します。
    const QString value = qEnvironmentVariable( name );
    return value.isEmpty() ? configuredPath( name, repoRootPath(), fallback ) : value;
}

QString repoRootPath()
{
    // 実行ファイル、カレントディレクトリの順で探し、最後にこのソース位置を確認します。
    const QString application_root = findRepositoryRoot( QCoreApplication::applicationDirPath() );
    if ( !application_root.isEmpty() ) { return application_root; }

    const QString current_root = findRepositoryRoot( QDir::currentPath() );
    if ( !current_root.isEmpty() ) { return current_root; }

    const QString source_root = findRepositoryRoot( QFileInfo( QString::fromUtf8( __FILE__ ) ).absolutePath() );
    if ( !source_root.isEmpty() ) { return source_root; }

    return QDir::currentPath();
}

bool waitForCondition(
    const std::function<bool()>& condition,
    int timeout_ms,
    int interval_ms )
{
    // 待機中もQtイベントを処理することで、GUIや通信の状態更新を進めます。
    if ( !condition ) { return false; }

    QElapsedTimer timer;
    timer.start();
    while ( timer.elapsed() < timeout_ms )
    {
        if ( condition() ) { return true; }
        QTest::qWait( interval_ms );
    }

    return condition();
}

void bringWindowToFront( QWidget* window, int settle_ms )
{
    // ウィンドウ操作の前後で共通して必要な表示・アクティブ化を行います。
    QVERIFY2( window != nullptr, "Test target window was not found" );

    window->show();
    window->raise();
    window->activateWindow();
    QTest::qWait( settle_ms );
}

void setLineEditText( QLineEdit* line_edit, const QString& text )
{
    // ファイルパスはOS標準の区切り文字へ統一してからキー入力します。
    QVERIFY2( line_edit != nullptr, "Target line edit was not found" );

    line_edit->setFocus();
    line_edit->clear();
    QTest::keyClicks( line_edit, text );
    QCOMPARE( line_edit->text(), text );
}

void logStep( const QString& message )
{
    // 各テストで同じ形式のシナリオログを出力します。
    qInfo().noquote() << message;
}

} // namespace ClientTests

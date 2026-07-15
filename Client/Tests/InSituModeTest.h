#ifndef INSITUMODETEST_H
#define INSITUMODETEST_H

#include <QObject>
#include <QString>
#include <functional>

class MainWindow;

namespace ClientTests
{
class InSituModeTest : public QObject
{
    Q_OBJECT

public:
    explicit InSituModeTest( QObject* parent = nullptr );

private slots:
    void initTestCase();
    void performs_in_situ_mode_scenario();

private:
    QString repoRootPath() const;
    bool waitForCondition( const std::function<bool()>& condition, int timeout_ms, int interval_ms = 50 ) const;
    void bringWindowToFront( MainWindow* window ) const;
    void saveScreenshot() const;

    QString m_output_dir_path;
    QString m_screenshot_path;
};
}

#endif // INSITUMODETEST_H

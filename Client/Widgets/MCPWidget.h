#pragma once
#include <QDockWidget>
class QCloseEvent;
namespace Ui { class MCPWidget; }

class MCPWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit MCPWidget( QWidget* parent = nullptr );
    ~MCPWidget() override;
public slots:
    void appendEntry( const QString& kind, const QString& text );
protected:
    void closeEvent( QCloseEvent* event ) override;
private:
    Ui::MCPWidget* ui;
};

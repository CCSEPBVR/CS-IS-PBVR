#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QDockWidget>
#include <QWebSocket>

namespace Ui {
class Communication;
}

class Communication : public QDockWidget
{
    Q_OBJECT

public:
    explicit Communication(QWidget *parent = nullptr);
    ~Communication();

private:
    Ui::Communication *ui;
    QWebSocket m_web_socket;

    void initialize();

private slots:
    // UI
    void onConnectClicked();
    void onDisconnectClicked();
};

#endif // COMMUNICATION_H

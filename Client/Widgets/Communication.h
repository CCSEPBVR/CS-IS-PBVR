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
    QWebSocket* m_web_binary_socket = nullptr;
    QWebSocket* m_web_text_socket = nullptr;

    void initialize();

    bool isSocketsConnected() const;

private slots:
    // UI
    void onConnectClicked();
    void onDisconnectClicked();
};

#endif // COMMUNICATION_H

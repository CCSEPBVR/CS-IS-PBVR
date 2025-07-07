#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel
{
    Q_OBJECT

signals:
    void doubleClicked();

public:
    explicit ClickableLabel(QWidget* parent = nullptr);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};

#endif // CLICKABLELABEL_H

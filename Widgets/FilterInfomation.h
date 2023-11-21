#ifndef FILTERINFOMATION_H
#define FILTERINFOMATION_H

#include <QDockWidget>

namespace Ui {
class FilterInfomation;
}

class FilterInfomation : public QDockWidget
{
    Q_OBJECT

public:
    explicit FilterInfomation(QWidget *parent = nullptr);
    ~FilterInfomation();

private:
    Ui::FilterInfomation *ui;
};

#endif // FILTERINFOMATION_H

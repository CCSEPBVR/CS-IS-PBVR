#ifndef FILTERINFOMATION_H
#define FILTERINFOMATION_H

#include <QDockWidget>
#include "ParticleTransferProtocol.h"

namespace Ui {
class FilterInfomation;
}

class FilterInfomation : public QDockWidget
{
    Q_OBJECT

public:
    explicit FilterInfomation(QWidget *parent = nullptr);
    ~FilterInfomation();
    void updateFilterInfomation( QString volumeDataFilePath, jpv::ParticleTransferServerMessage reply );

private:
    Ui::FilterInfomation *ui;
};

#endif // FILTERINFOMATION_H

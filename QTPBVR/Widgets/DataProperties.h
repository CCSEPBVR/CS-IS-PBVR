#ifndef DATAPROPERTIES_H
#define DATAPROPERTIES_H

#include <QDockWidget>
#include "ParticleTransferProtocol.h"

namespace Ui {
class DataProperties;
}

class DataProperties : public QDockWidget
{
    Q_OBJECT

public:
    explicit DataProperties(QWidget *parent = nullptr);
    ~DataProperties();
    void updateFilterInfomation( QString volumeDataFilePath, jpv::ParticleTransferServerMessage& reply );

private:
    Ui::DataProperties *ui;
};

#endif // DATAPROPERTIES_H

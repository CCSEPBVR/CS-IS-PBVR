#ifndef DATASUMMARY_H
#define DATASUMMARY_H

#include <QWidget>

namespace Ui {
class DataSummary;
}

class DataSummary : public QWidget
{
    Q_OBJECT

public:
    explicit DataSummary(QWidget *parent = nullptr);
    ~DataSummary();

public:
    void setTotalParticles( int totalParticles );
private:
    Ui::DataSummary *ui;
};

#endif // DATASUMMARY_H

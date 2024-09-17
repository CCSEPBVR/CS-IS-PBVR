#ifndef COORDINATES_H
#define COORDINATES_H

#include <QDialog>
#include "Widgets/MergePanel.h"
namespace Ui {
class Coordinates;
}

class Coordinates : public QDialog
{
    Q_OBJECT

public:
    explicit Coordinates(QWidget *parent = nullptr, MergePanel* merge = nullptr, Connect* connect_panel = nullptr);
    ~Coordinates();

private:
    Ui::Coordinates *ui;
    MergePanel* m_merge;
    Connect* m_connect;

private slots:
    void onApplyButtonClicked();
};

#endif // COORDINATES_H

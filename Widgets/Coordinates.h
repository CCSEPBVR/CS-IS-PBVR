#ifndef COORDINATES_H
#define COORDINATES_H

#include <QDockWidget>

namespace Ui {
class Coordinates;
}

class Coordinates : public QDockWidget
{
    Q_OBJECT

public:
    explicit Coordinates(QWidget *parent = nullptr);
    ~Coordinates();

private:
    Ui::Coordinates *ui;
};

#endif // COORDINATES_H

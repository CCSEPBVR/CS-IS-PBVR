#ifndef SHADINGCONTROLLER_H
#define SHADINGCONTROLLER_H

#include <QDockWidget>

namespace Ui {
class ShadingController;
}

class ShadingController : public QDockWidget
{
    Q_OBJECT

public:
    explicit ShadingController(QWidget *parent = nullptr);
    ~ShadingController();

private:
    Ui::ShadingController *ui;
};

#endif // SHADINGCONTROLLER_H

#ifndef RENDEROPTIONS_H
#define RENDEROPTIONS_H

#include <QDockWidget>

namespace Ui {
class RenderOptions;
}

class RenderOptions : public QDockWidget
{
    Q_OBJECT

public:
    explicit RenderOptions(QWidget *parent = nullptr);
    ~RenderOptions();

private:
    Ui::RenderOptions *ui;
};

#endif // RENDEROPTIONS_H

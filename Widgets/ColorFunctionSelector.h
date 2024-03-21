#ifndef COLORFUNCTIONSELECTOR_H
#define COLORFUNCTIONSELECTOR_H

#include <QWidget>

namespace Ui {
class ColorFunctionSelector;
}

class ColorFunctionSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ColorFunctionSelector(QWidget *parent = nullptr);
    ~ColorFunctionSelector();

private:
    Ui::ColorFunctionSelector *ui;
};

#endif // COLORFUNCTIONSELECTOR_H

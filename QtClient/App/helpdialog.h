#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QMainWindow>

namespace Ui {
class HelpDialog;
}

/**
 * @brief
 *
 */
class HelpDialog : public QMainWindow
{

public:

    explicit HelpDialog(QWidget *parent = 0);
    ~HelpDialog();

private:
    Ui::HelpDialog *ui; 
};

#endif // HELPDIALOG_H

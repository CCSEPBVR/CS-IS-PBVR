#ifndef REPETITIONLEVELCONTROL_H
#define REPETITIONLEVELCONTROL_H

#include <QDockWidget>

namespace Ui
{
class RepetitionLevelControl;
}

class RepetitionLevelControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit RepetitionLevelControl( QWidget *parent = nullptr );
    ~RepetitionLevelControl();

private:
    // メンバ変数群
    Ui::RepetitionLevelControl *ui;

    // メソッド群
    void initialize();
};

#endif // REPETITIONLEVELCONTROL_H

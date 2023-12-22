#ifndef TRANSFERFUNCTIONEDITOR_H
#define TRANSFERFUNCTIONEDITOR_H

#include <QDialog>

namespace Ui {
class TransferFunctionEditor;
}

class TransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TransferFunctionEditor(QWidget *parent = nullptr);
    ~TransferFunctionEditor();

private:
    Ui::TransferFunctionEditor *ui;
};

#endif // TRANSFERFUNCTIONEDITOR_H

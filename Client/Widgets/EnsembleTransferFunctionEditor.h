#ifndef ENSEMBLETRANSFERFUNCTIONEDITOR_H
#define ENSEMBLETRANSFERFUNCTIONEDITOR_H

#include <QDialog>

class QButtonGroup;

namespace Ui
{
class EnsembleTransferFunctionEditor;
}

class EnsembleTransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit EnsembleTransferFunctionEditor( QWidget* parent = nullptr );
    ~EnsembleTransferFunctionEditor();

private:
    Ui::EnsembleTransferFunctionEditor* ui;

    QButtonGroup* m_statistics_group = nullptr;
    QButtonGroup* m_average_min_max_group = nullptr;
    QButtonGroup* m_variance_min_max_group = nullptr;
    QButtonGroup* m_coefficient_variation_min_max_group = nullptr;

    void initializeButtonGroups();

    // Future signal/slot extension point:
    // add statistic selection, MinMax mode, transfer function apply,
    // and histogram update connections here when ensemble TF processing is implemented.
};

#endif // ENSEMBLETRANSFERFUNCTIONEDITOR_H

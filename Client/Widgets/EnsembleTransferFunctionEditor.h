#ifndef ENSEMBLETRANSFERFUNCTIONEDITOR_H
#define ENSEMBLETRANSFERFUNCTIONEDITOR_H

#include <cstddef>
#include <QDialog>

#include "WebSocketPair.h"

class QButtonGroup;

namespace Ui
{
class EnsembleTransferFunctionEditor;
}

class EnsembleTransferFunctionEditor : public QDialog
{
    Q_OBJECT

public:
    explicit EnsembleTransferFunctionEditor( WebSocketPair* websockets, QWidget* parent = nullptr );
    ~EnsembleTransferFunctionEditor();

signals:
    void statisticChanged( const QString& statistic );

public slots:
    void setRepeatLevel( size_t repeatLevel );

private:
    Ui::EnsembleTransferFunctionEditor* ui;

    WebSocketPair* m_web_sockets = nullptr;
    size_t m_repeat_level = 4;

    QButtonGroup* m_statistics_group = nullptr;
    QButtonGroup* m_average_min_max_group = nullptr;
    QButtonGroup* m_variance_min_max_group = nullptr;
    QButtonGroup* m_coefficient_variation_min_max_group = nullptr;

    void initializeButtonGroups();
    QString selectedStatistic() const;

private slots:
    void onApply();
};

#endif // ENSEMBLETRANSFERFUNCTIONEDITOR_H

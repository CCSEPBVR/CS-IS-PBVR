#ifndef ENSEMBLETRANSFERFUNCTIONEDITOR_H
#define ENSEMBLETRANSFERFUNCTIONEDITOR_H

#include <cstddef>
#include <QDialog>

#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"
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
    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    QButtonGroup* m_statistics_group = nullptr;
    QButtonGroup* m_average_min_max_group = nullptr;
    QButtonGroup* m_variance_min_max_group = nullptr;
    QButtonGroup* m_coefficient_variation_min_max_group = nullptr;

    void initializeButtonGroups();
    void initializeTransferFunctionWidgets();
    QString selectedStatistic() const;
    bool validateForApply() const;
    bool validateForExport() const;

    bool eventFilter( QObject* watched, QEvent* event ) override;

private slots:
    void onApply();
    void onExport();
    void onImport();
};

#endif // ENSEMBLETRANSFERFUNCTIONEDITOR_H

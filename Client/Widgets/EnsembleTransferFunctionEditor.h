#ifndef ENSEMBLETRANSFERFUNCTIONEDITOR_H
#define ENSEMBLETRANSFERFUNCTIONEDITOR_H

#include <QColor>
#include <QDialog>
#include <QString>
#include <QVector>

#include <array>
#include <vector>

#include "ColorMapEditor.h"
#include "OpacityMapEditor.h"
#include "WebSocketPair.h"

class QButtonGroup;
class QJsonObject;

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
    void reset();
    QString selectedStatistic() const;

public slots:
    void onReceiveEnsembleStatisticsParameter( const QJsonObject& payload );

private:
    Ui::EnsembleTransferFunctionEditor* ui;

    WebSocketPair* m_web_sockets = nullptr;
    ColorMapEditor m_color_map_editor;
    OpacityMapEditor m_opacity_map_editor;

    enum StatisticIndex
    {
        AverageStatistic = 0,
        VarianceStatistic,
        CoefficientVariationStatistic,
        StatisticCount
    };

    struct StatisticUiState
    {
        QString displayName;
        QString statisticName;
        QVector<QColor> colorMap;
        QVector<float> opacityMap;
        bool useUserMinMax = true;
        double userMin = 0.0;
        double userMax = 1.0;
        double serverMin = 0.0;
        double serverMax = 1.0;
        std::vector<int> histogram;
    };

    QButtonGroup* m_min_max_group = nullptr;
    std::array<StatisticUiState, StatisticCount> m_statistics;
    QVector<QColor> m_default_color_map;
    QVector<float> m_default_opacity_map;
    StatisticIndex m_current_statistic = AverageStatistic;

    void initialize();
    void saveCurrentStatisticState();
    void loadStatisticState( StatisticIndex statistic );
    StatisticIndex selectedStatisticIndex() const;
    bool validateForApply() const;
    bool validateForExport() const;

    bool eventFilter( QObject* watched, QEvent* event ) override;

private slots:
    void onStatisticSelectionChanged( int index );
    void onApply();
    void onExport();
    void onImport();
};

#endif // ENSEMBLETRANSFERFUNCTIONEDITOR_H

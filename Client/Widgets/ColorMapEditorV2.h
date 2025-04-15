#ifndef ColorMapEditorV2_H
#define ColorMapEditorV2_H

#include <QDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QColorDialog>

#include "function.h"
#include "function_parser.h"
#include "ColorMap.h"

namespace Ui {
class ColorMapEditorV2;
}

class ColorMapEditorV2 : public QDialog
{
    Q_OBJECT

public:
    explicit ColorMapEditorV2( QWidget *parent = nullptr );
    ~ColorMapEditorV2();
    void setDefaultColorMap( const QVector<QColor>& colors );
    QVector<QColor> getColorMap();

private:
    Ui::ColorMapEditorV2 *ui;
    void initialize();
    void initializePreset();
    void initializeFreeformCurve();
    void initializeExpression();
    void initializeControlPoints();
    QList<QPair<QString, QVector<QColor>>> loadDefaultColorMap( const QString& filePath );

private:
    QVector<QColor> m_default_colors;


private slots:
    void onReset();
    void onTabChanged( int index );
    void onPresetColorMapDoubleClicked( int row, int column );
    void onDrawingColorDoubleClicked();
    void onExpressionChanged();
    void onNumberOfControlPointsChanged( int value );
    void onControlPointChanged();
};

#endif // ColorMapEditorV2_H

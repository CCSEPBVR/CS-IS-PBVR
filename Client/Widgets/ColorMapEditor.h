#ifndef COLORMAPEDITOR_H
#define COLORMAPEDITOR_H

#include <QDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QColorDialog>

#include "Function.h"
#include "FunctionParser.h"
#include "ColorMap.h"

namespace Ui {
class ColorMapEditor;
}

class ColorMapEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ColorMapEditor( QWidget *parent = nullptr );
    ~ColorMapEditor();
    void setDefaultColorMap( const QVector<QColor>& colors );
    QVector<QColor> getColorMap();

private:
    Ui::ColorMapEditor *ui;
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

#endif // COLORMAPEDITOR_H

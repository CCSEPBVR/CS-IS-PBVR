#ifndef OPACITYMAPEDITOR_H
#define OPACITYMAPEDITOR_H

#include <QDialog>

#include "Function.h"
#include "FunctionParser.h"
#include "ColorMap.h"

namespace Ui {
class OpacityMapEditor;
}

class OpacityMapEditor : public QDialog
{
    Q_OBJECT

public:
    explicit OpacityMapEditor(QWidget *parent = nullptr);
    ~OpacityMapEditor();
    void setDefaultOpacityMap( const QVector<float>& colors );
    QVector<float> getOpacityMap();

private:
    Ui::OpacityMapEditor *ui;
    void initialize();
    void initializeExpression();
    void initializeControlPoints();

private:
    QVector<float> m_default_opacities;

private slots:
    void onReset();
    void onTabChanged( int index );
    void onExpressionChanged();
    void onNumberOfControlPointsChanged( int value );
    void onControlPointChanged();
};

#endif // OPACITYMAPEDITOR_H

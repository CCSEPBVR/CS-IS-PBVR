#ifndef OpacityMapEditor_H
#define OpacityMapEditor_H

#include <QDialog>

#include "function.h"
#include "function_parser.h"
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

#endif // OpacityMapEditor_H

#ifndef OpacityMapEditorV2_H
#define OpacityMapEditorV2_H

#include <QDialog>

#include "function.h"
#include "function_parser.h"
#include "ColorMap.h"

namespace Ui {
class OpacityMapEditorV2;
}

class OpacityMapEditorV2 : public QDialog
{
    Q_OBJECT

public:
    explicit OpacityMapEditorV2(QWidget *parent = nullptr);
    ~OpacityMapEditorV2();
    void setDefaultOpacityMap( const QVector<float>& colors );
    QVector<float> getOpacityMap();

private:
    Ui::OpacityMapEditorV2 *ui;
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

#endif // OpacityMapEditorV2_H

#ifndef POINTSIZECONTROL_H
#define POINTSIZECONTROL_H

#include <QDockWidget>

#include "Screen.h"
#include <kvs/IDManager>
#include <kvs/RendererManager>
#include <kvs/StochasticRendererBase>
#include <kvs/ParticleBasedRenderer>

namespace Ui
{
class PointSizeControl;
}

class PointSizeControl : public QDockWidget
{
    Q_OBJECT

public:
    explicit PointSizeControl( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~PointSizeControl();

private:
    // メンバ変数群
    Ui::PointSizeControl *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    // メソッド群
    void initialize();

private slots:
    void onPointSizeValueChanged();

public slots:
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );
};

#endif // POINTSIZECONTROL_H

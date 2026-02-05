#ifndef VOLUMETRANSFORM_H
#define VOLUMETRANSFORM_H

#include <QDockWidget>

#include <kvs/RotationMatrix33>
#include "Screen.h" // <kvs/qt/Screen>

namespace Ui
{
class VolumeTransform;
}

class VolumeTransform : public QDockWidget
{
    Q_OBJECT

public:
    explicit VolumeTransform( kvs::qt::jaea::Screen* screen, QWidget *parent = nullptr );
    ~VolumeTransform();

signals:
    void updateTranslation();

public slots:
    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::VolumeTransform *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

private slots:
    void onApply();
};

#endif // VOLUMETRANSFORM_H

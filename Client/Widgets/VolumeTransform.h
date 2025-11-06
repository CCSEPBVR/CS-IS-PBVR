#ifndef VOLUMETRANSFORM_H
#define VOLUMETRANSFORM_H

#include <QDockWidget>

#include "Screen.h"
#include <kvs/RotationMatrix33>

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

public slots:
    void loadParameter( const QString& filePath );
    void saveParameter( const QString& filePath );

signals:
    void updatePointsTranslation(); // FIXME:シグナルスロットの接続が必要

private:
    Ui::VolumeTransform *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    void initialize();

private slots:
    void onApply();
};

#endif // VOLUMETRANSFORM_H

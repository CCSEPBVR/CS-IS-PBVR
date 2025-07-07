#ifndef VOLUMETRANSFORM_H
#define VOLUMETRANSFORM_H

#include <QDockWidget>

#include "Screen.h"
#include <kvs/ObjectManager>
#include <kvs/RotationMatrix33>

namespace Ui {
class VolumeTransform;
}

class VolumeTransform : public QDockWidget
{
    Q_OBJECT

public:
    explicit VolumeTransform( kvs::qt::jaea::Screen*,
                              QWidget *parent = nullptr );
    ~VolumeTransform();

private:
    Ui::VolumeTransform *ui;
    kvs::qt::jaea::Screen* m_screen = nullptr;

    void initialize();

signals:
    void updatePointsTranslation();

private slots:
    void onApply();
};

#endif // VOLUMETRANSFORM_H

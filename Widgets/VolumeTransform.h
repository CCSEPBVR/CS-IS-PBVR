#ifndef VOLUMETRANSFORM_H
#define VOLUMETRANSFORM_H

#include <QDockWidget>

#include "ExtendedKVS/Screen.h"
#include <kvs/RotationMatrix33>

namespace Ui {
class VolumeTransform;
}

class VolumeTransform : public QDockWidget
{
    Q_OBJECT

public:
    explicit VolumeTransform(QWidget *parent = nullptr);
    ~VolumeTransform();

public:
    void setScreen( kvs::qt::jaea::Screen* screen ) { m_screen = screen; }

private:
    Ui::VolumeTransform *ui;
    kvs::qt::jaea::Screen* m_screen;

private slots:
    void onApplyButtonClicked();
};

#endif // VOLUMETRANSFORM_H

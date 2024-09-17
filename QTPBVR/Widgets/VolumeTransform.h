#ifndef VOLUMETRANSFORM_H
#define VOLUMETRANSFORM_H

#include <QDockWidget>

#include "ExtendedKVS/Screen.h"
#include <kvs/RotationMatrix33>

class PBVRGUI;

namespace Ui {
class VolumeTransform;
}

class VolumeTransform : public QDockWidget
{
    Q_OBJECT

public:
    explicit VolumeTransform(QWidget *parent = nullptr, PBVRGUI *pbvr_gui = nullptr);
    ~VolumeTransform();

private:
    Ui::VolumeTransform *ui;
    PBVRGUI *m_pbvr_gui;

private slots:
    void onApplyButtonClicked();
};

#endif // VOLUMETRANSFORM_H

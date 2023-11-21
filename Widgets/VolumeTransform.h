#ifndef VOLUMETRANSFORM_H
#define VOLUMETRANSFORM_H

#include <QDockWidget>

#include <kvs/qt/Screen>
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
    void setScreen( kvs::qt::Screen* screen ) { m_screen = screen; }

private:
    Ui::VolumeTransform *ui;
    kvs::qt::Screen* m_screen;

private slots:
    void onApplyButtonClicked();
};

#endif // VOLUMETRANSFORM_H

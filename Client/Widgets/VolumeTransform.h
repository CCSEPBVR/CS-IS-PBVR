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

private:
    // メンバ変数群
    Ui::VolumeTransform *ui;

    kvs::qt::jaea::Screen* m_screen = nullptr;

    // メソッド群
    void initialize();

private slots:
    void onApply();

signals:
    void updatePointsTranslation(); // FIXME:シグナルスロットの接続が必要
};

#endif // VOLUMETRANSFORM_H

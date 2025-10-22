#ifndef OBJECTEDITORWIP_H
#define OBJECTEDITORWIP_H

#include <QDockWidget>
#include <QList>
#include <QFileDialog>

#include "Screen.h"

namespace Ui
{
class ObjectEditorWIP;
}

class ObjectEditorWIP : public QDockWidget
{
    Q_OBJECT

public:
    explicit ObjectEditorWIP(QWidget *parent = nullptr);
    ~ObjectEditorWIP();

private:
    // メンバ変数群
    Ui::ObjectEditorWIP *ui;

    QList<QWidget*> m_group_common_object_widgets;  // 全オブジェクト共通で表示させるウィジェット群
    QList<QWidget*> m_group_server_object_widgets;  // サーバオブジェクト(kvs::PointObject)で表示させるウィジェット群
    QList<QWidget*> m_group_nontexture_polygon_object_widgets; // テクスチャ無しポリゴンオブジェクト(.stl)を表示させるウィジェット群

    kvs::qt::jaea::Screen* m_screen = nullptr;

    // メソッド群
    void initialize();
    void toggleCommonObjectWidgets( bool isObject ); // 選択したアイテムがオブジェクトかどうか。 // true:オブジェクトである。 // FIXME:そもそもアイテムない状態で選択はできないのでは。
    void toggleServerObjectWidgets( bool isServerObject ); // true:サーバオブジェクトである。
    void toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject ); // true:テクスチャ無しポリゴンオブジェクトである。(.stl)

private slots:
    void onBrowse(); // ...ボタン(ファイルブラウザ)が押された時
    void onDelete();
    void onApply();
};

#endif // OBJECTEDITORWIP_H

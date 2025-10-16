#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDialog>

namespace Ui
{
class GlyphEditor;
}

class GlyphEditor : public QDialog
{
    Q_OBJECT

public:
    explicit GlyphEditor( QWidget *parent = nullptr );
    ~GlyphEditor();

private:
    // メンバ変数群
    Ui::GlyphEditor *ui;

    //  メソッド群
    void initialize();
};

#endif // GLYPHEDITOR_H

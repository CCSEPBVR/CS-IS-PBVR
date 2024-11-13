#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDockWidget>

namespace Ui {
class GlyphEditor;
}

class GlyphEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit GlyphEditor(QWidget *parent = nullptr);
    ~GlyphEditor();

private:
    Ui::GlyphEditor *ui;
};

#endif // GLYPHEDITOR_H

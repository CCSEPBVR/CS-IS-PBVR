#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDockWidget>
#include <kvs/ColorMap>

class MergePanel;
class Connect;

namespace Ui {
class GlyphEditor;
}

class GlyphEditor : public QDockWidget
{
    Q_OBJECT

public:
    enum GlyphType
    {
        Arrow   = 0, //Defaukt
        Diamond = 1,
        Sphere  = 2,
    };

    enum DataDefines
    {
        Constant            = 0, //
        SingleVariable      = 1, //
        VariableArray       = 2  //
    };

    enum GlyphMode
    {
        UniformDistribution = 0, //max sampepoints,seed
        AllPoints           = 1, //No UI
        EveryNthPoints      = 2  //Stride
    };

public:
    explicit GlyphEditor(QWidget *parent = nullptr, MergePanel* merge = nullptr, Connect* connect_panel = nullptr);
    ~GlyphEditor();

private:
    Ui::GlyphEditor *ui;
    MergePanel* m_merge;
    Connect* m_connect;

    GlyphType m_glyph_type;
    std::string m_glyph_direction_1;
    std::string m_glyph_direction_2;
    std::string m_glyph_direction_3;
    DataDefines m_glyph_size;
    float m_scale_factor;
    GlyphMode m_glyph_mode;
    kvs::ColorMap m_color_map; //need default defined
    DataDefines m_color_data;

private slots:
    void onApplyButtonClicked();
};

#endif // GLYPHEDITOR_H

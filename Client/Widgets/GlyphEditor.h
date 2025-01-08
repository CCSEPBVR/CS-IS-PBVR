#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDockWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <kvs/ColorMap>

#include "ParticleTransferProtocol.h"
#include "Widgets/ColorMapEditor.h"
class PBVRGUI;
class MergePanel;
class Connect;
class ShadingController;

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
    explicit GlyphEditor(QWidget *parent = nullptr,
                         PBVRGUI *pbvr_gui = nullptr,
                         MergePanel* merge = nullptr,
                         Connect* connect_panel = nullptr,
                         ShadingController* shading_controller = nullptr);
    void updateNumberOfVector( jpv::ParticleTransferServerMessage& server_message );
    ~GlyphEditor();
    int getGlyphType() const { return static_cast<int>(m_glyph_type);}
    float getScaleFactor() const { return m_scale_factor; }

private:
    Ui::GlyphEditor *ui;
    PBVRGUI *m_pbvr_gui;
    MergePanel* m_merge;
    Connect* m_connect;
    ShadingController* m_shading_controller;
    ColorMapEditor m_color_map_editor;

    int m_direction_previus_index[3];

    QStringList* m_vector_list;

    QVBoxLayout* m_size_variable_layout;
    QVector<QLabel*> m_size_variable_labels;
    QVector<QComboBox*> m_size_variable_combo_boxes;

    QVBoxLayout* m_color_data_variable_layout;
    QVector<QLabel*> m_color_data_variable_labels;
    QVector<QComboBox*> m_color_data_variable_combo_boxes;

    GlyphType m_glyph_type;
    std::string m_glyph_direction_1;
    std::string m_glyph_direction_2;
    std::string m_glyph_direction_3;
    DataDefines m_glyph_size;
    float m_scale_factor;
    GlyphMode m_glyph_mode;
    kvs::ColorMap m_color_map; //need default defined
    DataDefines m_color_data;

public:
    kvs::ValueArray<kvs::Real32> m_coords;
    kvs::ValueArray<kvs::Real32> m_directions;
    kvs::ValueArray<kvs::Real32> m_sizes;
    kvs::ValueArray<kvs::UInt8>  m_colors;

public:
    void enableGlyphUpdateButton();
    void disableGlyphUpdateButton();
    void glyphInitialize();

private:
    void directionComboBoxBlockSignals( bool block );

private slots:
    void onDirectionComboBoxIndexChanged( int index );
    void onSizeNumberOfVariableChanged( int value );
    void onColorDataNumberOfVariableChanged( int value );
    void onUpdateButtonClicked();
    void onEditColorMapButtonClicked();
    void onApplyButtonClicked();
};

#endif // GLYPHEDITOR_H

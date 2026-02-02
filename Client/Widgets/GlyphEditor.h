#ifndef GLYPHEDITOR_H
#define GLYPHEDITOR_H

#include <QDialog>
#include <QSpinBox>
#include <QGridLayout>

#include "ColorMapEditor.h"

#include "WebSocketPair.h"

#include "../../Shared/JsonKeys.h"
#include "../../Shared/GlyphParameter.h"

namespace Ui
{
class GlyphEditor;
}

class GlyphEditor : public QDialog
{
    Q_OBJECT

public:
    explicit GlyphEditor( WebSocketPair* websockets, QWidget *parent = nullptr );
    ~GlyphEditor();
    void reset();

signals:
    void updateStatusBarMessage( const QString& message );
    void glyphParameterUpdate();

public slots:
    void onOperatorStateUpdate( const bool operatorState ); // true: 権限有り, false: 権限無し

    void onUpdateNumberOfVector( const int numberOfVector );

    // NOTE:テキストソケット用
    void onReceiveGlyphParameter( const QJsonObject& payload );
    // void onReceiveGlyphParameter( const QJsonObject& payload );

    void onLoadParameter( const QString& filePath ); // KPI
    void onSaveParameter( const QString& filePath ); // KPI

private:
    Ui::GlyphEditor *ui;

    WebSocketPair* m_web_sockets = nullptr;

    bool m_is_operator           = false;

    QStringList m_variable_list;
    ColorMapEditor m_color_map_editor;

    struct GlyphUiSnapshot
    {
        int type = 0;
        double scaleFactor = 1.0;

        int dir1 = 0;
        int dir2 = 0;
        int dir3 = 0;

        GlyphParameter::DataMode sizeMode = GlyphParameter::DataMode::Constant;
        QVector<int> sizeVariables;

        GlyphParameter::DistributionMode distributionMode = GlyphParameter::DistributionMode::UniformDistribution;
        int numberOfSamplePoints = 0;
        int seed = 0;
        int stride = 1;

        QVector<QColor> colorMap;

        GlyphParameter::DataMode colorDataMode = GlyphParameter::DataMode::Constant;
        QVector<int> colorDataVariables;
    };

    GlyphUiSnapshot m_last_snap_shot;
    bool m_has_last_snap_shot = false;

    GlyphUiSnapshot captureUiSnapshot() const;
    static QVector<int> captureVariableIndicesFromGrid( const QGridLayout* grid, int count );
    static bool fuzzyEqual( double a, double b );

    // NOTE:updateUI
    void updateUiEnableState( const bool isVariableArray, QGridLayout* gridLayout );
    void updateVariableGridLayout( QSpinBox* spinBox, QGridLayout* grid );

private slots:
    void onDirectionComboBoxIndexChanged();
    void onSizeUiState();
    void onSizeNumberOfVariableValueChanged();
    void onColorMapEditorClicked();
    void onColorDataUiState();
    void onColorDataNumberOfVariableValueChanged();
    void onApply();
};

#endif // GLYPHEDITOR_H

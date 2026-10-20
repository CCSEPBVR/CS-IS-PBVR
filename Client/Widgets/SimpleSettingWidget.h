#ifndef SIMPLESETTINGWIDGET_H
#define SIMPLESETTINGWIDGET_H

#include <QDockWidget>
#include <QTimer>
#include "ColorMapEditor.h"

class Communication;
class TransferFunctionEditor;
class ObjectEditor;
class GlyphEditor;
class PlotOverLineEditor;
class PointSizeControl;
class RepetitionLevelControl;
class WebSocketPair;
class QButtonGroup;
class QPushButton;
class QLabel;

class SimpleSettingWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit SimpleSettingWidget( Communication* communication, TransferFunctionEditor* editor,
                                  WebSocketPair* sockets, ObjectEditor* objects, GlyphEditor* glyphs,
                                  PointSizeControl* pointSize, RepetitionLevelControl* repetition,
                                  PlotOverLineEditor* plotOverLine,
                                  QWidget* parent = nullptr );

signals:
    void selectColorFunction( int number );
    void updateStatusBarMessage( const QString& message );
    void automaticJumpRequested();
    void targetLoadCompleted();

public slots:
    void onAutomaticJumpStarted();
    void onAutomaticJumpFailed();
    void onDataRequestCompleted( int timeStep );

private:
    enum class Stage { Idle, Disconnecting, Connecting, Initializing, WaitingForObjects, AddingTerrain, ApplyingColorMap,
                       WaitingForJump, WaitingForJumpData };
    Communication* m_communication;
    TransferFunctionEditor* m_editor;
    WebSocketPair* m_sockets;
    ObjectEditor* m_objects;
    GlyphEditor* m_glyphs;
    PlotOverLineEditor* m_plot_over_line;
    PointSizeControl* m_point_size;
    RepetitionLevelControl* m_repetition;
    QButtonGroup* m_targets;
    QPushButton* m_target_apply;
    ColorMap* m_preview;
    QPushButton* m_color_select;
    QPushButton* m_color_apply;
    QString m_last_error;
    ColorMapEditor m_selector;
    QTimer m_timeout;
    Stage m_stage = Stage::Idle;
    int m_requested_target = -1;
    int m_applied_target = -1;
    bool m_pending_color = false;
    bool m_external_initializing = false;
    bool m_jump_completes_target_load = false;
    quint64 m_generation = 0;
    QString m_terrain_uuid;

    void applyTarget();
    void startConnection();
    void finishInitialization( bool success, const QString& reason );
    void startTerrainAddition();
    void applyInitialSettings();
    void finishTerrainAddition( bool success, const QString& reason = QString() );
    void startAutomaticJump();
    void beginOperation();
    void finishOperation();
    void fail( const QString& reason );
    void invalidateTarget();
    void refreshColorMap();
    void updateControls();
    void setStatus( const QString& message, bool error = false );
    int appliedColorFunction() const;
};

#endif // SIMPLESETTINGWIDGET_H

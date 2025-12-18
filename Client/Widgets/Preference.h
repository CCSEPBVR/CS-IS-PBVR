#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QDialog>
#include <QColorDialog>

#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>

#include "ClickableLabel.h"

namespace Ui
{
class Preference;
}

class Preference : public QDialog
{
    Q_OBJECT

public:
    enum OrientationType // for color map bar
    {
        Horizontal,
        Vertical
    };

    enum AxisType // for orientation axis
    {
        CorneredAxis,
        CenteredAxis,
        NoneAxis
    };

    enum BoxType // for orientation axis
    {
        WiredBox,
        SolidBox,
        NoneBox
    };

    explicit Preference( QWidget *parent = nullptr );
    ~Preference();

    void setScreen( kvs::qt::jaea::Screen* screen )                      { m_screen = screen; }
    void setCompositor( kvs::StochasticRenderingCompositor* compositor ) { m_compositor = compositor; }
    void setColorMapBar( kvs::ColorMapBar* color_map_bar )
    {
        m_color_map_bar = color_map_bar;
        m_color_map_bar->anchorToTopLeft();
    }
    void setOrientationAxis( kvs::OrientationAxis* orientation_axis )
    {
        m_orientation_axis = orientation_axis;
        m_orientation_axis->anchorToBottomRight();
    }
    void setFpsLabel( kvs::Label* fps_label )            { m_fps_label = fps_label; }
    void setTimeStepLabel( kvs::Label* time_step_label ) { m_time_step_label = time_step_label; }

    kvs::qt::jaea::Screen* screen()                  const { return m_screen; }
    kvs::StochasticRenderingCompositor* compositor() const { return m_compositor; }
    kvs::ColorMapBar* colorMapBar()                  const { return m_color_map_bar; }
    kvs::OrientationAxis* orientationAxis()          const { return m_orientation_axis; }
    kvs::Label* fpsLabel()                           const { return m_fps_label; }
    kvs::Label* timeStepLabel()                      const { return m_time_step_label; }

public slots:
    void onReadyScreen();
    void onDataRequestCompleted( const int requestTimeStep );

protected:
    void closeEvent( QCloseEvent *event ) override;

private:
    Ui::Preference *ui;

    kvs::qt::jaea::Screen* m_screen                  = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor = nullptr;
    kvs::ColorMapBar* m_color_map_bar                = nullptr;
    kvs::OrientationAxis* m_orientation_axis         = nullptr;
    kvs::Label* m_fps_label                          = nullptr;
    kvs::Label* m_time_step_label                    = nullptr;

    QSettings m_settings;
    int m_current_time_step = -1;

    bool isConfigFileExists() { return QFile::exists( QCoreApplication::applicationDirPath() + "/config.ini" ); }

    void loadSettings();
    void loadColorMapBarSetting();
    void loadOrientationAxisSetting();
    void loadBackgroundColorSetting();
    void loadResolutionSetting();
    void loadLabelSetting();
    void loadFontColorSetting();

    void saveSettings();
    void saveColorMapBarSetting();
    void saveOrientationAxisSetting();
    void saveBackgroundColorSetting();
    void saveResolutionSetting();
    void saveLabelSetting();
    void saveFontColorSetting();

    void applySettings();
    void applyColorMapBarSetting();
    void applyOrientationAxisSetting();
    void applyBackgroundColorSetting();
    void applyResolutionSetting();
    void applyLabelSetting();
    void applyFontColorSetting();

    void defaultSettings();
    void setColor( ClickableLabel& clickableLabel, const QColor& color );

private slots:
    void onBackGroundColorDoubleClicked();
    void onLabelsColorDoubleClicked();
    void onDefault();
    void onApply();
    void onCancel();
    void onOK();
};

#endif // PREFERENCE_H

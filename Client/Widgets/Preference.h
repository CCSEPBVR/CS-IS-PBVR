#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QDialog>
#include <QSettings>
#include <QFile>
#include <QColorDialog>

#include "Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/Label>

#include "ClickableLabel.h"

namespace Ui {
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

public:
    explicit Preference( kvs::qt::jaea::Screen*,
                         kvs::StochasticRenderingCompositor*,
                         kvs::ColorMapBar*,
                         kvs::OrientationAxis*,                         
                         kvs::Label*,
                         kvs::Label*,
                         QWidget *parent = nullptr );
    ~Preference();
    void readyScreen() { applySettings(); }

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Preference *ui;
    kvs::qt::jaea::Screen* m_screen                             = nullptr;
    kvs::StochasticRenderingCompositor* m_compositor            = nullptr;
    kvs::ColorMapBar* m_color_map_bar                           = nullptr;
    kvs::OrientationAxis* m_orientation_axis                    = nullptr;
    kvs::Label* m_fps_label                                     = nullptr;
    kvs::Label* m_time_step_label                               = nullptr;
    QSettings m_settings;
    int m_current_time_step;

    void initialize();
    bool checkConfigFileExists() { return QFile::exists( "config.ini" ); }

    void defaultSettings();

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

    void setColor( ClickableLabel& clickableLabel, const QColor& color );

public slots:
    void mergingFinish( int );

private slots:
    void onBackGroundColorDoubleClicked();
    void onLabelsColorDoubleClicked();
    void onApply();
    void onDefault();
    void onCancel();
    void onOK();
};

#endif // PREFERENCE_H

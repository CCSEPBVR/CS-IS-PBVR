#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QDialog>
#include <QSettings>

#include "ExtendedKVS/Screen.h"
#include <kvs/StochasticRenderingCompositor>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>
#include <kvs/StochasticRendererBase>
#include <kvs/Label>

namespace Ui {
class Preference;
}

class Preference : public QDialog
{
    Q_OBJECT

public:
    //for color map bar
    enum OrientationType
    {
        Horizontal,
        Vertical
    };

    //for orientation axis
    enum AxisType
    {
        CorneredAxis,
        CenteredAxis,
        NoneAxis
    };

    enum BoxType
    {
        WiredBox,
        SolidBox,
        NoneBox
    };

public:
    explicit Preference( QWidget *parent = nullptr );
    ~Preference();

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    void initialize();

    void setScreen( kvs::qt::jaea::Screen* screen )                            { m_screen = screen;                    }
    void setCompositor( kvs::StochasticRenderingCompositor* compositor ) { m_compositor = compositor;            }
    void setColorMapBar( kvs::ColorMapBar* colorMapBar )                 { m_color_map_bar = colorMapBar;        }
    void setOrientationAxis( kvs::OrientationAxis* orientationAxis )     { m_orientation_axis = orientationAxis; }
    void setFPSLabel( kvs::Label* fps_label )                            { m_fps_label = fps_label;              }
    void setTimeStepLabel( kvs::Label* time_step_label )                 { m_time_step_label = time_step_label;  }
    void setCurrentTimeStep( int currentTimeStep ) { m_time_step_label->setText("Time step: " + std::to_string(currentTimeStep));}

    void doneInitialize()                                                { m_initialized = true;                 }
    void loadShadingSettings();
    void applyShadingSettings();

private:
    Ui::Preference *ui;
    kvs::qt::jaea::Screen* m_screen;
    kvs::StochasticRenderingCompositor* m_compositor;
    kvs::ColorMapBar* m_color_map_bar;
    kvs::OrientationAxis* m_orientation_axis;
    kvs::Label* m_fps_label;
    kvs::Label* m_time_step_label;
    QSettings m_settings;
    bool m_initialized;

private:
    bool checkConfigFileExists() { return QFile::exists( "config.ini" ); }

    void loadColorMapBarSettings();
    void loadOrientationAxisSettings();
    void loadBackGroundColorSettings();
    void loadResolutionSettings();
    void loadLabelsSettings();
    void loadFontSettings();

    void setDefaultSettings();

    void saveSettings();
    void saveColorMapBarSettings();
    void saveOrientationAxisSettings();
    void saveBackGroundColorSettings();
    void saveResolutionSettings();
    void saveLabelsSettings();
    void saveFontSettings();
    void saveShadingSettings();

    void applySettings( bool isInit );
    void applyColorMapBarSettings();
    void applyOrientationAxisSettings();
    void applyBackGroundColor();
    void applyResolution();
    void applyLabelsSettings();
    void applyFontSettings();

    void setBackGroundColor( const QColor& color );
    void setFontColor( const QColor& color );

private slots:
    void onBackGroundColorDoubleClicked();
    void onLabelsColorDoubleClicked();

    void onApplyButtonClicked();
    void onCancelButtonClicked();
    void onOKButtonClicked();
};

#endif // PREFERENCE_H

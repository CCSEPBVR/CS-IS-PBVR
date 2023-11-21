#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QDialog>
#include <QSettings>

#include <kvs/qt/Screen>
#include <kvs/ColorMapBar>
#include <kvs/OrientationAxis>

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
    void setScreen( kvs::qt::Screen* screen ) { m_screen = screen; }
    void setColorMapBar( kvs::ColorMapBar* colorMapBar ) { m_color_map_bar = colorMapBar; }
    void setOrientationAxis( kvs::OrientationAxis* orientationAxis ) { m_orientation_axis = orientationAxis; }

private:
    Ui::Preference *ui;
    kvs::qt::Screen* m_screen;
    kvs::ColorMapBar* m_color_map_bar;
    kvs::OrientationAxis* m_orientation_axis;
    QSettings m_settings;

private:
    bool checkConfigFileExists() { return QFile::exists( "config.ini" ); }
    void loadScreenSettings();
    void loadColorMapBarSettings();
    void loadOrientationAxisSettings();
    void loadResolutionSettings();
    void setDefaultSettings();
    void applySettings( bool isInit );
    void applyScreenSettings();
    void applyColorMapBarSettings();
    void applyOrientationAxisSettings();
    void setSelectedColor(const QColor& color);
    void saveSettings();

private slots:
    void onSelectedColorDoubleClicked();
    void onApplyButtonClicked();
    void onCancelButtonClicked();
    void onOKButtonClicked();
};

#endif // PREFERENCE_H

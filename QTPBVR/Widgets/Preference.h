#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QDialog>
#include <QSettings>
#include <QFile>

class PBVRGUI;

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
    explicit Preference( QWidget *parent = nullptr, PBVRGUI *pbvr_gui = nullptr );
    ~Preference();

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    void initialize();
    void setCurrentTimeStep( int currentTimeStep );

private:
    Ui::Preference *ui;
    PBVRGUI *m_pbvr_gui;
    QSettings m_settings;
    int m_current_time_step;

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

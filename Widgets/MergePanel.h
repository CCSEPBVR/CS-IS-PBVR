#ifndef MERGEPANEL_H
#define MERGEPANEL_H

#include <QDockWidget>

#include <QFileDialog>
//#include "ExtendedKVS/Screen.h"
#include <kvs/qt/Screen>
#include "Widgets/TimeControl.h"
#include "Widgets/Preference.h"
#include "Widgets/Connect.h"
#include "Widgets/DataSummary.h"
#include <kvs/PointObject>
class FilesManager
{
public:
    enum FormatType
    {
        Unknown                       = 0, // Aka Error
        ServerPointObject             = 1, // Server side Point Object
        PointObjectKVSML              = 2, // Point Object(.kvsml)
        PointObjectLAS                = 3, // Point Object(.las)
        NonTexturedPolygonObjectKVSML = 4, // Non Textured Polygon Object(.kvsml)
        NonTexturedPolygonObjectSTL   = 5, // Non Textured Polygon Object(.stl)
        TexturedPolygonObject3DS      = 6, // Textured Polygon Object(.3ds)
        TexturedPolygonObjectFBX      = 7, // Textured Polygon Object(.fbx)
    };

public:
    void setFileInfo( QFileInfo file_info ){ m_file_info = file_info; }
    void setMinTimeStep( int min_time_step ){ m_min_time_step = min_time_step; }
    void setMaxTimeStep( int max_time_step ){ m_max_time_step = max_time_step; }
    void setFormat( FormatType format ){ m_format = format; }
    void setRGBColor( QColor rgb_color ) { m_rgb_color = rgb_color; }
    void setOpacity( double opacity ) { m_opacity = opacity; }    
    void setIds( std::pair<int,int> ids ){ m_ids = ids; }
    void setCurrentDisplayedStep( int current_displayed_step ) { m_current_displayed_step = current_displayed_step; }
    void setObject( kvs::ObjectBase* next_object ) { m_next_object = next_object; }
    void setIsReplacementNeeded( bool is_replacement_needed ){ m_is_replacement_needed = is_replacement_needed; }

    QFileInfo getFileInfo(){ return m_file_info; }
    int getMinTimeStep(){ return m_min_time_step; }
    int getMaxTimeStep(){ return m_max_time_step; }
    FormatType getFormat(){ return m_format; }
    QColor getRGBColor() { return m_rgb_color; }
    double getOpacity() { return m_opacity; }    
    std::pair<int,int> getIds() { return m_ids; }
    int getCurrentDisplayedStep() { return m_current_displayed_step; }
    kvs::ObjectBase* getObject() const { return m_next_object; }
    bool getIsReplacementNeeded() const { return m_is_replacement_needed; }

    QString formatTypeToString( FormatType format )
    {
        switch ( format )
        {
        case Unknown:
            return QStringLiteral( "Unknown" );
        case ServerPointObject:
            return QStringLiteral( "Server" );
        case PointObjectKVSML:
            return QStringLiteral( "KVSML(PointObject)" );
        case PointObjectLAS:
            return QStringLiteral( "las" );
        case NonTexturedPolygonObjectKVSML:
            return QStringLiteral( "KVSML(PolygonObject)" );
        case NonTexturedPolygonObjectSTL:
            return QStringLiteral( "stl" );
        case TexturedPolygonObject3DS:
            return QStringLiteral( "3ds" );
        case TexturedPolygonObjectFBX:
            return QStringLiteral( "fbx" );
        default:
            return QStringLiteral( "Unknown" );
        }
    }
private:
    QFileInfo m_file_info;
    int m_min_time_step;
    int m_max_time_step;
//    Qt::CheckState m_is_display;
//    Qt::CheckState m_is_keep_initial;
//    Qt::CheckState m_is_keep_final;
    FormatType m_format;
    QColor m_rgb_color;
    double m_opacity;    
    std::pair<int, int> m_ids = std::pair<int,int>(-1,-1);
    int m_current_displayed_step;
    kvs::ObjectBase* m_next_object;
    bool m_is_replacement_needed;

//    QColor m_rgb_color;
//    double m_opacity;
};

namespace Ui {
class MergePanel;
}

class MergePanel : public QDockWidget
{
    Q_OBJECT
public:
    enum CheckPattern
    {        
        KeepInitialChecked = 1,
        KeepFinalChecked   = 2,
        BothChecked        = 3,
        NoneChecked        = 4,
    };

public:
    explicit MergePanel(QWidget *parent = nullptr);
    ~MergePanel();
    void setTimeControl( TimeControl* time_control ){ m_time_control = time_control; }
    void setPreference( Preference* preference ){ m_preference = preference; }
    void setConnect( Connect* connect ){ m_connect = connect; }
    void setScreen( kvs::qt::jaea::Screen* screen ){ m_screen = screen; };    
    void setDataSummary( DataSummary* data_summary ){ m_data_summary = data_summary; }
    void setIsParticleGenerationNeeded( bool is_particle_generation_needed ){ m_is_particle_generation_needed = is_particle_generation_needed; }
    void serverObject( QString volumeDataFilePath, int min, int max );
    void mergeObjects();

    bool getIsParticleGenerationNeeded(){ return m_is_particle_generation_needed; }

private:
    Ui::MergePanel *ui;
    QVector<FilesManager*> m_files_manager;
    TimeControl* m_time_control;
    Preference* m_preference;
    Connect* m_connect;
    DataSummary* m_data_summary;
    bool m_is_particle_generation_needed;
    kvs::qt::jaea::Screen* m_screen;    
    int m_current_time_step;
    class WorkerThread;
    bool m_is_worker_thread_running;


    void checkMinMaxTimeStep( FilesManager *newFile );
    void checkFileFormat(  FilesManager *newFile );
    void addRowToFilesTableWidget( FilesManager *newFile );
    void calculateTotalMinMaxTimeStep();
    void removeChecked();//removeRowToFilesTableWidget
    void updatePolygonColorOpacity();
//    void mergeObjects();
    CheckPattern checkPattern( int row );

    template <typename Importer, typename ObjectType, typename RendererType>
    void timeStepCheckAndImport( int row );

    QString updateTimeStepInFileName(QString fileName, int nextTimeStep);
    void totalParticles();

private slots:
    void onFilesTWidgetCellDoubleClicked( int row, int column );
    void onBrowserButtonClicked();
    void onAddButtonClicked();
    void onCenteringButtonClicked();
    void onApplyButtonClicked();
    void onWorkerThreadFinished();
};

#include <QCheckBox>
#include <kvs/PointImporter>
#include <kvs/ParticleBasedRenderer>
#include <kvs/PolygonImporter>
#include <kvs/StochasticPolygonRenderer>
class MergePanel::WorkerThread : public QThread
{
    Q_OBJECT

public:
    enum CheckPattern
    {
        KeepInitialChecked = 1,
        KeepFinalChecked   = 2,
        BothChecked        = 3,
        NoneChecked        = 4,
    };

public:
    explicit WorkerThread(MergePanel* gui);

protected:
    void run() override;

signals:
    void workFinished();

private:
    MergePanel* m_merge;

private:
    CheckPattern checkPattern( int row );

    template <typename Importer, typename ObjectType, typename RendererType>
    void timeStepCheckAndImport( int row );

    QString updateTimeStepInFileName(QString fileName, int nextTimeStep);
};
#endif // MERGEPANEL_H

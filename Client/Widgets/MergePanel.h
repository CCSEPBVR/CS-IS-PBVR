#ifndef MERGEPANEL_H
#define MERGEPANEL_H

#include <QDockWidget>

#include "ExtendedKVS/Screen.h"
#include "Widgets/ToolBars.h"
#include "Widgets/Connect.h"
#include "Widgets/ShadingController.h"
#include <QFileInfo>
#include <kvs/ObjectBase>
class FilesManager
{
public:
    enum Format
    {
        Unknown                       = 0,  // Aka Error
        ServerPointObjectCS           = 1,  // Server side Point Object
        ServerPointObjectIS           = 2,  // Server side Point Object
        PointObjectKVSML              = 3,  // Point Object(.kvsml)
        PointObjectLAS                = 4,  // Point Object(.las)
        PointObjectPTS                = 5,  // Point Object(.pts)
        NonTexturedPolygonObjectKVSML = 6,  // Non Textured Polygon Object(.kvsml)
        NonTexturedPolygonObjectSTL   = 7,  // Non Textured Polygon Object(.stl)
        TexturedPolygonObject3DS      = 8,  // Textured Polygon Object(.3ds)
        TexturedPolygonObjectFBX      = 9,  // Textured Polygon Object(.fbx)
        LineObjectKVSML               = 10, // Line Object(.kvsml)
    };

    QString formatToString( Format format )
    {
        switch ( format )
        {
        case Unknown:
            return QStringLiteral( "Unknown" );
        case ServerPointObjectCS:
            return QStringLiteral( "Server(CS)" );
        case ServerPointObjectIS:
            return QStringLiteral( "Server(IS)" );
        case PointObjectKVSML:
            return QStringLiteral( "KVSML(PointObject)" );
        case PointObjectLAS:
            return QStringLiteral( "las" );
        case PointObjectPTS:
            return QStringLiteral( "pts" );
        case NonTexturedPolygonObjectKVSML:
            return QStringLiteral( "KVSML(PolygonObject)" );
        case NonTexturedPolygonObjectSTL:
            return QStringLiteral( "stl" );
        case TexturedPolygonObject3DS:
            return QStringLiteral( "3ds" );
        case TexturedPolygonObjectFBX:
            return QStringLiteral( "fbx" );
        case LineObjectKVSML:
            return QStringLiteral( "KVSML(LineObject)" );
        default:
            return QStringLiteral( "Unknown" );
        }
    }

public:
    void setFileInfo(const QFileInfo& file_info) { m_file_info = file_info; }
    void setDisplay( const bool& is_display ) { m_is_display = is_display; }
    void setKeepInital( const bool& is_keep_initial ) { m_is_keep_initial = is_keep_initial; }
    void setKeepFinal( const bool& is_keep_final ) { m_is_keep_final = is_keep_final; }
    void setFormat(const Format& format) { m_format = format; }
    void setMinTimeStep(const int& min_time_step ) { m_min_time_step = min_time_step; }
    void setMaxTimeStep(const int& max_time_step ) { m_max_time_step = max_time_step; }
    void setColor( const QColor& color ) { m_color = color; }
    void setOpacity( const double& opacity ) { m_opacity = opacity; }
    void setChangePolygonTransferFunction( const bool& is_change_polygon_transfer_function ) { m_is_change_polygon_transfer_function = is_change_polygon_transfer_function; }
    void setIDs( const std::pair<int,int>& ids ) { m_ids = ids; }
    void setAlreadyImportedTimeStep( const int& current_displayed_time_step ) { m_already_imported_time_step = current_displayed_time_step; }
    void setObject( kvs::ObjectBase* object ) { m_object = object; }

    const QFileInfo& getFileInfo() const { return m_file_info; }
    const bool& getDisplay() const { return m_is_display; }
    const bool& getKeepInitial() const { return m_is_keep_initial; }
    const bool& getKeepFinal() const { return m_is_keep_final; }
    const Format& getFormat() const { return m_format; }
    const int& getMinTimeStep() const { return m_min_time_step; }
    const int& getMaxTimeStep() const { return m_max_time_step; }
    const QColor& getColor() const { return m_color; }
    const double& getOpacity() const { return m_opacity; }
    const bool& getChangePolygonTransferFunction() const { return m_is_change_polygon_transfer_function; }
    const std::pair<int,int>& getIDs() const { return m_ids; }
    const int& getAlreadyImportedTimeStep() const { return m_already_imported_time_step; }
    kvs::ObjectBase* getObject() const { return m_object; }

private:    
    QFileInfo m_file_info;
    bool m_is_display;
    bool m_is_keep_initial;
    bool m_is_keep_final;
    Format m_format;
    int m_min_time_step;
    int m_max_time_step;
    QColor m_color;
    double m_opacity;
    bool m_is_change_polygon_transfer_function;
    std::pair<int,int> m_ids;
    int m_already_imported_time_step;
    kvs::ObjectBase* m_object;
};

class PBVRGUI;

namespace Ui {
class MergePanel;
}

class MergePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit MergePanel(QWidget *parent = nullptr,
                         PBVRGUI *pbvr_gui = nullptr,
                         Preference* preference = nullptr,
                         TimeControllerB* time_controller_b = nullptr,
                         TotalParticles* total_particles = nullptr,
                         Connect* connectUI = nullptr,
                         ShadingController* shading_controller = nullptr );
    ~MergePanel();

    void mergeObjects( int currentTimeStep, int requestTimeStep );
    void serverObjectCS( QString volumeDataFilePath, int min, int max );
    void serverObjectIS( QString volumeDataFilePath, int min, int max );
    void updateObjectTimeStepIS( int min, int max );

    void setIsParticleGenerationNeeded( const bool& is_particle_generation_needed ){ m_is_particle_generation_needed = is_particle_generation_needed; }
    void setIsExport( const bool& is_export ){ m_is_export = is_export; }
    void setExportFilePath( const QString& export_file_path ){ m_export_file_path = export_file_path; }

    const bool& getIsParticleGenerationNeeded() const { return m_is_particle_generation_needed; }
    const bool& getIsExport() const { return m_is_export; }
    const QString& getExportFilePath() const { return m_export_file_path; }

private:
    Ui::MergePanel *ui;
    PBVRGUI *m_pbvr_gui;
    Preference* m_preference;
    TimeControllerB* m_time_controller_b;
    TotalParticles* m_total_particles;
    Connect* m_connect;
    ShadingController* m_shading_controller;
    QVector<FilesManager*> m_files_manager;
    class WorkerThread;
    bool m_is_worker_thread_running;
    bool m_is_particle_generation_needed;
    bool m_is_export;
    QString m_export_file_path;
    bool IS_OBJ;
    bool IS_OBJ_DONE_INIT;

private:
    void registerFiles( const QString& filePath );
    bool checkFormat( FilesManager* newFile );
    bool checkMinMaxTimeStep( FilesManager* newFile );
    void addFilesTable( FilesManager* newFile );
    void calculateTotalMinMaxTimeStep();    
    void updateCheckState();
    void updatePolygonColorOpacity();
    void totalParticles();

private slots:
    void onBrowser();
    void onExport();
    void onCentering();
    void onApply();
    void onFilesTWidgetCellDoubleClicked( int row, int column );
    void onWorkerThreadFinished();
};

#include <QThread>
class MergePanel::WorkerThread : public QThread
{
    Q_OBJECT

public:

public:
    explicit WorkerThread( MergePanel* gui );
    void setCurrentTimeStep( int current_time_step ) { m_current_time_step = current_time_step; }
    void setRequestTimeStep( int request_time_step ) { m_request_time_step = request_time_step; }

protected:
    void run() override;

signals:
    void workFinished();

private:
    MergePanel* m_merge;
    int m_current_time_step;
    int m_request_time_step;

private:
    template <typename Importer, typename ObjectType, typename RendererType>
    void timeStepCheckAndImport( int row );

    std::string updateTimeStepInFileName( QString fileName, int nextTimeStep );
};
#endif // MERGEPANEL_H

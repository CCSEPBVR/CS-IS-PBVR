// .h
#ifndef MERGE_H
#define MERGE_H

#include <QDir>
#include <QDockWidget>
#include <QTableWidgetItem>
#include <kvs/RGBColor>
#include <ExtendedKVS/Screen.h>
#include <Widgets/TimeControl.h>
#include "Widgets/Preference.h"
#include "Widgets/Connect.h"
#include "Widgets/DataSummary.h"

class FilesManager
{
public:
    enum FileType
    {
        Unknown            = 0, // Aka Error
        ServerPointObject  = 1, // Server size Point Object
        PointObject        = 2, // Point Object(.kvsml)
        NonTexturedPolygon = 3, // Non Textured Polygon Object(.kvsml), .stl
        TexturedPolygon    = 4, // Textured Polygon Object .3ds, .fbx
        LASPointObject     = 5, // .las
    };

    void setFileInfo( QFileInfo fileInfo ) { m_file_info = fileInfo; }
    void setFileName( QString file_name ) { m_file_name = file_name; }
    void setMinTimeStep( int min_time_step ) { m_min_time_step = min_time_step; }
    void setMaxTimeStep( int max_time_step ) { m_max_time_step = max_time_step; }

    void setVisible( Qt::CheckState is_visible ) { m_is_visible = is_visible; }
    void setKeepInitial( Qt::CheckState is_keep_initial ) { m_is_keep_initial = is_keep_initial; }
    void setKeepFinal( Qt::CheckState is_keep_final ) { m_is_keep_final = is_keep_final; }
    void setFileSuffix( QString file_suffix ) { m_file_suffix = file_suffix; }
    void setFileFormat( FileType file_format ) { m_file_format = file_format; }
    void setRGBColor( QColor rgb_color ) { m_rgb_color = rgb_color; }
    void setOpacity( double opacity ) { m_opacity = opacity; }
    void setIds( std::pair<int, int> ids ) { m_ids = ids; }
    void setIsModified( bool is_modified ) { m_is_modified = is_modified; }

    QFileInfo getFileInfo() { return m_file_info; }
    QString getFileName() const {return m_file_name; }
    int getMinTimeStep() const { return m_min_time_step; }
    int getMaxTimeStep() const { return m_max_time_step; }

    Qt::CheckState getVisible() { return m_is_visible; }
    Qt::CheckState getKeepInitial() { return m_is_keep_initial; }
    Qt::CheckState getKeepFinal() { return m_is_keep_final; }
    QString getFileSuffix() { return m_file_suffix; }
    FileType getFileFormat() { return m_file_format; }
    QColor getRGBColor() { return m_rgb_color; }
    double getOpacity() { return m_opacity; }
    std::pair<int,int> getIds() { return m_ids; }
    bool getIsModified() { return m_is_modified; }

private:
    QFileInfo m_file_info; //この値はui->filesTWidgetでは使いません。
    QString m_file_name;   //この値はui->filesTWidgetでは使いません。
    int m_min_time_step;   //この値はui->filesTWidgetでは使いません。
    int m_max_time_step;   //この値はui->filesTWidgetでは使いません。

    Qt::CheckState m_is_visible;
    Qt::CheckState m_is_keep_initial;
    Qt::CheckState m_is_keep_final;
    QString m_file_suffix;
    FileType m_file_format;
    QColor m_rgb_color;
    double m_opacity;
    std::pair<int, int> m_ids = std::pair<int,int>(0,0);
    bool m_is_modified;
};

namespace Ui {
class Merge;
}

class Merge : public QDockWidget
{
    Q_OBJECT
public:
    enum pattern
    {
        KeepInitialChecked = 0,
        KeepFinalChecked   = 1,
        BothChecked        = 2,
        NoneChecked        = 3,
    };

public:
    explicit Merge(QWidget *parent = nullptr);
    ~Merge();
    void setScreen( kvs::qt::jaea::Screen* screen ){ m_screen = screen; }
    void setTimeControl( TimeControl* time_control ){ m_time_control = time_control; }
    void setPreference( Preference* preference ){ m_preference = preference; }
    void setConnect( Connect* connect ){ m_connect = connect; }
    void setDataSummary( DataSummary* data_summary ){ m_data_summary = data_summary; }
    void serverObject( QString volumeDataFilePath, int min, int max );

private:
    Ui::Merge *ui;
    QStringList headerLabels;
    QVector<FilesManager*> m_files_manager;
    kvs::qt::jaea::Screen* m_screen;
    TimeControl* m_time_control;
    Preference* m_preference;
    Connect* m_connect;
    DataSummary* m_data_summary;
    int currentTimeStep;

    void onBrowserButtonClicked();
    void onAddButtonClicked();
    void checkMinMaxTimeStep( QFileInfo *fileInfo, QDir *directory, FilesManager *filesManager );
    void checkFileFormat( QFileInfo *fileInfo, FilesManager *filesManager );
    void onApplyButtonClicked();
    void removeChecker();
    void registerFile( FilesManager* filesManager );
    void updateFiles();
    void calculateMinMaxTimeStep();
    void mergeObjects();

    template <typename Importer, typename ObjectType>
    ObjectType* selectPattern( FilesManager* filesManager );
    template <typename Importer, typename ObjectType>
    ObjectType* timeStepCheckAndImport( FilesManager* filesManager, pattern pattern );

    kvs::PointObject* selectPattern( FilesManager* filesManager );
    kvs::PointObject* timeStepCheckAndImport( FilesManager* filesManager, pattern pattern );

    QString updateTimeStepInFileName( QString fileName,int nextTimeStep );
    void removeObject( FilesManager* filesManager );
    void updateObject( FilesManager* filesManager, kvs::ObjectBase* object );
    void onFilesTWidgetCellDoubleClicked( int row, int column );
    void showFilesManager();
    void totalParticles();
};

#endif // MERGE_H

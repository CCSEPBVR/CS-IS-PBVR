#ifndef MERGE_H
#define MERGE_H

#include <QDir>
#include <QDockWidget>
#include <QTableWidgetItem>
#include <kvs/RGBColor>
namespace Ui {
class Merge;
}
class FilesManager
{
public:
    enum FileType
    {
        Unknown            = 0, // Aka Error
        ServerPointObject  = 1, // Server size Point Object
        PointObject        = 2, // Point Object(.kvsml), .las
        NonTexturedPolygon = 3, // Non Textured Polygon Object(.kvsml), .stl
        TexturedPolygon    = 4, // Textured Polygon Object .3ds, .fbx
    };

    void setFileInfo( QFileInfo fileInfo )      { m_file_info       = fileInfo;        }
    void setMinTimeStep( int min_time_step )    { m_min_time_step   = min_time_step;   }
    void setMaxTimeStep( int max_time_step )    { m_max_time_step   = max_time_step;   }
    void setVisible( Qt::CheckState is_visible )          { m_is_visible      = is_visible;      }
    void setKeepInitial( Qt::CheckState is_keep_initial ) { m_is_keep_initial = is_keep_initial; }
    void setKeepFinal( Qt::CheckState is_keep_final )     { m_is_keep_final   = is_keep_final;   }
    void setFileName( QString file_name )       { m_file_name       = file_name;       }
    void setFileSuffix( QString file_suffix )       { m_file_suffix       = file_suffix;       }
    void setFileFormat( FileType file_format )   { m_file_format     = file_format;     }
    void setRGBColor( QColor rgb_color ) { m_rgb_color       = rgb_color;       }
    void setOpacity( double opacity )           { m_opacity         = opacity;         }

    QFileInfo getFileInfo()     { return m_file_info;       }
    int getMinTimeStep()        { return m_min_time_step;   }
    int getMaxTimeStep()        { return m_max_time_step;   }
    Qt::CheckState getVisible()           { return m_is_visible;      }
    Qt::CheckState getKeepInitial()       { return m_is_keep_initial; }
    Qt::CheckState getKeepFinal()         { return m_is_keep_final;   }
    QString getFileName()       { return m_file_name;       }
    QString getFileSuffix()       { return m_file_suffix;       }
    FileType getFileFormat()     { return m_file_format;     }
    QColor getRGBColor() { return m_rgb_color;       }
    double getOpacity()         { return m_opacity;         }

private:
    QFileInfo m_file_info;
    int m_min_time_step;
    int m_max_time_step;
    Qt::CheckState m_is_visible; //c
    Qt::CheckState m_is_keep_initial; //c
    Qt::CheckState m_is_keep_final; //c
    QString m_file_name;
    QString m_file_suffix;
    FileType m_file_format;
    QColor m_rgb_color; //c
    double m_opacity; //c
};

class Merge : public QDockWidget
{
    Q_OBJECT
//    enum FileType
//    {
//        Unknown            = 0, // Aka Error
//        ServerPointObject  = 1, // Server size Point Object
//        PointObject        = 2, // Point Object(.kvsml), .las
//        NonTexturedPolygon = 3, // Non Textured Polygon Object(.kvsml), .stl
//        TexturedPolygon    = 4, // Textured Polygon Object .3ds, .fbx
//    };


public:
    explicit Merge(QWidget *parent = nullptr);
    ~Merge();

private:
    Ui::Merge *ui;
    QVector<FilesManager*> m_files_manager;
    QStringList item_name;

    void updateFilesManagerFromTable();
    void onBrowserButtonClicked();
    void onAddButtonClicked();
    void onApplyButtonClicked();
    void registerFile( FilesManager* filesManager );
    void removeSelectedRows();
    void checkMinMaxTimeStep( QFileInfo* fileInfo, QDir* directory, FilesManager* filesManager );
    void checkFileFormat( QFileInfo* fileInfo, FilesManager* filesManager );
    void tableItemClicked(int row, int column);


//    void checkMinMaxTimeStep( QFileInfo* fileInfo, QDir* directory );
//    void checkFileFormat( QFileInfo* fileInfo  );


};



#endif // MERGE_H

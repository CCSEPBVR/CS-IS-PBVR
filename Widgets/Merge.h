// .h
#ifndef MERGE_H
#define MERGE_H

#include <QDir>
#include <QDockWidget>
#include <QTableWidgetItem>
#include <kvs/RGBColor>

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

    void setFileInfo( QFileInfo fileInfo )      { m_file_info       = fileInfo;        }
    void setFileName(QString file_name){ m_file_name = file_name; }
    void setMinTimeStep(int min_time_step) { m_min_time_step = min_time_step; }
    void setMaxTimeStep(int max_time_step) { m_max_time_step = max_time_step; }

    void setVisible( Qt::CheckState is_visible )          { m_is_visible      = is_visible;      }
    void setKeepInitial( Qt::CheckState is_keep_initial ) { m_is_keep_initial = is_keep_initial; }
    void setKeepFinal( Qt::CheckState is_keep_final )     { m_is_keep_final   = is_keep_final;   }
    void setFileSuffix( QString file_suffix )       { m_file_suffix       = file_suffix;       }
    void setFileFormat( FileType file_format )   { m_file_format     = file_format;     }
    void setRGBColor( QColor rgb_color ) { m_rgb_color       = rgb_color;       }
    void setOpacity( double opacity )           { m_opacity         = opacity;         }

    QFileInfo getFileInfo()     { return m_file_info;       }
    QString getFileName() const {return m_file_name; }
    int getMinTimeStep() const { return m_min_time_step; }
    int getMaxTimeStep() const { return m_max_time_step; }

    Qt::CheckState getVisible()           { return m_is_visible;      }
    Qt::CheckState getKeepInitial()       { return m_is_keep_initial; }
    Qt::CheckState getKeepFinal()         { return m_is_keep_final;   }
    QString getFileSuffix()       { return m_file_suffix;       }
    FileType getFileFormat()     { return m_file_format;     }
    QColor getRGBColor() { return m_rgb_color;       }
    double getOpacity()         { return m_opacity;         }

private:
    QFileInfo m_file_info; //この値はui->filesTWidgetでは使いません。
    QString m_file_name; //この値はui->filesTWidgetでは使いません。
    int m_min_time_step; //この値はui->filesTWidgetでは使いません。
    int m_max_time_step; //この値はui->filesTWidgetでは使いません。

    Qt::CheckState m_is_visible;
    Qt::CheckState m_is_keep_initial;
    Qt::CheckState m_is_keep_final;
    QString m_file_suffix;
    FileType m_file_format;
    QColor m_rgb_color;
    double m_opacity;
};

namespace Ui {
class Merge;
}

class Merge : public QDockWidget
{
    Q_OBJECT

public:
    explicit Merge(QWidget *parent = nullptr);
    ~Merge();

private:
    Ui::Merge *ui;
    QStringList headerLabels;
    QVector<FilesManager*> m_files_manager;
    void onBrowserButtonClicked();
    void onAddButtonClicked();
    void onApplyButtonClicked();
    void removeFiles();
    void updateData();
    void printFilesManagerContents();
    void registerFile( FilesManager* filesManager );
    void checkMinMaxTimeStep(QFileInfo *fileInfo, QDir *directory,FilesManager *filesManager);
    void checkFileFormat(QFileInfo *fileInfo, FilesManager *filesManager);
};

#endif // MERGE_H

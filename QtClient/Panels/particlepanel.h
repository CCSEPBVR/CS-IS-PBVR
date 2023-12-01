#ifndef PARTICLEPANEL_H
#define PARTICLEPANEL_H

#include <QDockWidget>
#include "QGlue/extCommand.h"
#include <QCheckBox>
#include <QGroupBox>

#include "coloreditdialog.h"
namespace Ui {
class ParticlePanel;
}

class ParticlePanel : public QDockWidget
{
//    Q_OBJECT

public:
    explicit ParticlePanel(QWidget *parent = 0);
    ~ParticlePanel();

    Qt::DockWidgetArea default_area=Qt::NoDockWidgetArea;
    void updateUI2Cmd();
    void setParam( kvs::visclient::ParticleMergeParameter* param );
    void particleExportButtonActive();
    void particleResetChangedFlag();
    bool checkChangeFlag();
    void logPlay(std::string funcName, std::string str[]);
    void recToggle(){ m_isRec ? m_isRec = false : m_isRec = true; }

private:

    void on_particle_changeFlag(int flag){
        (void )flag;
        m_changed=true;
    }
    void onDeleteButtonClick();
    void onExportButtonClick();
    void onItemToAddIndexChange(int index);
    void onBrowseButtonClick();
    void onSetButtonClick();
    void onCloseButtonClick();

    void onDivColorChanged();
    void changeOpacityValue();

private:

    int m_total_item;
    std::string m_item_strings[11];
    std::string m_file_path_strings[11];
    QList<QCheckBox*> m_keep_initial_value;
    QList<QCheckBox*> m_keep_final_value;
    QList<QCheckBox*> m_particle_value;
    bool m_file_path_changed[11];
    bool m_do_export;
    bool m_changed;
    bool m_polygon_changed[5];
    bool m_color_opacity_changed[5] = {false,false,false,false,false};
//    kvs::UInt8 m_polygon_opacity[5]= {128,128,128,128,128};
    double m_polygon_opacity[5]= {0.5,0.5,0.5,0.5,0.5};
    kvs::RGBColor m_polygon_color[5];
    int m_start_export_time;

//    QFileDialog* m_file_dialog_file;
//    QFileDialog* m_file_dialog_directory;

    QList<QCheckBox *> initializeDisplayValues();
    QList<QCheckBox *> initializeKeepInitial();
    QList<QCheckBox *> initializeKeepFinal();

    QString m_text_output_file_path;
    kvs::visclient::ParticleMergeParameter* m_param;
    Ui::ParticlePanel *ui;

    ColorEditDialog* m_polygon_color_edit;
    bool m_isRec = false;
};

#endif // PARTICLEPANEL_H

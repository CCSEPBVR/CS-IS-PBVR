#ifndef PBVR__KVS__VISCLIENT__PBVR_PARTICLE_MERGER_UI_H_INCLUDE
#define PBVR__KVS__VISCLIENT__PBVR_PARTICLE_MERGER_UI_H_INCLUDE

#include <string>
#include <vector>
#include <kvs/PointObject>
#include "ParticleMergeParameter.h"
#include "FileDialog.h"

#include "GL/glui.h"


namespace kvs
{
namespace visclient
{
class ParticleMergerParam;

class ParticleMergerUI
{

//protected:
private:

    kvs::visclient::ParticleMergeParameter* m_param;
    int m_total_item;
    std::string m_item_strings[11];
    std::string m_file_path_strings[11];
    int m_keep_initial_value[11];
    int m_keep_final_value[11];
    int m_particle_value[11];
    bool m_file_path_changed[11];
    bool m_do_export;
    int m_start_export_time;
    GLUI* m_glui_add_delete;
    GLUI* m_glui_particele_file;
    GLUI* m_glui_particele_directory;

    GLUI_Panel* m_main_particle;
    GLUI_Panel* m_panel_display;
    GLUI_Panel* m_panel_keep_initial;
    GLUI_Panel* m_panel_keep_final;
    GLUI_Panel* m_panel_del;
    GLUI_Listbox* m_list_delete;
    GLUI_Button* m_buttom_delete;

    GLUI_Checkbox* m_checkbox_particle[11];
    GLUI_Checkbox* m_checkbox_keep_initial[11];
    GLUI_Checkbox* m_checkbox_keep_final[11];

    GLUI_Panel* m_panel_add;
    GLUI_Listbox* m_list_add;
    GLUI_Panel* m_panel_file_browser1;
    GLUI_EditText* m_text_file_path;
    GLUI_EditText* m_text_particle_name;
    GLUI_Button* m_buttom_directory_file_dialog;
    GLUI_Panel* m_panel_output;
    GLUI_Panel* m_panel_file_browser2;
    GLUI_EditText* m_text_output_file_path;
    GLUI_Button* m_button_file_path_file_dialog;
    GLUI_Button* m_buttom_export;

    FileDialog* m_file_dialog_file;
    FileDialog* m_file_dialog_directory;

    // 粒子の表示ボタンのon/offが変化した場合 true になる（正規化用）
    bool m_changed;

    bool m_show_particle_merger_ui;

public:
    GLUI* m_glui_particle_main;

public:

    ParticleMergerUI();
    virtual ~ParticleMergerUI();

    void setParam( kvs::visclient::ParticleMergeParameter* param );
    void setArgParam();
    void createPanel( const int client_server_mode = 0, kvs::visclient::ParticleMergeParameter* param = NULL );

    void particleAddFunction();
    void particleDeleteFunction();
    void particleFileActiveFunction();
    void particleFileCloseFunction();
    void paricleBrowseDirectoryActiveFunction();
    void paricleBrowseFileActiveFunction();
    void paricleExportDoFunction();
    void paricleExportBottomActive();
    void paricleChangedFlag();
    void paricleResetChangedFlag();
    bool checkChengeFlag();
    bool getShowParticleMergerUI();
    void changeShowParticleMergerUI();
    void pariclePanelClose();
};

} // namespace visclient
} // namespace kvs

#endif // PBVR__KVS__VISCLIENT__PBVR_PARTICLE_MERGER_UI_H_INCLUDE

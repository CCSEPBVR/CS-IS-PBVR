#include <algorithm>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>
#include "ParticleMerger.h"
#include "ParticleMergerUI.h"
#include "KVSMLObjectPointWriter.h"

extern void CallBackParticlePanelShow( const int i );

namespace
{
kvs::visclient::ParticleMergerUI* pparticle_mui;

void CallBackParicleAdd( const int id )
{
    pparticle_mui->particleAddFunction();
}
void CallBackParicleDelete( const int id )
{
    pparticle_mui->particleDeleteFunction();
}
void CallBackParicleFileActive( const int id )
{
    pparticle_mui->particleFileActiveFunction();
}
void CallBackParicleFileClose( const int id )
{
    pparticle_mui->particleFileCloseFunction();
}
void CallBackParicleBrowseDirectory( const int id )
{
    pparticle_mui->paricleBrowseDirectoryActiveFunction();
}
void CallBackParticleBrowseFile( const int id )
{
    pparticle_mui->paricleBrowseFileActiveFunction();
}
void CallBackParticleExport( const int id )
{
    pparticle_mui->paricleExportDoFunction();
}
void CallBackParticleDisplayValue( const int id )
{
    pparticle_mui->paricleChangedFlag();
}
void CallBackParticlePanelClose( const int id )
{
    CallBackParticlePanelShow( 1 );
}
} // end of namespace

namespace kvs
{
namespace visclient
{

ParticleMergerUI::ParticleMergerUI()
{
    static int tmpm_keep_initial_value[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static int tmpm_keep_final_value[11]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static int tmpm_particle_value[11] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for ( int i = 0; i < 11; i++ )
    {
        m_keep_initial_value[i] = tmpm_keep_initial_value[i];
        m_keep_final_value[i] = tmpm_keep_final_value[i];
        m_particle_value[i] = tmpm_particle_value[i];
        if ( m_file_path_strings[i].empty() != false )
        {
            m_file_path_strings[i].clear();
        }
        m_file_path_changed[i] = false;
    }
    m_file_path_strings[0] = "server";
    m_total_item = 0;
    m_do_export = false;
    m_start_export_time = -1;

    m_file_dialog_file = NULL;
    m_file_dialog_directory = NULL;

    m_changed = false;
}

ParticleMergerUI::~ParticleMergerUI()
{
}

void ParticleMergerUI::createPanel( const int client_server_mode, kvs::visclient::ParticleMergeParameter* param ) //client_server_mode=1 はサーバ通信モード
{
    pparticle_mui = this;

    m_glui_particle_main = GLUI_Master.create_glui( "Particle panel", 0, 800, 100 );
    m_main_particle = m_glui_particle_main->add_panel( "", GLUI_PANEL_NONE );
    // クライアントが読み込む粒子一覧の表示
    m_panel_display = m_glui_particle_main->add_panel_to_panel( m_main_particle, "Display Particle", GLUI_PANEL_EMBOSSED );
    if ( client_server_mode == 0 )
    {
        *m_particle_value = 0; // スタンドアローンモードならserver欄のチェックは外す
    }
    m_checkbox_particle[0] = m_glui_particle_main->add_checkbox_to_panel( m_panel_display, "Server", m_particle_value, 0, CallBackParticleDisplayValue );
    // スタンドアローンモードならserver欄は不活性
    if ( client_server_mode == 0 )
    {
        m_checkbox_particle[0]->disable();
    }
    char buf[64];
    for ( int i = 1; i < 11; i++ )
    {
        sprintf( buf, "(Particle%d)", i ); // Particle文字列で初期化
        m_checkbox_particle[i] = m_glui_particle_main->add_checkbox_to_panel( m_panel_display, buf, m_particle_value + i, 1, CallBackParticleDisplayValue );
        m_item_strings[i].append( buf );
    }
    for ( int i = 1; i < 11; i++ )
    {
        m_checkbox_particle[i]->disable(); // 不活性
    }

    // 前方補間
    m_glui_particle_main->add_column_to_panel( m_main_particle, false );
    GLUI_Panel* m_panel_keep_initial = m_glui_particle_main->add_panel_to_panel( m_main_particle, "Keep Initial Step", GLUI_PANEL_EMBOSSED );
    m_checkbox_keep_initial[0] = m_glui_particle_main->add_checkbox_to_panel( m_panel_keep_initial, "Server", m_keep_initial_value );
    // スタンドアローンモードならserver欄は不活性
    if ( client_server_mode == 0 )
    {
        m_checkbox_keep_initial[0]->disable();
    }
    for ( int i = 1; i < 11; i++ )
    {
        sprintf( buf, "(Particle%d)", i );
        m_checkbox_keep_initial[i] = m_glui_particle_main->add_checkbox_to_panel( m_panel_keep_initial, buf, m_keep_initial_value + i );
    }
    for ( int i = 1; i < 11; i++ )
    {
        m_checkbox_keep_initial[i]->disable();
    }

    // 後方補間
    m_glui_particle_main->add_column_to_panel( m_main_particle, false );
    GLUI_Panel* m_panel_keep_final = m_glui_particle_main->add_panel_to_panel( m_main_particle, "Keep Final Step", GLUI_PANEL_EMBOSSED );
    m_checkbox_keep_final[0] = m_glui_particle_main->add_checkbox_to_panel( m_panel_keep_final, "Server", m_keep_final_value );
    for ( int i = 1; i < 11; i++ )
    {
        sprintf( buf, "(Particle%d)", i );
        m_checkbox_keep_final[i] = m_glui_particle_main->add_checkbox_to_panel( m_panel_keep_final, buf, m_keep_final_value + i );
    }
    // スタンドアローンモードならserver欄は不活性
    if ( client_server_mode == 0 )
    {
        m_checkbox_keep_final[0]->disable();
    }
    for ( int i = 1; i < 11; i++ )
    {
        m_checkbox_keep_final[i]->disable();
    }

    m_glui_particle_main->add_button( "Particle File (Add/Export)", -1, CallBackParicleFileActive );
    m_panel_del = m_glui_particle_main->add_panel( "Delete Particle" );
    m_list_delete = m_glui_particle_main->add_listbox_to_panel( m_panel_del, "Delete Item" );
    int total_cnt = 0;
    for ( int i = 1; i < 11; i++ )
    {
        if ( m_checkbox_particle[i]->enabled )
        {
            m_list_delete->add_item( i, m_checkbox_particle[i]->name.c_str() );
            total_cnt++;
        }
    }
    m_buttom_delete = m_glui_particle_main->add_button_to_panel( m_panel_del, "Delete", -1, CallBackParicleDelete );
    if ( total_cnt == 0 )
    {
        m_list_delete->disable();
        m_buttom_delete->disable();
    }
    m_glui_add_delete = GLUI_Master.create_glui( "Particle File", 0, 900, 200 );
    m_panel_add = m_glui_add_delete->add_panel( "Add Particle" );
    m_list_add = m_glui_add_delete->add_listbox_to_panel( m_panel_add, "Add Item" );
    m_list_add->set_int_val( 1 );
    m_panel_file_browser1 = m_glui_add_delete->add_panel_to_panel( m_panel_add, NULL, GLUI_PANEL_NONE );
    m_text_file_path = m_glui_add_delete->add_edittext_to_panel( m_panel_file_browser1, "File path:", GLUI_EDITTEXT_TEXT, NULL );
    m_text_file_path->set_w( 300 );
    m_glui_add_delete->add_column_to_panel( m_panel_file_browser1, false );
    m_buttom_directory_file_dialog = m_glui_add_delete->add_button_to_panel( m_panel_file_browser1, "Browse...", -1, CallBackParicleBrowseDirectory );
    m_text_particle_name = m_glui_add_delete->add_edittext_to_panel( m_panel_add, "Particle name(Optional):", GLUI_EDITTEXT_TEXT, NULL );
    m_text_particle_name->set_w( 250 );
    m_glui_add_delete->add_button_to_panel( m_panel_add, "Add", -1, CallBackParicleAdd );
    // ------
    m_panel_output = m_glui_add_delete->add_panel( "Export Particle" );
    m_panel_file_browser2 = m_glui_add_delete->add_panel_to_panel( m_panel_output, NULL, GLUI_PANEL_NONE );
    m_text_output_file_path = m_glui_add_delete->add_edittext_to_panel( m_panel_file_browser2, "File path:", 1, NULL );
    m_text_output_file_path->set_w( 300 );
    m_glui_add_delete->add_column_to_panel( m_panel_file_browser2, false );
    m_button_file_path_file_dialog = m_glui_add_delete->add_button_to_panel( m_panel_file_browser2, "Browse...", -1, CallBackParticleBrowseFile );
    m_buttom_export = m_glui_add_delete->add_button_to_panel( m_panel_output, "Export", -1, CallBackParticleExport );

    m_glui_add_delete->add_button( "Close", -1, CallBackParicleFileClose );
    m_glui_add_delete->hide();

    // pinオプションの考慮
    for ( int i = 0; i < 10; i++ )
    {
        if ( param->m_particles[i + 1].m_file_path.empty() != true )
        {

            // ディレクトリを登録
            m_total_item++;
            m_file_path_strings[i + 1].clear();
            m_file_path_strings[i + 1].append( param->m_particles[i + 1].m_file_path );

            // ディレクトリ名でリストに追加
            std::string buffdir( param->m_particles[i + 1].m_file_path );
            if ( buffdir[buffdir.size() - 1] == kvs::Directory::Separator()[0] )
            {
                // パスの指定の最後が/ならば削除しておく
                buffdir.erase( buffdir.size() - 1, 1 );
            }
            if ( buffdir.find_last_of( kvs::Directory::Separator() ) != std::string::npos )
            {
                // 上の階層のパスは削除
                buffdir.erase( 0, buffdir.find_last_of( kvs::Directory::Separator() ) + 1 );
            }
            m_checkbox_particle[i + 1]->set_name( buffdir.c_str() );
            m_checkbox_keep_initial[i + 1]->set_name( buffdir.c_str() );
            m_checkbox_keep_final[i + 1]->set_name( buffdir.c_str() );
            // 操作可能にする
            m_checkbox_particle[i + 1]->enable();
            m_checkbox_keep_initial[i + 1]->enable();
            m_checkbox_keep_final[i + 1]->enable();
            // 削除リストに追加
            m_list_delete->add_item( i + 1, buffdir.c_str() );

            m_item_strings[i + 1].erase( 0 );
            m_item_strings[i + 1].append( buffdir.c_str() );

            //チェックを有効化.
            m_checkbox_particle[i + 1]->set_int_val( 1 );
            m_particle_value[i + 1] = 1;
            m_changed = 1;
            m_list_add->add_item( i + 1, buffdir.c_str() );
        }
        else
        {
            m_list_add->add_item( i + 1, m_checkbox_particle[i + 1]->name.c_str() );
        }
    }
    if ( m_total_item > 0 )
    {
        m_buttom_delete->enable();
        m_list_delete->enable();
    }

    m_glui_particle_main->add_button( "Close", -1, CallBackParticlePanelClose );
    m_glui_particle_main->hide();
    m_show_particle_merger_ui = false;
}

void ParticleMergerUI::particleFileActiveFunction()
{
    m_glui_add_delete->show();
}

void ParticleMergerUI::particleFileCloseFunction()
{
    m_glui_add_delete->hide();
}

void ParticleMergerUI::paricleBrowseDirectoryActiveFunction()
{
    m_glui_particele_file = GLUI_Master.create_glui( "FileDialog" );
    m_file_dialog_file = new FileDialog( m_glui_particele_file, m_buttom_directory_file_dialog, 0, m_text_file_path->get_text(), m_text_file_path );
}

void ParticleMergerUI::paricleBrowseFileActiveFunction()
{
    m_glui_particele_file = GLUI_Master.create_glui( "FileDialog" );
    m_file_dialog_file = new FileDialog( m_glui_particele_file, m_button_file_path_file_dialog, 0, m_text_output_file_path->get_text(), m_text_output_file_path );
}

void ParticleMergerUI::paricleExportDoFunction()
{
    int check = 0;
    // 表示しているものがあるか確認
    for ( int i = 0; i < 11; i++ )
    {
        if ( m_particle_value[i] == 1 )
        {
            check = 1;
        }
    }

    // 表示しているものがあれば粒子出力をする
    if ( check == 1 )
    {
        // Exportボタンを不活性に切り替える
        m_buttom_export->disable();
        // flagを立てる
        m_do_export = true;
    }
}

void ParticleMergerUI::setParam( kvs::visclient::ParticleMergeParameter* param )
{
    // printf( "In func ParticleMergerUI::setParam\n" );
    m_param = param;
    // 粒子の統合情報を設定
    for ( int i = 1; i < 11; i++ )
    {
        // 表示、非表示のフラグを設定
        if ( m_particle_value[i] == 1 )
        {
            m_param->m_particles[i].m_enable = true;
        }
        else
        {
            m_param->m_particles[i].m_enable = false;
        }
        // 粒子ファイルのパスを設定。ただし未設定の場合、表示フラグはfalseにする
        if ( m_file_path_strings[i].empty() != true )
        {
            printf( "Set filepat\n" );
            printf( "Input m_file_path %s\n", m_file_path_strings[i].c_str() );
            m_param->m_particles[i].m_file_path.clear();
            m_param->m_particles[i].m_file_path.append( m_file_path_strings[i] );
            printf( "Input m_file_path %s\n", m_param->m_particles[i].m_file_path.c_str() );
        }
        else
        {
            m_param->m_particles[i].m_enable = false;
            m_param->m_particles[i].m_file_path.clear();
        }
    }
    // 先頭のリストについてチェック
    if ( m_particle_value[0] == 1 )
    {
        m_param->m_particles[0].m_enable = true;
        m_param->m_particles[0].m_file_path = m_file_path_strings[0];
    }
    else
    {
        m_param->m_particles[0].m_enable = false;
    }

    for ( int i = 0; i < 11; i++ )
    {
        m_param->m_particles[i].m_keep_initial_step = m_keep_initial_value[i];
        m_param->m_particles[i].m_keep_final_step   = m_keep_final_value[i];
    }

    // Exportが押された時の処理
    if ( m_do_export )
    {
        if ( m_text_output_file_path->get_text() != NULL )
        {
            // Exportフラグを設定
            m_param->m_do_export = true;
            // 粒子の出力情報を設定
            m_param->m_export_file_path.clear();
            m_param->m_export_file_path.append( m_text_output_file_path->get_text() );
            // 出力時刻を記録（実際には書き出し１つ前の時刻）
            m_start_export_time = m_param->m_circuit_time;
            // Exportが押された時のフラグを元に戻す
            m_do_export = false;
        }
    }
    /*
        else{
        // 出力中の確認
        if(m_param->m_do_export){ // 出力中
            if(m_start_export_time == m_param->m_circuit_time){ // 出力時間が一周した
            m_param->m_do_export = false; // 出力を止める
            m_buttom_export->enable(); // ボタンを操作可能に戻す
            }
        }
        }
    */
    // printf( "End ParticleMergerUI::setParam\n" );
}

void ParticleMergerUI::paricleExportBottomActive()
{
    // Exportボタンを操作可能に戻す
    m_buttom_export->enable();
}

void ParticleMergerUI::paricleChangedFlag()
{
    // ビュー正規化用にflagを立てる
    m_changed = true;
}

void ParticleMergerUI::paricleResetChangedFlag()
{
    m_changed = false;
}

bool ParticleMergerUI::checkChengeFlag()
{
    return m_changed;
}

void ParticleMergerUI::particleAddFunction()
{
    printf( "In func particle_add_func \n" );
    int item_num = m_list_add->get_int_val();
    int flag = 0;

    if ( m_text_file_path->get_text() != NULL )
    {
        m_file_path_strings[item_num].clear();
        m_file_path_strings[item_num].append( m_text_file_path->get_text() );
        printf( "Set m_file_path : %s\n", m_file_path_strings[item_num].c_str() );
    }

    if ( m_text_particle_name->get_text() != NULL && m_text_file_path->get_text() != NULL )
    {
        if ( strlen( m_text_particle_name->get_text() ) > 0 )
        {
            // 削除リストを初期化
            for ( int i = 0; i < 11; i++ )
            {
                m_list_delete->delete_item( i );
            }
            m_total_item++;
            // パーティクルをリストに追加
            printf( "get particle name !!!!!! (%s)\n", m_text_particle_name->get_text() );
            // ユーザ指定の名前でリストに追加
            m_checkbox_particle[item_num]->set_name( m_text_particle_name->get_text() );
            m_checkbox_keep_initial[item_num]->set_name( m_text_particle_name->get_text() );
            m_checkbox_keep_final[item_num]->set_name( m_text_particle_name->get_text() );
            m_item_strings[item_num].erase( 0 );
            m_item_strings[item_num].append( m_text_particle_name->get_text() );
            flag = 1;
        }
    }
    if ( flag == 0 && m_text_file_path->get_text() != NULL )
    {
        if ( strlen( m_text_file_path->get_text() ) > 0 )
        {
            // 削除リストを初期化
            for ( int i = 0; i < 11; i++ )
            {
                m_list_delete->delete_item( i );
            }
            m_total_item++;
            // ディレクトリ名でリストに追加
            std::string buffdir( m_text_file_path->get_text() );
            if ( buffdir[buffdir.size() - 1] == kvs::Directory::Separator()[0] )
            {
                // パスの指定の最後が/ならば削除しておく
                printf( "erase last / \n" );
                buffdir.erase( buffdir.size() - 1, 1 );
            }
            printf( "check / \n" );
            // '/'が存在するか確認をする
            if ( buffdir.find_last_of( kvs::Directory::Separator() ) != std::string::npos )
            {
                // 上の階層のパスは削除
                buffdir.erase( 0, buffdir.find_last_of( kvs::Directory::Separator() ) + 1 );
            }
            m_checkbox_particle[item_num]->set_name( buffdir.c_str() );
            m_checkbox_keep_initial[item_num]->set_name( buffdir.c_str() );
            m_checkbox_keep_final[item_num]->set_name( buffdir.c_str() );
            m_item_strings[item_num].erase( 0 );
            m_item_strings[item_num].append( buffdir.c_str() );

            flag = 1;
        }
    }

    if ( flag == 1 )
    {
        // 操作可能にする
        m_checkbox_particle[item_num]->enable();
        m_checkbox_keep_initial[item_num]->enable();
        m_checkbox_keep_final[item_num]->enable();

        // 削除リストに追加
        for ( int i = 1; i < 11; i++ )
        {
            size_t loc = m_item_strings[i].find( "(Particle", 0 );
            if ( loc == std::string::npos )
            {
                m_list_delete->add_item( i, m_item_strings[i].c_str() );
            }
        }

        // addリストに追加
        for ( int i = 1; i < 11; i++ )
        {
            m_list_add->delete_item( i );
        }
        for ( int i = 1; i < 11; i++ )
        {
            m_list_add->add_item( i, m_checkbox_particle[i]->name.c_str() );
        }
        m_buttom_delete->enable();
        m_list_delete->enable();
    }
    printf( "End particle_add_func.\n" );
}

void ParticleMergerUI::particleDeleteFunction()
{
    printf( "In particle_delet_func\n" );
    int item_num = m_list_delete->get_int_val();
    GLUI_Listbox_Item* pitem = m_list_delete->get_item_ptr( item_num );
    if ( pitem != NULL )
    {
        printf( "delete item %s.(%d)\n", pitem->text.c_str(), item_num );
        if ( m_particle_value[item_num] != 0 )
        {
            paricleChangedFlag(); // 表示されていたものが消えるのでViewerの初期化
        }
        // パーティクルをリストをParticleに戻す
        char buf[64];
        sprintf( buf, "(Particle%d)", item_num );
        m_checkbox_particle[item_num]->set_name( buf );
        m_checkbox_keep_initial[item_num]->set_name( buf );
        m_checkbox_keep_final[item_num]->set_name( buf );
        // 内部テーブルも初期化
        m_item_strings[item_num].erase( 0 );
        m_item_strings[item_num].append( buf );
        m_file_path_strings[item_num].clear();
        // チェックを外す
        m_checkbox_particle[item_num]->set_int_val( 0 );
        m_checkbox_keep_initial[item_num]->set_int_val( 0 );
        m_checkbox_keep_final[item_num]->set_int_val( 0 );
        // 操作不可能にする
        m_checkbox_particle[item_num]->disable();
        m_checkbox_keep_initial[item_num]->disable();
        m_checkbox_keep_final[item_num]->disable();
        m_total_item--;
        // 削除リストを初期化
        for ( int i = 0; i < 11; i++ )
        {
            m_list_delete->delete_item( i );
        }
        // 削除リストを更新
        for ( int i = 1; i < 11; i++ )
        {
            size_t loc = m_item_strings[i].find( "(Particle", 0 );
            //printf( "item = %s %d\n", m_item_strings[i].c_str(), loc );
			printf("item = %s %zd\n", m_item_strings[i].c_str(), loc);
			if ( loc == std::string::npos )
            {
                m_list_delete->add_item( i, m_item_strings[i].c_str() );
            }
        }
        // addリストを初期化
        for ( int i = 1; i < 11; i++ )
        {
            m_list_add->delete_item( i );
        }
        // addリストを更新
        for ( int i = 1; i < 11; i++ )
        {
            m_list_add->add_item( i, m_checkbox_particle[i]->name.c_str() );
        }
    }
    if ( m_total_item == 0 )
    {
        m_buttom_delete->disable();
        m_list_delete->disable();
    }
}

bool ParticleMergerUI::getShowParticleMergerUI()
{
    return m_show_particle_merger_ui;
}

void ParticleMergerUI::changeShowParticleMergerUI()
{
    m_show_particle_merger_ui = ! m_show_particle_merger_ui;
}

} // namespace visclient
} // namespace kvs


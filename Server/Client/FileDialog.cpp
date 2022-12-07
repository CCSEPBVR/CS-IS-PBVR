//
// FileDialog class
//
// Created by FP)M.tanaka 2015.02.27
//

#include <sys/stat.h>
#include "FileDialog.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifndef PBVR_S_ISDIR
#define PBVR_S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef PBVR_S_ISREG
#define PBVR_S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

GLUI* g_glui;
GLUI_Button* g_btn_from;

GLUI_FileBrowser* fb_file;
GLUI_EditText* tb_directory;
GLUI_EditText* tb_file;

GLUI_EditText* g_et;

static char cdir[256];

bool IsFile( const char* );
bool IsDirectory( const char* );

// Constructor
FileDialog::FileDialog( GLUI* glui, GLUI_Button* btn_from, const int type, const char* path, GLUI_EditText* et )
{

    GLUI_Button* btn_ok;
    GLUI_Button* btn_close;

    static char buff_directory[256];
    static char buff_file[256];
    static char sdir[256], sfile[256], *p;

    void CallBackButtonOk( const int );
    void CallBackButtonClose( const int );
    void CallBackFileBrowser( const int );
    void CallBackTextBoxDirectory( const int );

    g_et = et;

    sfile[0] = '\0';
    strcpy( sdir, path );
    if ( IsFile( sdir ) )
    {
        printf( "is file : %s\n", sdir );
#ifdef WIN32
        p = strrchr( sdir, '\\' );
#else
        p = strrchr( sdir, '/' );
#endif
        if ( p != NULL )
        {
            *p = '\0';
            strcpy( sfile, ( p + 1 ) );
        }
        else
        {
#ifdef WIN32
            GetCurrentDirectory( 255, sdir );
#else
            //strcpy(sdir, "/tmp");
            getcwd( sdir, 255 );
#endif
        }
    }
    else if ( IsDirectory( sdir ) )
    {
        printf( "is dir : %s\n", sdir );
    }
    else
    {
        printf( "is not file or dir : %s\n", sdir );
#ifdef WIN32
        GetCurrentDirectory( 255, sdir );
#else
        //strcpy(sdir, "/tmp");
        getcwd( sdir, 255 );
#endif
    }


    strcpy( cdir, sdir );

    g_glui = glui;
    g_btn_from = btn_from;
    g_btn_from->disable();

    // for Raised Panel
    GLUI_Panel* panel_raised = glui->add_panel( "", GLUI_PANEL_RAISED );

    // for set directory
    tb_directory = glui->add_edittext_to_panel( panel_raised, "Set Directory ", GLUI_EDITTEXT_TEXT, buff_directory, 1, CallBackTextBoxDirectory );
    tb_directory->set_w( 200 );
    tb_directory->set_text( cdir );

    // for File list
    if ( type == 0 )
    {
        fb_file = new GLUI_FileBrowser( panel_raised, "Select File", GLUI_PANEL_EMBOSSED, -1, CallBackFileBrowser );
    }
    else
    {
        fb_file = new GLUI_FileBrowser( panel_raised, "Select Directory", GLUI_PANEL_EMBOSSED, 1, CallBackFileBrowser );
    }
    fb_file->set_alignment( GLUI_ALIGN_LEFT );
    fb_file->set_w( 200 );
//		fb_file->set_allow_change_dir(-1);
    fb_file->set_allow_change_dir( false ); // MODIFY Fj 2015.03.06
    fb_file->fbreaddir( cdir );

    // for set file
    tb_file = glui->add_edittext_to_panel( panel_raised, "Set File ", GLUI_EDITTEXT_TEXT, buff_file );
    tb_file->set_w( 200 );
    tb_file->set_text( sfile );

    // for OK/Close panel
    GLUI_Panel* panel_select = glui->add_panel_to_panel( panel_raised, "", GLUI_PANEL_NONE );
    panel_select->set_alignment( GLUI_ALIGN_LEFT );
    panel_select->set_w( 200 );

    // for OK/Close button
    if ( type == 0 )
    {
        glui->add_button_to_panel( panel_select, "OK", -1, CallBackButtonOk );
    }
    else
    {
        glui->add_button_to_panel( panel_select, "OK", 1, CallBackButtonOk );
    }
    glui->add_column_to_panel( panel_select, false );
    glui->add_button_to_panel( panel_select, "Close", 1, CallBackButtonClose );
}

// OK button
void CallBackButtonOk( const int n )
{
    static char  tb_fname[256];

    printf( "OK\n" );
//	g_glui->close();
    g_glui->hide(); // MODIFY fj 2015.03.06
    g_glui = 0;
    g_btn_from->enable();

#ifdef WIN32
    if ( n != 1 )
    {
        sprintf( tb_fname, "%s\\%s", tb_directory->get_text(), tb_file->get_text() );
    }
    else
    {
        sprintf( tb_fname, "%s", tb_directory->get_text() );
    }
#else
    if ( n != 1 )
    {
        sprintf( tb_fname, "%s/%s", tb_directory->get_text(), tb_file->get_text() );
    }
    else
    {
        sprintf( tb_fname, "%s", tb_directory->get_text() );
    }
#endif
    g_et->set_text( tb_fname );
}

// Close Button
void CallBackButtonClose( const int n )
{
    printf( "Close\n" );
//	g_glui->close();
    g_glui->hide(); // MODIFY fj 2015.03.06
    g_glui = 0;
    g_btn_from->enable();
}

// callback for FileBrowser
void CallBackFileBrowser( const int n )
{

    std::string file_name;
    char works[256];
    char fname[256];
    char* p;
//bool IsFile(char*);
//bool IsDirectory(char*);

    printf( "called CallBackFileBrowser\n" );

    file_name = fb_file->get_file();

#ifdef notopen
    printf( "cdir1 : %s\n", cdir );
    if ( cdir[strlen( cdir ) - 1] == '/' )
    {
        cdir[strlen( cdir ) - 1] = '\0';
    }
    printf( "cdir2 : %s\n", cdir );
#endif

    sprintf( fname, "%s", file_name.c_str() );
    // delete last '/'
    /*
    if(strlen(fname)>0){
    	if(fname[strlen(fname)-1] == '/'){
    		fname[strlen(fname)-1] = '\0';
    	}
    }
    */

#ifdef WIN32
    if ( strcmp( cdir, "\\" ) == 0 )
    {
        sprintf( works, "\\%s", fname );
    }
    else
    {
        if ( fname[0] == '\\' )
        {
            sprintf( works, "%s%s", cdir, fname );
        }
        else
        {
            sprintf( works, "%s\\%s", cdir, fname );
        }
    }
#else
    if ( strcmp( cdir, "/" ) == 0 )
    {
        sprintf( works, "/%s", fname );
    }
    else
    {
        sprintf( works, "%s/%s", cdir, fname );
    }
#endif

#ifdef WIN32
    if ( strcmp( fname, "." ) == 0 || strcmp( fname, ".\\" ) == 0 )
    {
        printf( "====> ./\n" );
        return;
    }
#else
    if ( strcmp( fname, "./" ) == 0 )
    {
        printf( "====> ./\n" );
        return;
    }
#endif

    if ( IsFile( works ) )
    {
        tb_file->set_text( fname );
        printf( "File : %s\n", works );
    }
    else if ( IsDirectory( works ) )
    {
        // Directory Dialogならば設定
        if ( n == 1 )
        {
            tb_file->set_text( fname );
            printf( "!!!!!!!! Directory %s !!!!!!!!!!!\n", fname );
        }
#ifdef WIN32
        if ( strcmp( fname, "\\.." ) == 0 )
        {
#else
        if ( strcmp( fname, "../" ) == 0 )
        {
#endif
            printf( "====> ../\n" );
#ifdef WIN32
            p = strrchr( cdir, '\\' );
#else
            p = strrchr( cdir, '/' );
#endif
            if ( p != NULL )
            {
                *p = '\0';
#ifdef WIN32
                if ( cdir[0] == '\0' )
                {
                    strcpy( cdir, "\\" );
                }
                else if ( strlen( cdir ) == 2 ) // ex) C:
                {
                    cdir[2] = '\0';
//					cdir[3] = '\0';
                }
#else
                if ( cdir[0] == '\0' )
                {
                    strcpy( cdir, "/" );
                }
#endif
                fb_file->fbreaddir( cdir );
                tb_directory->set_text( cdir );
                if ( n != 1 )
                {
                    if ( tb_file != NULL )  tb_file->set_text( "" );
                }
            }
            else
            {
                fb_file->fbreaddir( works );
                tb_directory->set_text( works );
                if ( n != 1 )
                {
                    tb_file->set_text( "" );
                }
                strcpy( cdir, works );
            }
        }
        else
        {
            fb_file->fbreaddir( works );
            tb_directory->set_text( works );
            if ( n != 1 )
            {
                tb_file->set_text( "" );
            }
            strcpy( cdir, works );
        }
        printf( "Dir : %s\n", works );
    }
    else
    {
        printf( "??? : %s\n", works );
    }
}

// callback for FileBrowser
void CallBackTextBoxDirectory( const int n )
{
    char works[256];

    printf( "called CallBackTextBoxDirectory\n" );

    strcpy( works, tb_directory->get_text() );
    printf( "called directory : %s\n", works );

    if ( IsDirectory( works ) )
    {
        strcpy( cdir, works );
        fb_file->fbreaddir( works );
        tb_file->set_text( "" );
    }
}

#ifdef WIN32
bool IsFile( const char* path )
{
    WIN32_FIND_DATA FN;
    HANDLE hFind;
    hFind = FindFirstFile( path, &FN );
    if ( FN.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
        FindClose( hFind );
        return false;
    }
    FindClose( hFind );
    return true;
}
#else
bool IsFile( const char* path )
{
//bool IsFile(char* path) {
    struct stat buf;
    stat( path, &buf );
    return PBVR_S_ISREG( buf.st_mode );
}
#endif

#ifdef WIN32
bool IsDirectory( const char* path )
{
    WIN32_FIND_DATA FN;
    HANDLE hFind;
    hFind = FindFirstFile( path, &FN );
    if ( FN.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
        FindClose( hFind );
        return true;
    }
    FindClose( hFind );
    return false;
}
#else
bool IsDirectory( const char* path )
{
//bool IsDirectory(char* path) {
    struct stat buf;
    stat( path, &buf );
    return PBVR_S_ISDIR( buf.st_mode );
}
#endif

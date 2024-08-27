#ifndef PBVR__FILE_DIALOG_H_INCLUDE
#define PBVR__FILE_DIALOG_H_INCLUDE

#include "Types.h"
#include "GL/glui.h"

class FileDialog
{

private:
    GLUI* m_glui;

public:
// GLUI *glui			: base window
// GLUI_Button* btn_from	: source button (for enable/disable control)
// int type			: reserved
// const char* path		: initial fullpath to file
// GLUI_EditText* et		: If OK, edit-text control for set detarmin file path
    FileDialog( GLUI* glui, GLUI_Button* btn_from, const int type, const char* path, GLUI_EditText* et );
//	void CallBackButtonOk(int n);
//	void CallBackButtonClose(int n);
//	void CallBackFileBrowser(int);
//	void CallBackTextBoxDirectory(int);
//	bool IsFile(char* file);
//	bool IsDirectory(char* file);
};

#endif // PBVR__FILE_DIALOG_H_INCLUDE

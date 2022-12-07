/****************************************************************************
  
  GLUI User Interface Toolkit
  ---------------------------

     glui_filebrowser.cpp - GLUI_FileBrowser control class


          --------------------------------------------------

  Copyright (c) 1998 Paul Rademacher

  This software is provided 'as-is', without any express or implied 
  warranty. In no event will the authors be held liable for any damages 
  arising from the use of this software. 

  Permission is granted to anyone to use this software for any purpose, 
  including commercial applications, and to alter it and redistribute it 
  freely, subject to the following restrictions: 

  1. The origin of this software must not be misrepresented; you must not 
  claim that you wrote the original software. If you use this software 
  in a product, an acknowledgment in the product documentation would be 
  appreciated but is not required. 
  2. Altered source versions must be plainly marked as such, and must not be 
  misrepresented as being the original software. 
  3. This notice may not be removed or altered from any source distribution. 


*****************************************************************************/

#include "GL/glui.h"
#include "glui_internal.h"
#include <sys/types.h>

#ifdef __GNUC__
#include <dirent.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <sys/stat.h>

GLUI_FileBrowser::GLUI_FileBrowser( GLUI_Node *parent, 
                                    const char *name,
                                    int type,
                                    int id,
                                    GLUI_CB cb)
{
  common_init();

  set_name( name );
  user_id    = id;
  int_val    = type;
  callback   = cb;

  parent->add_control( this );
  list = new GLUI_List(this, true, 1);
  list->set_object_callback( GLUI_FileBrowser::dir_list_callback, this );
// DELETE START BY FP)M.Tanaka 2015.03.02
//  list->set_click_type(GLUI_DOUBLE_CLICK);
// DELETE END T BY FP)M.Tanaka 2015.03.02
  this->fbreaddir(this->current_dir.c_str());
}

/****************************** GLUI_FileBrowser::draw() **********/

void GLUI_FileBrowser::dir_list_callback(GLUI_Control *glui_object) {
  GLUI_List *list = dynamic_cast<GLUI_List*>(glui_object);
  if (!list) 
    return;
  GLUI_FileBrowser* me = dynamic_cast<GLUI_FileBrowser*>(list->associated_object);
  if (!me)
    return;
  int this_item;
  const char *selected;
  this_item = list->get_current_item();
#ifdef __GNUC__
  if (this_item >= 0) { /* file or directory selected */
#else
  if (this_item > 0) { /* file or directory selected */
#endif
    selected = list->get_item_ptr( this_item )->text.c_str();
    if (selected[0] == '/' || selected[0] == '\\') {
      if (me->allow_change_dir) {
	if(me->user_id != -1){
           me->file = selected;
        }
#ifdef __GNUC__
        chdir(selected+1);
#endif
#ifdef _WIN32
        SetCurrentDirectory(selected+1);
#endif
        me->fbreaddir(".");
      } else{
#ifdef _WIN32
	  me->file = selected;
	  me->execute_callback();
#endif
      }
    } else {
      me->file = selected;
      me->execute_callback();
    }
  }
}



void GLUI_FileBrowser::fbreaddir(const char *d) {
	GLUI_String item;
	int i = 0;

	if (!d)
		return;

#ifdef _WIN32

	WIN32_FIND_DATA FN;
	HANDLE hFind;
	//char search_arg[MAX_PATH], new_file_path[MAX_PATH];
	//sprintf(search_arg, "%s\\*.*", path_name);

	//  hFind = FindFirstFile("*.*", &FN);
	GLUI_String tmps = d;
	if (tmps.compare("\\")==0){
		hFind = FindFirstFile(d, &FN);
	} else{
		tmps += "\\*.*";
		hFind = FindFirstFile(tmps.c_str(), &FN); // MODIFY Fj 2015.03.06
	}
  if (list) {
    list->delete_all();
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        int len = strlen(FN.cFileName);
        if(user_id==-1){
          if (FN.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            item = '\\';
            item += FN.cFileName;
          } else {
            item = FN.cFileName;
          }
          list->add_item(i,item.c_str());
          i++;
	}else{
	  if (FN.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
	    item = '\\';
            item += FN.cFileName;
            list->add_item(i,item.c_str());
            i++;
          }
	}
      } while (FindNextFile(hFind, &FN) != 0);
      
      if (GetLastError() == ERROR_NO_MORE_FILES)
        FindClose(hFind);
      else
        perror("fbreaddir");
    }
  }

#elif defined(__GNUC__)
  DIR *dir;
#ifndef __APPLE__
  struct dirent64 *dirp;
#else
  struct dirent *dirp;
#endif
  if (list) {
    list->delete_all();
    if ((dir = opendir(d)) == NULL)
      perror("fbreaddir:");
    else {
#ifndef __APPLE__
      while ((dirp = readdir64(dir)) != NULL)   /* open directory     */
#else
      while ((dirp = readdir(dir)) != NULL)   /* open directory     */
#endif
      {
	// SATRT multibyte character check !!
	int len = strlen(dirp->d_name);
	int mbflag=0;
	wchar_t wbuff[1024];
	if ( mbstowcs(wbuff, dirp->d_name, len) < 0 ) {
		continue;
	}
	size_t ssz = len;
	if ( ssz > 0 ) {
		for ( size_t ii = 0; ii < ssz; ii++ ){
			if ( dirp->d_name[ii] != wbuff[ii] ) {
				mbflag = 1; // This is multibyte character.
				break;
			}
		}
	}
	if(mbflag==1) continue; // multibyte character is not display.
	// END multibyte character check !!

	if(user_id==-1){
          if (dirp->d_type == DT_DIR) /* dir is directory   */
            item = dirp->d_name + GLUI_String("/");
          else
            item = dirp->d_name;

          list->add_item(i,item.c_str());
          i++;
	}else{
	  if (dirp->d_type == DT_DIR){ /* dir is directory   */
	    item = dirp->d_name + GLUI_String("/");
	    list->add_item(i,item.c_str());
            i++;
	  }
	}
      }
      closedir(dir);
    }
  }
#endif
}

void ProcessFiles(const char *path_name)
{	

}


void GLUI_FileBrowser::set_w(int w) 
{ 
  if (list) list->set_w(w);
}

void GLUI_FileBrowser::set_h(int h) 
{
  if (list) list->set_h(h);
}

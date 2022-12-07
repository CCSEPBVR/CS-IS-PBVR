/*
 * NormalVectorSelectionUI.h
 *
 *  Created on: 2017/01/13
 *      Author: hira
 */

#ifndef NORMALVECTORSELECTIONUI_H_
#define NORMALVECTORSELECTIONUI_H_

#include <string>
#include <vector>
#include <kvs/glut/Screen>
#include <kvs/glut/PushButton>
#include <kvs/glut/Text>
#include "Label.h"

#include "GL/glui.h"

namespace kvs {
namespace visclient {

class NormalVectorSelectionUI {
public:
    NormalVectorSelectionUI();
    virtual ~NormalVectorSelectionUI();
    void createPanel( const int client_server_mode = 0 );
    bool isShowNormalVectorUI();
    void changeShowNormalVectorUI();
    int show();
    virtual void hide();

private:
    kvs::ScreenBase*  m_screen; ///< pointer to screen
    GLUI* m_glui_normal_main;
    GLUI_Panel* m_normal_panel;
    GLUI_Panel* m_button_panel;
    GLUI_RadioGroup* m_select_normal;
    GLUI_Button  *m_btn_apply;
    GLUI_Button  *m_btn_cancel;
    bool m_isshow_normalvector;

};

} /* namespace visclient */
} /* namespace kvs */

#endif /* NORMALVECTORSELECTIONUI_H_ */

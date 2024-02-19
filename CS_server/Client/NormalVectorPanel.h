/*
 * NormalVectorPanel.h
 *
 *  Created on: 2017/01/13
 *      Author: hira
 */

#ifndef NORMALVECTORPANEL_H_
#define NORMALVECTORPANEL_H_

#include <string>
#include <kvs/ClassName>
#include <kvs/glut/Screen>
#include <kvs/glut/PushButton>
#include <kvs/glut/Text>
#include "GL/glui.h"
#include "NormalVectorSelectionUI.h"

namespace kvs {
namespace visclient {

class NormalVectorPanel {
public:
    NormalVectorPanel();
    virtual ~NormalVectorPanel();
    void createNormalVectorPanel(GLUI* glui);
    void showNormal();
    void createDialogNormalVectorSelection();

private:
    GLUI_StaticText  *m_lbl_normalvector;
    GLUI_Button  *m_btn_select;
    NormalVectorSelectionUI *m_dialogNormalVector;
};

} /* namespace visclient */
} /* namespace kvs */

#endif /* NORMALVECTORPANEL_H_ */

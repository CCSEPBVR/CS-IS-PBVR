/*
 * TransferFunctionEditorColorFunction.h
 *
 *  Created on: 2017/01/12
 *      Author: hira
 */

#ifndef TRANSFERFUNCTIONEDITORCOLORFUNCTION_H_
#define TRANSFERFUNCTIONEDITORCOLORFUNCTION_H_

#include <kvs/glut/GLUT>
#include <string>
#include <kvs/ClassName>
#include <kvs/OpenGL>
#include <kvs/glut/PushButton>
#include <kvs/glut/Text>
#include "GL/glui.h"

#include <SupportGLUT/Viewer/Screen.h>
#include "ExtendedTransferFunctionParameter.h"

namespace kvs {
namespace visclient {

enum FUNCTION_DIALOG_TYPE {
    UNKNOWN_DIALOG = 1,
    COLOR_FUNCTION_DIALOG = 1,
    OPACITY_FUNCTION_DIALOG = 2,
};

class TransferFunctionEditorMain;

/**
 * 色関数を定義する
 */
class TransferFunctionEditorColorFunction: public glut::Screen {
    kvsClassName( kvs::visclient::TransferFunctionEditorColorFunction );

public:

    typedef kvs::glut::Screen SuperClass;

public:
    TransferFunctionEditorColorFunction();
    TransferFunctionEditorColorFunction(
            kvs::visclient::TransferFunctionEditorMain* parent);
    virtual ~TransferFunctionEditorColorFunction();

    int show(FUNCTION_DIALOG_TYPE dialog_type,
            std::vector<NamedTransferFunctionParameter> *color_opacity_transfer_function);
    virtual void hide();
    virtual int select();
    virtual int save();
    virtual void cancel();
    virtual int addFunction(const int id );
    virtual int editFunction();
    virtual int deleteFunction();
    int changeFunctionList();
    FUNCTION_DIALOG_TYPE getFunctionType() {return m_dialogtype;};
    void setTransferFunctionParameterList(std::vector<NamedTransferFunctionParameter> *color_opacity_transfer_function);

protected:
    GLUI_List_Item * getCurrentFunctionItem();
    void setMessage(const char *message);
    int getEditTransferFunctionIndex(const char *left);
    bool validateFunction(const char *left, const char *right);

protected:
    bool m_visible;
    kvs::visclient::TransferFunctionEditorMain* m_parent;

    GLUI* m_glui_func_win;
    GLUI_EditText* m_glui_edit_name;
    GLUI_EditText* m_glui_edit_func;
    GLUI_List *m_funclist;
    GLUI_StaticText  *m_functionlist_label;
    GLUI_StaticText  *m_function_label;
    GLUI_StaticText  *m_message_label;
    FUNCTION_DIALOG_TYPE  m_dialogtype;

    std::vector<NamedTransferFunctionParameter> *m_original_transfer_function;

private:
    std::vector<NamedTransferFunctionParameter> m_edit_transfer_function;

};

} /* namespace visclient */
} /* namespace kvs */

#endif /* TRANSFERFUNCTIONEDITORCOLORFUNCTION_H_ */

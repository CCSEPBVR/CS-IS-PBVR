/*
 * TransferFunctionEditorColorFunction.cpp
 *
 *  Created on: 2017/01/12
 *      Author: hira
 */
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include "TransferFunctionEditorColorFunction.h"
#include "TransferFunctionEditorMain.h"


#define COLOR_FUNCTION_TITLE    "Color Function Editor"
#define OPACITY_FUNCTION_TITLE    "Opacity Function Editor"
#define COLOR_FUNCTIONLIST_TEXT    "Color Function List [ ex : C1-99 = q1+q2*X ]"
#define OPACITY_FUNCTIONLIST_TEXT    "Opacity Function List [ ex : O1-99 = q1+q2*X ]"
#define FUNCTION_TEXT    "Function"

namespace
{
kvs::visclient::TransferFunctionEditorColorFunction* ptfe;


void CallBackSelect( const int id )
{
    ptfe->select();
}

void CallBackSave( const int id )
{
    ptfe->save();
}

void CallBackCancel( const int id )
{
    ptfe->cancel();
}

void CallBackFunctionAdd( const int id )
{
    ptfe->addFunction(id);
}

void CallBackFunctionEdit( const int id )
{
    ptfe->editFunction();
}

void CallBackFunctionDelete( const int id )
{
    ptfe->deleteFunction();
}

void CallBackChangeList( const int id )
{
    ptfe->changeFunctionList();
}

} // end of namespace

namespace kvs {
namespace visclient {

/**
 * コンストラクタ
 */
TransferFunctionEditorColorFunction::TransferFunctionEditorColorFunction()
    : m_parent(NULL), m_visible(false), m_dialogtype(UNKNOWN_DIALOG)
{

}

TransferFunctionEditorColorFunction::TransferFunctionEditorColorFunction(
        kvs::visclient::TransferFunctionEditorMain* parent)
    : m_parent(parent), m_visible(false), m_original_transfer_function(NULL), m_dialogtype(UNKNOWN_DIALOG)
{

    const std::string title = "Function Editor";
    const int width = 400;
    const int height = 270;
    const int margin = 10;
    const int x = ( parent != 0 ) ? parent->x() + parent->width() : 0;
    const int y = ( parent != 0 ) ? parent->y() : 0;
    const kvs::RGBColor color( 200, 200, 200 );

    SuperClass::background()->setColor( color );
    SuperClass::setTitle( title );
    SuperClass::setPosition( x, y );
    SuperClass::setSize( width, height );

}

/**
 * デストラクタ
 */
TransferFunctionEditorColorFunction::~TransferFunctionEditorColorFunction() {

}

int TransferFunctionEditorColorFunction::show(
        FUNCTION_DIALOG_TYPE dialogtype,
        std::vector<NamedTransferFunctionParameter> *color_opacity_transfer_function)
{
    if ( m_visible ) return m_id;
    m_visible = true;

    char title[128] = {0x00};
    char list_msg[128] = {0x00};
    if (dialogtype == COLOR_FUNCTION_DIALOG) {
        strcpy(title, COLOR_FUNCTION_TITLE);
        strcpy(list_msg, COLOR_FUNCTIONLIST_TEXT);
    }
    else {
        strcpy(title, OPACITY_FUNCTION_TITLE);
        strcpy(list_msg, OPACITY_FUNCTIONLIST_TEXT);
    }
    m_dialogtype = dialogtype;

    SuperClass::setTitle( title );

    int id = SuperClass::show();
    ::ptfe = this;

    m_glui_func_win = GLUI_Master.create_glui_subwindow( m_id, GLUI_SUBWINDOW_TOP );

    m_glui_func_win->set_main_gfx_window( m_id );

    {
        m_function_label = m_glui_func_win->add_statictext(list_msg);

        m_funclist = new GLUI_List(m_glui_func_win, true, 1);
        m_funclist->set_object_callback( CallBackChangeList, m_funclist );
        m_funclist->set_w(350);
    }
    {
        GLUI_Panel     *m_func_panel = m_glui_func_win->add_panel(NULL, GLUI_PANEL_NONE);
        m_func_panel->set_alignment( GLUI_ALIGN_LEFT );

        m_glui_func_win->add_column_to_panel(m_func_panel, false);
        m_glui_edit_name = m_glui_func_win->add_edittext_to_panel(m_func_panel, "Function : ", GLUI_EDITTEXT_TEXT, NULL, -1);
        m_glui_edit_name->set_w( 40 );

        m_glui_func_win->add_column_to_panel(m_func_panel, false);
        m_glui_edit_func = m_glui_func_win->add_edittext_to_panel(m_func_panel, "= f(", GLUI_EDITTEXT_TEXT, NULL, -1);
        m_glui_edit_func->set_w( 150 );
        m_glui_edit_func->text_x_offset = 0;
        // label = ")"
        m_glui_func_win->add_column_to_panel(m_func_panel, false);
        GLUI_StaticText *label = m_glui_func_win->add_statictext_to_panel(m_func_panel, ")");
        label->set_w(5);
        label->y_off_top = 10;

        // C1,O1は初期設定された関数のみ編集可とする。追加はなし： by @hira at 2016/12/01
        // m_glui_func_win->add_column_to_panel(m_func_panel, false);
        // GLUI_Button  *add = m_glui_func_win->add_button_to_panel(m_func_panel, "Add", id, ::CallBackFunctionAdd );
        // add->set_w( 30 );

        m_glui_func_win->add_column_to_panel(m_func_panel, false);
        GLUI_Button  *edit = m_glui_func_win->add_button_to_panel(m_func_panel, "Edit", id, ::CallBackFunctionEdit );
        edit->set_w( 30 );

        // C1,O1は初期設定された関数のみ編集可とする。削除はなし： by @hira at 2016/12/01
        // m_glui_func_win->add_column_to_panel(m_func_panel, false);
        // GLUI_Button  *del = m_glui_func_win->add_button_to_panel(m_func_panel, "Del", id, ::CallBackFunctionDelete );
        // del->set_w( 30 );
    }

    {
        GLUI_Panel     *m_button_panel = m_glui_func_win->add_panel(NULL, GLUI_PANEL_NONE);
        m_button_panel->set_alignment( GLUI_ALIGN_CENTER );
        GLUI_Button  *select = m_glui_func_win->add_button_to_panel(m_button_panel, "Select", id, ::CallBackSelect );
        select->set_alignment(GLUI_ALIGN_CENTER);

        m_glui_func_win->add_column_to_panel(m_button_panel, false);
        GLUI_Button  *save = m_glui_func_win->add_button_to_panel(m_button_panel, "Save", id, ::CallBackSave );
        save->set_alignment(GLUI_ALIGN_CENTER);

        m_glui_func_win->add_column_to_panel(m_button_panel, false);
        GLUI_Button  *cancel = m_glui_func_win->add_button_to_panel(m_button_panel, "Cancel", id, ::CallBackCancel );
        cancel->set_alignment(GLUI_ALIGN_CENTER);
    }
    {
        m_message_label = m_glui_func_win->add_statictext("");
        m_message_label->set_alignment(GLUI_ALIGN_CENTER);
    }

    this->setTransferFunctionParameterList(color_opacity_transfer_function);

    // 初期関数表示
    this->changeFunctionList();

    return id;
}

void TransferFunctionEditorColorFunction::hide()
{
    if (m_id == 0) return;
    ::ptfe = NULL;
    glutSetWindow( m_id );
    int x = glutGet( ( GLenum )GLUT_WINDOW_X );
    int y = glutGet( ( GLenum )GLUT_WINDOW_Y );
    SuperClass::setPosition( x - 4, y - 28 );

    SuperClass::hide();
    m_visible = false;
}

int TransferFunctionEditorColorFunction::addFunction(const int id )
{
    int num_lines = m_funclist->num_lines;
    const char *left = this->m_glui_edit_name->get_text();
    const char *right = this->m_glui_edit_func->get_text();
    char function[256] = {0x00};
    sprintf(function, "%s = f(%s)", left , right);

    if (!this->validateFunction(left, right)) {
        return -1;
    }
    if (this->getEditTransferFunctionIndex(left) >= 0) {
        this->setMessage("[Error] Can not be added function to duplication.");
        return -1;
    }

    NamedTransferFunctionParameter edit_trans;
    edit_trans.m_name = std::string(left);
    if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
        edit_trans.m_color_variable = std::string(right);
    }
    else {
        edit_trans.m_opacity_variable = std::string(right);
    }
    this->m_edit_transfer_function.push_back(edit_trans);
    m_funclist->add_item(num_lines, function);
    m_funclist->set_int_val(num_lines);
    m_funclist->update_and_draw_text();
    m_funclist->translate_and_draw_front();

    this->setMessage("");
    return 0;
}

int TransferFunctionEditorColorFunction::editFunction()
{
    GLUI_List_Item *item = getCurrentFunctionItem();
    if (item == NULL) return -1;

    const char *left = this->m_glui_edit_name->get_text();
    const char *right = this->m_glui_edit_func->get_text();
    char function[256] = {0x00};
    sprintf(function, "%s = f(%s)", left , right);

    if (!this->validateFunction(left, right)) {
        return -1;
    }

    int curr_line = m_funclist->get_current_item();
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return -1;
    }
    NamedTransferFunctionParameter *edit_trans = &(this->m_edit_transfer_function[curr_line]);
    if (edit_trans->m_name != std::string(left)) {
        this->setMessage("[Error] Can not edit function definition name.");
        return -1;
    };

    if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
        edit_trans->m_color_variable = std::string(right);
    }
    else {
        edit_trans->m_opacity_variable = std::string(right);
    }

    edit_trans->m_name = std::string(left);
    item->text = function;

    this->setMessage("");
    return item->id;
}


int TransferFunctionEditorColorFunction::deleteFunction()
{
    GLUI_List_Item *item = getCurrentFunctionItem();
    if (item == NULL) return -1;

    int curr_line = m_funclist->get_current_item();
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return -1;
    }

    std::vector<NamedTransferFunctionParameter>::iterator itr = this->m_edit_transfer_function.begin() + curr_line;
    this->m_edit_transfer_function.erase(itr);

    item->unlink();
    delete item;
    m_funclist->num_lines--;

    return item->id;
}


int TransferFunctionEditorColorFunction::select()
{
    if (this->save() < 0) {
        return -1;
    }

    int curr_line = m_funclist->get_current_item();
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return -1;
    }

    bool selected = false;
    NamedTransferFunctionParameter select_trans = this->m_edit_transfer_function[curr_line];
    if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
        selected = this->m_parent->selectTransferFunctionEditorColorFunction(&select_trans);
    }
    else {
        selected = this->m_parent->selectTransferFunctionEditorOpacityFunction(&select_trans);
    }
    if (!selected) {
        this->setMessage("[Error] Can not select Transfer Function.");
        return -1;
    }
    this->hide();
    this->setMessage("");

    return 0;
}

int TransferFunctionEditorColorFunction::save()
{
    if (this->m_original_transfer_function == NULL) {
        this->setMessage("[Error] Invalid transfer function.");
        return -1;
    }
    std::vector<NamedTransferFunctionParameter>::iterator edit_itr;
    std::vector<NamedTransferFunctionParameter>::iterator save_itr;
    for (edit_itr = this->m_edit_transfer_function.begin(); edit_itr != this->m_edit_transfer_function.end(); edit_itr++) {
        NamedTransferFunctionParameter edit_trans = (*edit_itr);
        bool exists = false;
        for (save_itr = this->m_original_transfer_function->begin(); save_itr != this->m_original_transfer_function->end(); save_itr++) {
            NamedTransferFunctionParameter *save_trans = &(*save_itr);
            if (edit_trans.m_name == save_trans->m_name) {
                if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
                    save_trans->m_color_variable = edit_trans.m_color_variable;
                }
                else {
                    save_trans->m_opacity_variable = edit_trans.m_opacity_variable;
                }
                exists = true;
            }
        }
        if (!exists) {
            this->m_original_transfer_function->push_back(edit_trans);
        }
    }

    // 定義名でソート
    std::sort(this->m_original_transfer_function->begin(), this->m_original_transfer_function->end(), NamedTransferFunctionParameter::compareName);

    this->setMessage("");
    return 0;
}

void TransferFunctionEditorColorFunction::cancel()
{
    this->hide();
}

int TransferFunctionEditorColorFunction::changeFunctionList()
{

    GLUI_List_Item *item = getCurrentFunctionItem();
    if (item == NULL) return -1;

    int curr_line = m_funclist->get_current_item();
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return -1;
    }

    NamedTransferFunctionParameter edit_trans = this->m_edit_transfer_function[curr_line];
    this->m_glui_edit_name->set_text(edit_trans.m_name.c_str());
    if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
        this->m_glui_edit_func->set_text(edit_trans.m_color_variable.c_str());
    }
    else {
        this->m_glui_edit_func->set_text(edit_trans.m_opacity_variable.c_str());
    }

    this->setMessage("");
    return 0;
}

GLUI_List_Item * TransferFunctionEditorColorFunction::getCurrentFunctionItem()
{
    int curr_line = m_funclist->get_current_item();
    if (curr_line < 0) return NULL;

    GLUI_List_Item *item = NULL;
    item = (GLUI_List_Item *) m_funclist->items_list.first_child();
    int count = 0;
    while( item ) {
        if (curr_line == count) {
            break;
        }
        item = (GLUI_List_Item *) item->next();
        count++;
    }

    return item;
}

void TransferFunctionEditorColorFunction::setTransferFunctionParameterList(
        std::vector<NamedTransferFunctionParameter> *original_transfer_function)
{
    this->m_original_transfer_function = original_transfer_function;
    if (original_transfer_function == NULL) return;

    this->m_edit_transfer_function.clear();
    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = original_transfer_function->begin(); itr != original_transfer_function->end(); itr++) {
        NamedTransferFunctionParameter trans = (*itr);

        NamedTransferFunctionParameter edit_trans;
        edit_trans.m_name = trans.m_name;
        const char *left = trans.m_name.c_str();
        const char *right = NULL;
        if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
            if (trans.m_name.substr(0, 1) != "C") continue;
            right = trans.m_color_variable.c_str();
            edit_trans.m_color_variable = trans.m_color_variable;
        }
        else {
            if (trans.m_name.substr(0, 1) != "O") continue;
            right = trans.m_opacity_variable.c_str();
            edit_trans.m_opacity_variable = trans.m_opacity_variable;
        }

        char function[256] = {0x00};
        sprintf(function, "%s = f(%s)", left , right);
        int num_lines = m_funclist->num_lines;
        m_funclist->add_item(num_lines, function);

        this->m_edit_transfer_function.push_back(edit_trans);
    }

    return;
}


void TransferFunctionEditorColorFunction::setMessage(const char *message)
{
    this->m_message_label->set_text(message);
}

int TransferFunctionEditorColorFunction::getEditTransferFunctionIndex(const char *left)
{
    int index = 0;
    std::string name = std::string(left);

    std::vector<NamedTransferFunctionParameter>::iterator itr;
    for (itr = this->m_edit_transfer_function.begin(); itr != this->m_edit_transfer_function.end(); itr++) {
        NamedTransferFunctionParameter edit_trans = (*itr);
        if (edit_trans.m_name == name) {
            return index;
        }
        index++;
    }

    return -1;
}

bool TransferFunctionEditorColorFunction::validateFunction(const char *left, const char *right)
{

    if (left == NULL || strlen(left) <= 0) {
        this->setMessage("[Error] Empty Function name.");
        return false;
    }
    if (right == NULL || strlen(right) <= 0) {
        this->setMessage("[Error] Empty Function variable.");
        return false;
    }

    char prefix = left[0];
    if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
        if (prefix != 'C') {
            this->setMessage("[Error] prefix is not 'C' for color.");
            return false;
        }
    }
    else {
        if (prefix != 'O') {
            this->setMessage("[Error] prefix is not 'O' for opacity.");
            return false;
        }
    }

    const char *surfix = left+1;
    int func_num = atoi(surfix);
    if (func_num <= 0 || func_num > 99) {
        this->setMessage("[Error] Function number is not between 1 and 99.");
        return false;
    }


    return true;
}

} /* namespace visclient */
} /* namespace kvs */

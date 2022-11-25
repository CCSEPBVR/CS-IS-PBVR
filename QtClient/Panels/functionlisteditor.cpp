#include "functionlisteditor.h"
#include "ui_functionlisteditor.h"
#include "transferfunceditor.h"

#include "Client/v3defines.h"

#define COLOR_FUNCTION_TITLE    "Color Function Editor"
#define OPACITY_FUNCTION_TITLE    "Opacity Function Editor"
#define COLOR_FUNCTIONLIST_TEXT    "Color Function List [ ex : C1-99 = q1+q2*X ]"
#define OPACITY_FUNCTIONLIST_TEXT    "Opacity Function List [ ex : O1-99 = q1+q2*X ]"
#define FUNCTION_TEXT    "Function"

// Flag to ignore current changed signal when adding and removing rows to control.
bool ignore_signal=false;

FunctionListEditor::FunctionListEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FunctionListEditor)
{
    ui->setupUi(this);
    m_parent=(TransferFuncEditor*)parent;
    connect(ui->functionList,&QListWidget::currentRowChanged,
            this, &FunctionListEditor::onFunctionListRowChanged);

    connect(ui->setButton,&QPushButton::clicked,
            this, &FunctionListEditor::onSetButtonClicked);
    connect(ui->selectButton,&QPushButton::clicked,
            this, &FunctionListEditor::onSelectButtonClicked);
    connect(ui->saveButton,&QPushButton::clicked,
            this, &FunctionListEditor::onSaveButtonClicked);
    connect(ui->cancelButton,&QPushButton::clicked,
            this, &FunctionListEditor::onCancelButtonClicked);

}

FunctionListEditor::~FunctionListEditor()
{
    delete ui;
}

/**
 * @brief FunctionListEditor::show, shows the dialog and waits for accept or cancel.
 * @param dialogtype
 * @param color_opacity_transfer_function
 * @return
 */
void FunctionListEditor::initalize(FUNCTION_DIALOG_TYPE dialogtype,  kvs::visclient::ExtendedTransferFunctionMessage& m_doc, int i)
{
    //Osaki QTISPBVR update(2020/11/16) IS版でfunctionListEditorを使用する際に必要となるためifdefを無効にしました。
//#ifdef CS_MODE
    this->m_dialogtype=dialogtype;
    if (dialogtype == COLOR_FUNCTION_DIALOG) {
        setWindowTitle(COLOR_FUNCTION_TITLE);
        ui->example->setText(COLOR_FUNCTIONLIST_TEXT);
        setTransferFunctionParameterList(&m_doc.m_color_transfer_function);
    }
    else {
        setWindowTitle(OPACITY_FUNCTION_TITLE);
        ui->example->setText(OPACITY_FUNCTIONLIST_TEXT);
        setTransferFunctionParameterList(&m_doc.m_opacity_transfer_function);
    }
    ui->functionList->setCurrentRow(i);
//#endif
}

/**
 * @brief FunctionListEditor::getSelectedFunction
 * @return
 */
int FunctionListEditor::getSelectedFunctionIndex(){
    return ui->functionList->currentRow();

}
/**
 * @brief FunctionListEditor::save
 * @return 0 if successfully saved
 */
int FunctionListEditor::save()
{
    if (this->m_original_transfer_function == NULL) {
        this->setMessage("[Error] Invalid transfer function.");
        return -1;
    }


//    *m_original_transfer_function = m_edit_transfer_function;

    // Changed by Martin - oct 13 2020
    //
    //  FunctionListEditor::setTransferFunctionParameterList only copies in m_opacity_variable,
    //  and m_color_variable from  *m_original_transfer_function.  This means m_edit_transfer_function
    //  is incomplete - and can m_original_transfer_function can not be restored by simple
    //  *m_original_transfer_function = m_edit_transfer_function
    //
    //  (In the current status, it wont help to add a fix to setTransferFunctionParameterList to copy
    //  range variables, as m_doc may still be incomplete when TransferFunctionEditor sets up this dialog)

    NamedTransferFunctionParameter edit_ntfp;

    for (size_t i=0; i< m_edit_transfer_function.size(); i++){
        edit_ntfp=m_edit_transfer_function[i];
        if (edit_ntfp.m_name.substr(0, 1) == "O" && this->m_dialogtype == OPACITY_FUNCTION_DIALOG) {
            m_original_transfer_function->at(i).m_opacity_variable = edit_ntfp.m_opacity_variable;
        }
        if (edit_ntfp.m_name.substr(0, 1) == "C" && this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
            m_original_transfer_function->at(i).m_color_variable = edit_ntfp.m_color_variable;
        }
    }


    //    std::vector<NamedTransferFunctionParameter>::iterator edit_itr;
    //    std::vector<NamedTransferFunctionParameter>::iterator save_itr;
    //    for (edit_itr = this->m_edit_transfer_function.begin(); edit_itr != this->m_edit_transfer_function.end(); edit_itr++) {
    //        NamedTransferFunctionParameter edit_trans = (*edit_itr);
    //        bool exists = false;
    //        for (save_itr = this->m_original_transfer_function->begin(); save_itr != this->m_original_transfer_function->end(); save_itr++) {
    //            NamedTransferFunctionParameter *save_trans = &(*save_itr);
    //            if (edit_trans.Name == save_trans->Name) {
    //                if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
    //                    save_trans->ColorVar = edit_trans.ColorVar;
    //                }
    //                else{
    //                    save_trans->OpacityVar = edit_trans.OpacityVar;
    //                }
    //                exists = true;
    //            }

    //        }

    //        if (!exists) {
    //            this->m_original_transfer_function->push_back(edit_trans);
    //        }
    //    }

    // 定義名でソート
    //    std::sort(this->m_original_transfer_function->begin(), this->m_original_transfer_function->end(), NamedTransferFunctionParameter::compareName);

    this->setMessage("");
    return 0;
}

/**
 * @brief FunctionListEditor::setMessage set the message label text
 * @param message
 */
void FunctionListEditor::setMessage(QString message)
{
    ui->messageLabel->setText(message);
}


/**
 * @brief FunctionListEditor::setTransferFunctionParameterList
 * @param original_transfer_function
 */
void FunctionListEditor::setTransferFunctionParameterList(
        std::vector<NamedTransferFunctionParameter> *original_transfer_function)
{
    ignore_signal=true;
    this->m_original_transfer_function = original_transfer_function;
    if (original_transfer_function == NULL) return;
    ui->functionList->clearSelection();
    ui->functionList->clear();
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
        ui->functionList->addItem(QString(function));
        //        m_funclist->add_item(num_lines, function);

        this->m_edit_transfer_function.push_back(edit_trans);
    }

    ignore_signal=false;
    return;
}

/**
 * @brief FunctionListEditor::validateFunction validates new function
 * @param left side (function name)
 * @param right side (function body)
 * @return
 */
bool FunctionListEditor::validateFunction(const char *left, const char *right)
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

/**
 * @brief FunctionListEditor::on_functionList_currentRowChanged
 * @param curr_line currently selected line
 */
void FunctionListEditor::onFunctionListRowChanged(int curr_line)
{

    if (ignore_signal) return;
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return;
    }
    NamedTransferFunctionParameter edit_trans = this->m_edit_transfer_function[curr_line];
    if(m_dialogtype ==COLOR_FUNCTION_DIALOG){
        ui->selectedName->setText(edit_trans.m_name.c_str());
        ui->selectedFunction->setText(edit_trans.m_color_variable.c_str());
    }
    else{
        ui->selectedName->setText(edit_trans.m_name.c_str());
        ui->selectedFunction->setText(edit_trans.m_opacity_variable.c_str());
    }
    this->setMessage("");
}

/**
 * @brief FunctionListEditor::on_editButton_clicked
 */
void FunctionListEditor::onSetButtonClicked()
{

    std::string sleft=ui->selectedName->text().toStdString();
    std::string sright=ui->selectedFunction->text().toStdString();

    const char *left = sleft.c_str();
    const char *right =sright.c_str();
    char function[256] = {0x00};
    sprintf(function, "%s = f(%s)", left , right);

    if (!this->validateFunction(left, right)) {
        return;
    }

    int curr_line =ui->functionList->currentRow();
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return;
    }
    NamedTransferFunctionParameter *edit_trans = &(this->m_edit_transfer_function[curr_line]);
    if (edit_trans->m_name != std::string(left)) {
        this->setMessage("[Error] Can not edit function definition name.");
        return ;
    };

    if (this->m_dialogtype == COLOR_FUNCTION_DIALOG) {
        edit_trans->m_color_variable = std::string(right);
    }
    else {
        edit_trans->m_opacity_variable = std::string(right);
    }

    edit_trans->m_name = std::string(left);
    ui->functionList->currentItem()->setText(QString(function));


    this->setMessage("");

}

/**
 * @brief FunctionListEditor::on_selectButton_clicked
 *          triggers dialog accept()
 */
void FunctionListEditor::onSelectButtonClicked()
{
    if (this->save() < 0) {
        return ;
    }

    int curr_line = ui->functionList->currentRow();
    if (this->m_edit_transfer_function.size() < curr_line + 1) {
        this->setMessage("[Error] Invalid edit transfer list size.");
        return ;
    }

    bool selected = false;
    //    NamedTransferFunctionParameter select_trans = this->m_edit_transfer_function[curr_line];

    //    selected = this->m_parent->selectTransferFunctionEditorColorFunction(&select_trans);
    //    if (!selected) {
    //        this->setMessage("[Error] Can not select Transfer Function.");
    //        return;
    //    }
    //    this->hide();
    this->setMessage("");

    // Martin changed - caller will show dialog as modal and handle update.
//    if (m_dialogtype ==COLOR_FUNCTION_DIALOG )
//        this->m_parent->selectTransferFunctionEditorColorFunction(curr_line);
//    if (m_dialogtype==OPACITY_FUNCTION_DIALOG)
//        this->m_parent->selectTransferFunctionEditorOpacityFunction(curr_line);
    this->accept();
}

/**
 * @brief FunctionListEditor::on_saveButton_clicked
 */
void FunctionListEditor::onSaveButtonClicked()
{
    this->save();
}
/**
 * @brief FunctionListEditor::on_cancelButton_clicked
 *  triggers dialog reject()
 */
void FunctionListEditor::onCancelButtonClicked()
{
    //    this->close();
    this->reject();
}

#include "transferfunceditor_CS.h"
#include "ui_transferfunceditor.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include "Client/ParameterFile.h"
#include "QGlue/extCommand.h"
#include "QGlue/renderarea.h"
#include <fstream>
#include <QGlue/typedSignalConnect.h>

static bool blockEventHandling=false;
static kvs::visclient::FrequencyTable ft;
static kvs::visclient::ExtendedTransferFunctionMessage importdoc;

TransferFuncEditor::TransferFuncEditor(QWidget *parent) :
    QDialog(parent),
    cFuncEditor(this),
    oFuncEditor(this),
    ui(new Ui::TransferFuncEditor)
{
    blockEventHandling=true;
    ui->setupUi(this);
    m_opacity_map_palette = ui->qscr_opacity_map;
    m_opacity_map_palette->setEditable(false);
    m_opacity_map_palette->setOpacityMap(m_doc.m_opacity_transfer_function[0].opacityMap());

    m_color_map_palette =  ui->qscr_color_map;
    m_color_map_palette->setEditable(false,nullptr);
    m_color_map_palette->setColorMap(m_doc.m_color_transfer_function[0].colorMap());

    m_color_histogram = ui->qscr_color_hist;
    m_color_histogram->setGraphColor(kvs::RGBAColor( 64, 64,64, 1.0f ));
    m_opacity_histogram = ui->qscr_opacity_hist;
    m_opacity_histogram->setGraphColor(kvs::RGBAColor( 64, 64,64, 1.0f ));
    m_doc_initial=m_doc;

    ui->operatingModeStack->setCurrentIndex(0);

    this->populateColorFunctionLists(m_doc.m_transfer_function_number);
    this->populateOpacityFunctionLists(m_doc.m_transfer_function_number);

    cFuncEditor.initalize( FunctionListEditor::COLOR_FUNCTION_DIALOG, this->m_doc, ui->colormapFunction->currentIndex());
    oFuncEditor.initalize( FunctionListEditor::OPACITY_FUNCTION_DIALOG, this->m_doc, ui->opacitymapFunction->currentIndex());

    blockEventHandling=false;
}

TransferFuncEditor::~TransferFuncEditor()
{
    this->colorEditor.close();
    this->opacityEditor.close();
    delete ui;
}

/**
 * @brief TransferFuncEditor::apply
 */
void TransferFuncEditor::apply()
{
   std::cout<<"---\n m_color_transfer_function_synthesis:"<< m_doc.m_color_transfer_function_synthesis.c_str()<<std::endl;

    for (size_t i=0; i < m_doc.m_color_transfer_function.size(); i++){
        std::cout<<"  m_color_transfer_function["<<i<<"] "
                 <<m_doc.m_color_transfer_function.at(i).m_name <<": "
                 <<m_doc.m_color_transfer_function.at(i).m_color_variable.c_str() <<   "    [ "
                 <<m_doc.m_color_transfer_function.at(i).m_color_variable_min <<": "
                 <<m_doc.m_color_transfer_function.at(i).m_color_variable_max <<   " ]"<<std::endl;
    }
    std::cout<<"\nm_opacity_transfer_function_synthesis:"<< m_doc.m_opacity_transfer_function_synthesis.c_str()<<std::endl;
    for (size_t i=0; i < m_doc.m_opacity_transfer_function.size(); i++){
        std::cout<<"m_opacity_transfer_function["<<i<<"] "
                 <<m_doc.m_opacity_transfer_function.at(i).m_name <<": "
                 <<m_doc.m_opacity_transfer_function.at(i).m_opacity_variable.c_str() <<   "    [ "
                 <<m_doc.m_opacity_transfer_function.at(i).m_opacity_variable_min <<": "
                 <<m_doc.m_opacity_transfer_function.at(i).m_opacity_variable_max <<   " ]"<<std::endl;
    }
    std::cout<<"   "<<std::endl;


    int n_select_color = ui->colormapFunction->currentIndex();
    int n_select_opacity =ui->opacitymapFunction->currentIndex();

     requested_opacity_min=m_doc.m_opacity_transfer_function.at(n_select_opacity).m_opacity_variable_min;
     requested_opacity_max=m_doc.m_opacity_transfer_function.at(n_select_opacity).m_opacity_variable_max;
     requested_color_min=m_doc.m_color_transfer_function.at(n_select_color).m_color_variable_min;
     requested_color_max=m_doc.m_color_transfer_function.at(n_select_color).m_color_variable_max;

    extCommand->m_parameter.m_parameter_extend_transfer_function = m_doc;
    extCommand->m_screen->update();
}



/**
 * @brief TransferFuncEditor::ConnectSignalsToSlots,
 *        Connects all UI Signals to Slots
 */
void TransferFuncEditor::connectSignalsToSlots()
{
    //DEL BY)T.Osaki 2020.05.25
    //connect_T(ui->resolution,      QSpinBox,  valueChanged,int,  &TransferFuncEditor::onResolutionChanged);
    connect_T(ui->num_transfer,    QSpinBox, valueChanged, int,  &TransferFuncEditor::onNumTransferChanged);

    connect_T(ui->transfer_function_min_color,    QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFminColorChanged);
    connect_T(ui->transfer_function_max_color,    QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFmaxColorChanged);
    connect_T(ui->transfer_function_min_opacity,  QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFminOpacityChanged);
    connect_T(ui->transfer_function_max_opacity,  QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFmaxOpacityChanged);
    connect_T(ui->transfer_function_min_opacity,  QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFminOpacityChanged);
    connect_T(ui->lock_color_range,               QToolButton,     clicked ,bool, &TransferFuncEditor::onLockColorRange);
    connect_T(ui->lock_opacity_range,               QToolButton,     clicked ,bool, &TransferFuncEditor::onLockOpacityRange);


    connect_T(ui->colormapFunction,  QComboBox, currentIndexChanged, int, &TransferFuncEditor::onColorMapFunctionChanged);
    connect_T(ui->opacitymapFunction,QComboBox, currentIndexChanged, int, &TransferFuncEditor::onOpacityMapFunctionChanged);

    connect_T(ui->cfuncButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::onCfuncClicked);
    connect_T(ui->ofuncButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::onOfuncClicked);
    connect_T(ui->colorButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::onColorButtonClicked);
    connect_T(ui->opacityButton,   QAbstractButton, clicked, bool, &TransferFuncEditor::onOpacityButtonClicked);
    connect_T(ui->importButton,    QAbstractButton, clicked, bool, &TransferFuncEditor::onImportButtonClicked);
    connect_T(ui->exportButton,    QAbstractButton, clicked, bool, &TransferFuncEditor::onExportButtonClicked);
    connect_T(ui->resetButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::onResetButtonClicked);
    connect_T(ui->applyButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::apply);

    connect_T(ui->color_function_synth,  QLineEdit, textEdited, const QString&, &TransferFuncEditor::onColorFunctionSynthEdited);
    connect_T(ui->opacity_function_synth,QLineEdit, textEdited, const QString&, &TransferFuncEditor::onOpacityFunctionSynthEdited);
    connect_T(ui->transfer_function_var_color,    QLineEdit, textEdited, const QString&, &TransferFuncEditor::onTransferFunctionVarColorEdited);
    connect_T(ui->transfer_function_var_opacity,  QLineEdit, textEdited, const QString&, &TransferFuncEditor::onTransferFunctionVarOpacityEdited);

    //ADD BY)T.Osaki 2020.02.05
    //connect(ui->resolutionChange, &QPushButton::clicked,this,&TransferFuncEditor::onResolutionChangeButtonClicked);
}
void TransferFuncEditor::onLockOpacityRange(bool locked)
{
    std::cout<<"TransferFuncEditor::onLockOpacityRange  "<<locked<<std::endl;
    ui->transfer_function_max_opacity->setDisabled(locked);
    ui->transfer_function_min_opacity->setDisabled(locked);
    if (locked){
        ui->transfer_function_max_opacity->setValue(ui->range_max_opacity->text().toDouble());
        ui->transfer_function_min_opacity->setValue(ui->range_min_opacity->text().toDouble());
//        this->apply();
    }
}
void TransferFuncEditor::onLockColorRange(bool locked)
{
    std::cout<<"TransferFuncEditor::onLockColorRange  "<<locked<<std::endl;
    ui->transfer_function_max_color->setDisabled(locked);
    ui->transfer_function_min_color->setDisabled(locked);
    if (locked){
        ui->transfer_function_max_color->setValue(ui->range_max_color->text().toDouble());
        ui->transfer_function_min_color->setValue(ui->range_min_color->text().toDouble());
//        this->apply();
    }
}
/**
 * @brief TransferFuncEditor::exportFile, export the transfer function file
 * @param fname file name
 * @param append,  wether to append to existing file or not
 */
void TransferFuncEditor::exportFile( const std::string& fname, const bool append)
{
    std::ofstream ofs;
    if ( append )
    {
        ofs.open( fname.c_str(), std::ofstream::app );
    }
    else
    {
        ofs.open( fname.c_str(), std::ofstream::out );
    }
    if ( ofs.fail() )
    {
        std::cerr << "Error: open file " << fname << std::endl;
        return;
    }

    ofs << "TF_RESOLUTION=" << m_doc.m_extend_transfer_function_resolution << std::endl;
    ofs << "TF_NUMBER=" << m_doc.m_transfer_function_number << std::endl;
    ofs << "TF_SYNTH_C=" << m_doc.m_color_transfer_function_synthesis << std::endl;
    ofs << "TF_SYNTH_O=" << m_doc.m_opacity_transfer_function_synthesis << std::endl;

    for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
        std::stringstream ss;
        int name_number =0;
        name_number = m_doc.m_color_transfer_function[n].getNameNumber();
        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "C=" << m_doc.m_color_transfer_function[n].m_name << std::endl;
        ofs << tag_base << "VAR_C=" << m_doc.m_color_transfer_function[n].m_color_variable << std::endl;
        ofs << tag_base << "MIN_C=" << m_doc.m_color_transfer_function[n].m_color_variable_min << std::endl;
        ofs << tag_base << "MAX_C=" << m_doc.m_color_transfer_function[n].m_color_variable_max << std::endl;
        kvs::ColorMap::Table color_table = m_doc.m_color_transfer_function[n].colorMap().table();

        ofs << tag_base << "TABLE_C=";
        for ( size_t i = 0; i < color_table.size(); i++ )
        {
            ofs << static_cast<int>( color_table.at( i ) ) << ",";
        }
        ofs << std::endl;
    }

    for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
        std::stringstream ss;
        int name_number =0;
        name_number = m_doc.m_opacity_transfer_function[n].getNameNumber();

        ss << "TF_NAME" << name_number << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "O=" << m_doc.m_opacity_transfer_function[n].m_name << std::endl;
        ofs << tag_base << "VAR_O=" << m_doc.m_opacity_transfer_function[n].m_opacity_variable << std::endl;
        ofs << tag_base << "MIN_O=" << m_doc.m_opacity_transfer_function[n].m_opacity_variable_min << std::endl;
        ofs << tag_base << "MAX_O=" << m_doc.m_opacity_transfer_function[n].m_opacity_variable_max << std::endl;

        kvs::OpacityMap::Table opacity_table = m_doc.m_opacity_transfer_function[n].opacityMap().table();
        ofs << tag_base << "TABLE_O=";
        for ( size_t i = 0; i < opacity_table.size(); i++ )
        {
            ofs << opacity_table.at( i ) << ",";
        }
        ofs << std::endl;
    }
    ofs.close();

    std::cerr << "TransferFunction parameters are exported to " << fname << std::endl;

}

/**
* @brief TransferFuncEditor::importFile , import transfer function file
* @param fname
*/
void TransferFuncEditor::importFile( const std::string& fname )
{


    ParameterFile param;
    bool stat;

    stat = param.loadIN( fname );

    if ( !stat ) return;


    const size_t resolution = param.getInt( "tf_resolution" );
    importdoc.m_extend_transfer_function_resolution =  resolution;

    std::cout<<"TF_RESOLUTION;"<<resolution<<std::endl;
    if (!param.hasParam("TF_SYNTH_C") || !param.hasParam("TF_SYNTH_O")) {
        std::cerr << "[Error] This import file is old format. file=" << fname << std::endl;
        QMessageBox::warning(this, QString("Obsolete File Format"), QString("The selected file is using an old unsupported format"));
        return;
    }

    importdoc.m_transfer_function_number = param.getInt( "TF_NUMBER" );
    importdoc.m_color_transfer_function_synthesis = param.getString( "TF_SYNTH_C" );
    importdoc.m_opacity_transfer_function_synthesis = param.getString( "TF_SYNTH_O" );

    importdoc.m_color_transfer_function.clear();
    for ( size_t n = 0; n < importdoc.m_transfer_function_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "C";

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        if (!param.hasParam(name)) {
            char c_name[8] = {0x00};
            sprintf(c_name, "C%zd", n + 1);
            trans.m_name = std::string(c_name);
            trans.m_color_variable      = "q1";
        }
        else {
            trans.m_name = param.getString( tag_base + "C" );
            trans.m_color_variable      = param.getString( tag_base + "VAR_C" );
            trans.m_color_variable_min   = param.getFloat( tag_base + "MIN_C" );
            trans.m_color_variable_max   = param.getFloat( tag_base + "MAX_C" );

            std::string s_color = param.getString( tag_base + "TABLE_C" );
            std::replace( s_color.begin(), s_color.end(), ',', ' ' );
            std::stringstream ss_color( s_color );
            kvs::ColorMap::Table color_table( resolution * 3 );
            for ( size_t i = 0; i < resolution; i++ )
            {
                for ( size_t c = 0; c < 3; c++ )
                {
                    int color_e;
                    ss_color >> color_e;
                    color_table.at( i * 3 + c ) = color_e;
                }
            }
            kvs::ColorMap color_map( color_table );
            trans.setColorMap(color_map);
        }
        if (importdoc.getColorTransferFunction(trans.m_name) == NULL) {
            importdoc.m_color_transfer_function.push_back(trans);
        }
    }

    importdoc.m_opacity_transfer_function.clear();
    for ( size_t n = 0; n < importdoc.m_transfer_function_number; n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";
        const std::string tag_base = ss.str();
        std::string name = tag_base + "O";

        NamedTransferFunctionParameter trans;
        trans.setResolution( resolution );
        if (!param.hasParam(name)) {
            char o_name[8] = {0x00};
            sprintf(o_name, "O%zd", n + 1);
            trans.m_name = std::string(o_name);
            trans.m_opacity_variable      = "q1";
        }
        else {
            trans.m_name = param.getString( tag_base + "O" );
            trans.m_opacity_variable    = param.getString( tag_base + "VAR_O" );
            trans.m_opacity_variable_min = param.getFloat( tag_base + "MIN_O" );
            trans.m_opacity_variable_max = param.getFloat( tag_base + "MAX_O" );

            std::string s_opacity = param.getString( tag_base + "TABLE_O" );
            std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );
            std::stringstream ss_opacity( s_opacity );
            kvs::OpacityMap::Table opacity_table( resolution );
            for ( size_t i = 0; i < resolution; i++ )
            {
                float opacity;
                ss_opacity >> opacity;
                opacity_table.at( i ) = opacity;
            }
            kvs::OpacityMap opacity_map( opacity_table );
            trans.setOpacityMap(opacity_map);
        }

        if (importdoc.getOpacityTransferFunction(trans.m_name) == NULL) {
            importdoc.m_opacity_transfer_function.push_back(trans);
        }
    }

    this->m_doc=importdoc;
    this->m_doc_initial=importdoc;

    //DEL BY)T.Osaki 2020.05.25
    //ui->resolution->setValue(resolution);
    ui->num_transfer->setValue(importdoc.m_transfer_function_number );
//    ui->tfSythesizer->setText(importdoc.ext_colorTransferFunctionSynthesis );
    ui->color_function_synth->setText(QString::fromStdString(importdoc.m_color_transfer_function_synthesis) );
    ui->opacity_function_synth->setText(QString::fromStdString(importdoc.m_opacity_transfer_function_synthesis));
    onColorMapFunctionChanged(ui->colormapFunction->currentIndex());
    onOpacityMapFunctionChanged(ui->opacitymapFunction->currentIndex());

    extCommand->m_is_import_transfer_function_parameter = true;
    this->apply();
}



/**
 * @brief TransferFuncEditor::populateOpacityFunctionLists
 *        Popuplates the opacity function list with O1 .. On
 * @param num
 */
void TransferFuncEditor::populateOpacityFunctionLists(int n){
    QStringList itemList;
    for (int i=1;i <=n;i++ ){
        itemList.append(QString("O%1").arg(i));
    }
    ui->opacitymapFunction->blockSignals(true);
    ui->opacitymapFunction->clear();
    ui->opacitymapFunction->addItems(itemList);
    ui->opacitymapFunction->blockSignals(false);
}

/**
 * @brief TransferFuncEditor::populateColorFunctionLists
 *        Popuplates the color function list with C1 .. Cn
 * @param n
 */
void TransferFuncEditor::populateColorFunctionLists(int n){

    QStringList itemList;
    for (int i=1;i <=n;i++ ){
        itemList.append(QString("C%1").arg(i));
    }
    ui->colormapFunction->blockSignals(true);
    ui->colormapFunction->clear();
    ui->colormapFunction->addItems(itemList);
    ui->colormapFunction->blockSignals(false);
}



/**
 * @brief TransferFuncEditor::selectTransferFunctionEditorOpacityFunction 不透明度関数定義を選択する。
 * @param select_trans the selected transfer function
 * @return
 */
bool TransferFuncEditor::selectTransferFunctionEditorOpacityFunction(int num)
{
    if (num >= ui->opacitymapFunction->count()){
        return false;
    }
    ui->opacitymapFunction->setCurrentIndex(num);
    //Martin added
    onOpacityMapFunctionChanged(num);
    return true;
}


/**
 * @brief TransferFuncEditor::selectTransferFunctionEditorColorFunction  色関数定義を選択する。
 * @param select_trans  the selected transfer function
 * @return
 */
bool TransferFuncEditor::selectTransferFunctionEditorColorFunction(int num)
{
    if (num >= ui->colormapFunction->count()){
        return false;
    }
    ui->colormapFunction->setCurrentIndex(num);
    //Martin added
    onColorMapFunctionChanged(num);
    return true;
}

/**
 * @brief TransferFuncEditor::setCommandInstance
 * @param command the command instance to set
 */
void TransferFuncEditor::setCommandInstance(ExtCommand* command)
{
    qDebug("TransferFuncEditor::setCommandInstance START");
    blockEventHandling=true;
    extCommand->extransfuncedit=this;
    qInfo("TransferFuncEditor::TransferFuncEditor END");
    connectSignalsToSlots();
    blockEventHandling=false;
}

/**
 * @brief TransferFuncEditor::displayTransferFunction  伝達関数の表示を行う。
 * @param n_color    色関数定義番号（１〜）
 * @param n_opacity  不透明度関数定義番号（１〜）
 */
void TransferFuncEditor::displayTransferFunction()
{
    if(blockEventHandling) return;
    m_color_histogram->update();
    m_opacity_histogram->update();
    TransferFuncEditor::updateRangeView();
    blockEventHandling=false;
}


/**
 * @brief TransferFuncEditor::clearTransferFunction
 */
void TransferFuncEditor::clearTransferFunction()
{

    blockEventHandling=true;
    qDebug("clearTransferFunction function in TransferFuncEditor");
    std::vector<size_t> table( 1024, 0);

    ft=kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() );
    m_color_histogram->setTable( ft );
    ui->color_hist_min->setNum(0.0);
    ui->color_hist_max->setNum(1.0);

    ft= kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() );
    m_opacity_histogram->setTable( ft );
    ui->opacity_hist_min->setNum(0.0);
    ui->opacity_hist_max->setNum(1.0);



    ui->transfer_function_var_color->clear();
    ui->transfer_function_min_color->setValue(0.0);
    ui->transfer_function_max_color->setValue(1.0);
    ui->transfer_function_var_opacity->clear();
    ui->transfer_function_min_opacity->setValue(0.0);
    ui->transfer_function_max_opacity->setValue(1.0);
    blockEventHandling=false;
    qDebug("clearTransferFunction function in TransferFuncEditor end");
    return;
}

/**
 * @brief TransferFuncEditor::updateRangeView
 */
void TransferFuncEditor::updateRangeView(){
    qInfo("TransferFuncEditor::updateRangeView");
    int n_select_color = ui->colormapFunction->currentIndex()+1;
    int n_select_opacity =ui->opacitymapFunction->currentIndex()+1;
    char color_function_name[8] = {0x00};
    char opacity_function_name[8] = {0x00};
    sprintf(color_function_name, "C%d", n_select_color);
    sprintf(opacity_function_name, "O%d", n_select_opacity);
    char tag_c[16] = {0x00};
    char tag_o[16] = {0x00};
    // Temporary string just to prevent crash in Windows debug mode
    std::string tmp_cmf=ui->colormapFunction->currentText().toStdString();
    std::string tmp_omf=ui->opacitymapFunction->currentText().toStdString();

    sprintf(tag_c, "%s_var_c", tmp_cmf.c_str());
    sprintf(tag_o, "%s_var_o", tmp_omf.c_str());

    const NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    const NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_select_opacity);


    if (trans_color != NULL) {
        kvs::ColorMap color_map = trans_color->colorMap();
        m_color_map_palette->setColorMap( color_map );

        std::vector<size_t> table( 1024, 0);
        m_color_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
        const kvs::visclient::FrequencyTable* freq_table = extCommand->m_result.findColorFrequencyTable(std::string(color_function_name));
        if ( freq_table != NULL ) {
            m_color_histogram->setTable( *freq_table );
            m_color_histogram->setRange(ui->transfer_function_min_color->value(),ui->transfer_function_max_color->value() );
            ui->color_hist_min->setNum(requested_color_min);
            ui->color_hist_max->setNum(requested_color_max);

        }

    }

    if (trans_opacity != NULL) {
        kvs::OpacityMap opacity_map = trans_opacity->opacityMap();
        m_opacity_map_palette->setOpacityMap( opacity_map );

        std::vector<size_t> table( 1024, 0);
        m_opacity_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
        const kvs::visclient::FrequencyTable* freq_table = extCommand->m_result.findOpacityFrequencyTable(std::string(opacity_function_name));
        if ( freq_table != NULL ) {
            m_opacity_histogram->setTable( *freq_table );
            // MODIFIED BY)T.Osaki 2019.12.20
            m_opacity_histogram->setRange(ui->transfer_function_min_opacity->value(), ui->transfer_function_max_opacity->value());
            ui->opacity_hist_min->setNum(requested_opacity_min );
            ui->opacity_hist_max->setNum(requested_opacity_max );
        }
    }

    ui->range_min_color->setText(  QString::number( extCommand->m_result.m_var_range.min( tag_c )));
    ui->range_max_color->setText(  QString::number( extCommand->m_result.m_var_range.max( tag_c )));
    ui->range_min_opacity->setText(QString::number( extCommand->m_result.m_var_range.min( tag_o )));
    ui->range_max_opacity->setText(QString::number( extCommand->m_result.m_var_range.max( tag_o )));

    if(ui->lock_opacity_range->isChecked()){
        ui->transfer_function_max_opacity->setValue(ui->range_max_opacity->text().toDouble());
        ui->transfer_function_min_opacity->setValue(ui->range_min_opacity->text().toDouble());
        requested_opacity_max=ui->range_max_opacity->text().toDouble();
                 requested_opacity_min=ui->range_min_opacity->text().toDouble();
    }
    if (ui->lock_color_range->isChecked()){
        ui->transfer_function_max_color->setValue(ui->range_max_color->text().toDouble());
        ui->transfer_function_min_color->setValue(ui->range_min_color->text().toDouble());
        requested_color_max=ui->range_max_color->text().toDouble();
        requested_color_min=ui->range_min_color->text().toDouble();
        extCommand->m_parameter.m_parameter_extend_transfer_function.m_color_transfer_function.at(n_select_color-1).m_color_variable_min =requested_color_min;
        extCommand->m_parameter.m_parameter_extend_transfer_function.m_color_transfer_function.at(n_select_color-1).m_color_variable_max=requested_color_max;
        extCommand->m_screen->update();
    }
    m_color_histogram->update();
    m_opacity_histogram->update();

    return;
}

/**
 * @brief TransferFuncEditor::updateRangeEdit
 */
void TransferFuncEditor::updateRangeEdit()
{
    qInfo("TransferFuncEditor::updateRangeEdit");
    onColorMapFunctionChanged(ui->colormapFunction->currentIndex());
    onOpacityMapFunctionChanged(ui->opacitymapFunction->currentIndex());
    return;
}

/**
 * @brief
 *
 * @return kvs::visclient::ExtendedTransferFunctionMessage
 */
kvs::visclient::ExtendedTransferFunctionMessage& TransferFuncEditor::doc()
{
    return m_doc;
}

/**
 * @brief
 *
 * @return const kvs::visclient::ExtendedTransferFunctionMessage
 */
const kvs::visclient::ExtendedTransferFunctionMessage& TransferFuncEditor::doc() const
{
    return m_doc;
}


/**
 * @brief TransferFuncEditor::on_cfuncButton_clicked
 */
void TransferFuncEditor::onCfuncClicked()
{
    cFuncEditor.initalize( FunctionListEditor::COLOR_FUNCTION_DIALOG, this->m_doc, ui->colormapFunction->currentIndex());
//    cFuncEditor.show();
    // Martin changed - show dialog as modal - then update
    cFuncEditor.exec();
    int index= cFuncEditor.getSelectedFunctionIndex();
    selectTransferFunctionEditorColorFunction(index);
}

/**
 * @brief TransferFuncEditor::on_colorButton_clicked
 */
void TransferFuncEditor::onColorButtonClicked()
{
    qInfo("Showing CED");
    //ADD BY)T.Osaki 2020.02.03
    int n_select_color = ui->colormapFunction->currentIndex()+1;
    const NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(n_select_color);
    kvs::ColorMap color_map = trans_color->colorMap();
    this->colorEditor.m_color_map_palette->setColorMap(color_map);
    this->colorEditor.m_color_map_palette->m_color_map_initial = color_map;

    int choice=this->colorEditor.exec();
    if (choice==QDialog::Accepted){
        const kvs::ColorMap cmap = this->colorEditor.m_color_map_palette->colorMap();
        m_color_map_palette->setColorMap(cmap);
        int n_select_color = ui->colormapFunction->currentIndex()+1;
        NamedTransferFunctionParameter *tc = this->m_doc.getColorTransferFunction(n_select_color);
        tc->setColorMap(cmap);
    }
    this->colorEditor.m_color_map_palette->m_cundoStack->clear();
}


/**
 * @brief TransferFuncEditor::on_color_function_synth_currentTextChanged
 * @param arg1
 */
void TransferFuncEditor::onColorFunctionSynthEdited(const QString &arg1)
{
    extCommand->m_is_import_transfer_function_parameter = true;
    m_doc.m_color_transfer_function_synthesis = ui->color_function_synth->text().toStdString();

}




/**
 * @brief TransferFuncEditor::on_colormap_function_currentIndexChanged
 * @param index
 */
void TransferFuncEditor::onColorMapFunctionChanged(int index)
{
    qInfo("TransferFuncEditor::onColorMapFunctionChanged, %d", index);
    const NamedTransferFunctionParameter *trans_color = this->m_doc.getColorTransferFunction(index+1);
    if (trans_color != NULL) {
        qInfo(" trans_color not null");
        kvs::ColorMap color_map = trans_color->colorMap();
        m_color_map_palette->setColorMap( color_map );
        const kvs::visclient::FrequencyTable* freq_table = extCommand->m_result.findColorFrequencyTable(trans_color->m_name);
        if ( freq_table != NULL ) {
            m_color_histogram->setTable( *freq_table );
            ui->color_hist_min->setNum(requested_color_min);
            ui->color_hist_max->setNum(requested_color_max);
        }

        ui->transfer_function_var_color->blockSignals(true);
        ui->transfer_function_min_color->blockSignals(true);
        ui->transfer_function_max_color->blockSignals(true);

        ui->transfer_function_var_color->setText(  trans_color->m_color_variable.c_str());
        ui->transfer_function_min_color->setValue( trans_color->m_color_variable_min );
        ui->transfer_function_max_color->setValue( trans_color->m_color_variable_max );

        ui->transfer_function_var_color->blockSignals(false);
        ui->transfer_function_min_color->blockSignals(false);
        ui->transfer_function_max_color->blockSignals(false);
    }
}

/**
 * @brief TransferFuncEditor::on_exportButton_clicked
 */
void TransferFuncEditor::onExportButtonClicked()
{
    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Export Current Settings to Parameter File"),
                                         ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )")  );
    if( fileName.right(4) != ".tfe" && fileName.right(4) != ".TFE" && fileName.right(3) != ".tf" && fileName.right(3) != ".TF")
    {
        fileName += ".tfe";
    }
    this->exportFile(fileName.toStdString(), false);
}

/**
 * @brief TransferFuncEditor::on_importButton_clicked
 */
void TransferFuncEditor::onImportButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Transfer Function File"),
                                                    ".", tr("Transfer Function Files (*.tfe *.TFE *.tf *.TF )") );
    this->importFile(fileName.toStdString());
}


/**
 * @brief TransferFuncEditor::on_num_transfer_valueChanged
 * @param val
 */
void TransferFuncEditor::onNumTransferChanged(int val)
{
    int num_transfer_function =val;
    m_doc.m_transfer_function_number = num_transfer_function;
    this->populateColorFunctionLists(val);
    this->populateOpacityFunctionLists(val);

    int n;
    int current_size;
    char name[8];
    // 色関数を追加、削除する。
    current_size = this->m_doc.m_color_transfer_function.size();
    if (current_size <= num_transfer_function) {
        // 追加を行う
        for (n=current_size+1; n<=num_transfer_function; n++) {
            sprintf(name, "C%d", n);
            this->m_doc.addColorTransferFunction(name, "q1");
        }
    }
    else {
        // 削除を行う
        for (n=num_transfer_function+1; n<=current_size; n++) {
            sprintf(name, "C%d", n);
            this->m_doc.removeColorTransferFunction(name);
        }
    }

    // 不透明度関数を追加、削除する。
    current_size = this->m_doc.m_opacity_transfer_function.size();
    if (current_size <= num_transfer_function) {
        // 追加を行う
        for (n=current_size+1; n<=num_transfer_function; n++) {
            sprintf(name, "O%d", n);
            this->m_doc.addOpacityTransferFunction(name, "q1");
        }
    }
    else {
        // 削除を行う
        for (n=num_transfer_function+1; n<=current_size; n++) {
            sprintf(name, "O%d", n);
            this->m_doc.removeOpacityTransferFunction(name);
        }
    }

    return;
}


/**
 * @brief TransferFuncEditor::on_ofuncButton_clicked
 */
void TransferFuncEditor::onOfuncClicked()
{
    oFuncEditor.initalize( FunctionListEditor::OPACITY_FUNCTION_DIALOG, this->m_doc, ui->opacitymapFunction->currentIndex());
//    oFuncEditor.show();
    // Martin changed - show dialog as modal - then update
    oFuncEditor.exec();
    int index= oFuncEditor.getSelectedFunctionIndex();
    selectTransferFunctionEditorOpacityFunction(index);


}


/**
 * @brief TransferFuncEditor::on_opacity_function_synth_currentTextChanged
 * @param arg1
 */
void TransferFuncEditor::onOpacityFunctionSynthEdited(const QString &arg1)
{
    extCommand->m_is_import_transfer_function_parameter = true;
    m_doc.m_opacity_transfer_function_synthesis =ui->opacity_function_synth->text().toStdString();
}



/**
 * @brief TransferFuncEditor::on_opacityButton_clicked
 */
void TransferFuncEditor::onOpacityButtonClicked()
{
    qInfo("Showing OED");
    //ADD BY)T.Osaki 2020.02.04
    int n_select_opacity = ui->opacitymapFunction->currentIndex()+1;
    const NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(n_select_opacity);
    kvs::OpacityMap opacity_map = trans_opacity->opacityMap();
    this->opacityEditor.m_opacity_map_palette->setOpacityMap(opacity_map);
    this->opacityEditor.m_opacity_map_palette->m_opacity_map_initial = opacity_map;

    int choice=this->opacityEditor.exec();
    if (choice==QDialog::Accepted){
        const kvs::OpacityMap omap = this->opacityEditor.m_opacity_map_palette->opacityMap();
        m_opacity_map_palette->setOpacityMap(omap);
        int n_select_opacity = ui->opacitymapFunction->currentIndex()+1;
        NamedTransferFunctionParameter *to = this->m_doc.getOpacityTransferFunction(n_select_opacity);
        to->setOpacityMap(omap);
    }
    this->opacityEditor.m_opacity_map_palette->m_cundoStack->clear();
}


/**
 * @brief TransferFuncEditor::on_opacitymap_function_currentIndexChanged
 * @param index
 */
void TransferFuncEditor::onOpacityMapFunctionChanged(int index)
{
    qInfo("TransferFuncEditor::onOpacityMapFunctionChanged %d",index);
    const NamedTransferFunctionParameter *trans_opacity = this->m_doc.getOpacityTransferFunction(index+1);
    if (trans_opacity != NULL) {
        qInfo(" trans_opacity not null");
        kvs::OpacityMap opacity_map = trans_opacity->opacityMap();
        m_opacity_map_palette->setOpacityMap( opacity_map );
        const kvs::visclient::FrequencyTable* freq_table = extCommand->m_result.findOpacityFrequencyTable(trans_opacity->m_name);
        if ( freq_table != NULL ) {
            m_opacity_histogram->setTable( *freq_table );
            ui->opacity_hist_min->setNum(requested_opacity_min );
            ui->opacity_hist_max->setNum(requested_opacity_max );
        }

        ui->transfer_function_var_opacity->blockSignals(true);
        ui->transfer_function_min_opacity->blockSignals(true);
        ui->transfer_function_max_opacity->blockSignals(true);

        ui->transfer_function_var_opacity->setText( trans_opacity->m_opacity_variable.c_str() );
        ui->transfer_function_min_opacity->setValue( trans_opacity->m_opacity_variable_min );
        ui->transfer_function_max_opacity->setValue( trans_opacity->m_opacity_variable_max );

        ui->transfer_function_var_opacity->blockSignals(false);
        ui->transfer_function_min_opacity->blockSignals(false);
        ui->transfer_function_max_opacity->blockSignals(false);
    }
}




/**
 * @brief TransferFuncEditor::on_resolution_valueChanged
 * @param val
 */
void TransferFuncEditor::onResolutionChanged(int val)
{
    m_resolution = val;
    //MODIFIED BY)T.Osaki 2020.02.05
//    size_t resolution = val;
//    m_doc.m_extend_transfer_function_resolution = resolution;

//    for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
//        m_doc.m_color_transfer_function[n].setResolution( resolution );
//    }

//    for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
//        m_doc.m_opacity_transfer_function[n].setResolution( resolution );
//    }

//    this->displayTransferFunction();
//    this->update();
}

//ADD BY)T.Osaki 2020.02.05
void TransferFuncEditor::onResolutionChangeButtonClicked()
{
    std::cout << "m_doc.m_extend_tarnsfer_function_resolution:" << m_doc.m_extend_transfer_function_resolution << std::endl;
    std::cout << "m_resolution:" << m_resolution << std::endl;
        m_doc.m_extend_transfer_function_resolution = m_resolution;

        for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
            m_doc.m_color_transfer_function[n].setResolution( m_resolution );
        }

        for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
            m_doc.m_opacity_transfer_function[n].setResolution( m_resolution );
        }

        this->displayTransferFunction();
        this->update();
}

/**
 * @brief TransferFuncEditor::on_resetButton_clicked
 */
void TransferFuncEditor::onResetButtonClicked()
{
    size_t resolution = m_doc.m_extend_transfer_function_resolution;

    for ( size_t n = 0; n < m_doc.m_color_transfer_function.size(); n++ ) {
        kvs::TransferFunction* ptf = &m_doc.m_color_transfer_function[n];
        *ptf = kvs::TransferFunction( resolution );

    }

    for ( size_t n = 0; n < m_doc.m_opacity_transfer_function.size(); n++ ) {
        kvs::TransferFunction* ptf = &m_doc.m_opacity_transfer_function[n];
        *ptf = kvs::TransferFunction( resolution );
    }

    this->displayTransferFunction();
}

/**
 * @brief TransferFuncEditor::on_transfer_function_var_color_textEdited
 * @param arg1
 */
void TransferFuncEditor::onTransferFunctionVarColorEdited(const QString &arg1)
{
    extCommand->m_is_import_transfer_function_parameter = true;
    std::string fName=ui->colormapFunction->currentText().toStdString();
    std::string variable= ui->transfer_function_var_color->text().toStdString();
    // 既存かチェックする
    bool exists = (this->m_doc.getColorTransferFunction(fName) != NULL);
    // 更新：存在しなければ追加
    m_doc.setColorTransferFunction(fName, variable);
    // 既存の場合は最小値、最大値を設定する。（新規の場合は、[0]のコピーを作成済み
    if (exists) {
        // 最小値、最大値
        this->m_doc.setColorTransferRange(fName,
                                          ui->transfer_function_min_color->value(),
                                          ui->transfer_function_max_color->value());
    }
}

/**
 * @brief TransferFuncEditor::on_transfer_function_var_opacity_textEdited
 * @param arg1
 */
void TransferFuncEditor::onTransferFunctionVarOpacityEdited(const QString &arg1)
{
    extCommand->m_is_import_transfer_function_parameter = true;
    std::string fName=ui->opacitymapFunction->currentText().toStdString();
    std::string variable= ui->transfer_function_var_opacity->text().toStdString();
    // 既存かチェックする
    bool exists = (this->m_doc.getOpacityTransferFunction(fName) != NULL);
    // 更新：存在しなければ追加
    m_doc.setOpacityTransferFunction(fName, variable);
    // 最小値、最大値
    if (exists) {
        // MODIFIED BY)T.Osaki 2019.12.20
        m_doc.setOpacityTransferRange(fName, ui->transfer_function_min_opacity->value(),
                                      ui->transfer_function_max_opacity->value());
    }
}

/**
 * @brief TransferFuncEditor::on_transfer_function_min_color_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFminColorChanged(double arg1)
{
    QString funcname=ui->colormapFunction->currentText();
    qInfo()<<"TransferFuncEditor::onTFminColorChanged "<<funcname<<
          ui->transfer_function_min_color->value()<<"," <<
          ui->transfer_function_max_color->value();
    this->m_doc.setColorTransferRange(ui->colormapFunction->currentText().toStdString(),
                                      ui->transfer_function_min_color->value(),
                                      ui->transfer_function_max_color->value());
}

/**
 * @brief TransferFuncEditor::on_transfer_function_max_color_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFmaxColorChanged(double arg1)
{

    QString funcname=ui->colormapFunction->currentText();
    qInfo()<<"TransferFuncEditor::onTFmaxColorChanged "<<funcname<<
          ui->transfer_function_min_color->value()<<"," <<
          ui->transfer_function_max_color->value();
    this->m_doc.setColorTransferRange(ui->colormapFunction->currentText().toStdString(),
                                      ui->transfer_function_min_color->value(),
                                      ui->transfer_function_max_color->value());
}

/**
 * @brief TransferFuncEditor::on_transfer_function_min_opacity_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFminOpacityChanged(double arg1)
{
    QString funcname=ui->opacitymapFunction->currentText();
    qInfo()<<"TransferFuncEditor::onTFminOpacityChanged "<<funcname<<
          ui->transfer_function_min_opacity->value()<<"," <<
          ui->transfer_function_max_opacity->value();
    this->m_doc.setOpacityTransferRange(ui->opacitymapFunction->currentText().toStdString(),
                                        ui->transfer_function_min_opacity->value(),
                                        ui->transfer_function_max_opacity->value());
}

/**
 * @brief TransferFuncEditor::on_transfer_function_max_opacity_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFmaxOpacityChanged(double arg1)
{

    QString funcname=ui->opacitymapFunction->currentText();
    qInfo()<<"TransferFuncEditor::onTFmaxOpacityChanged "<<funcname<<
          ui->transfer_function_min_opacity->value()<<"," <<
          ui->transfer_function_max_opacity->value();
    this->m_doc.setOpacityTransferRange(ui->opacitymapFunction->currentText().toStdString(),
                                        ui->transfer_function_min_opacity->value(),
                                        ui->transfer_function_max_opacity->value());
}






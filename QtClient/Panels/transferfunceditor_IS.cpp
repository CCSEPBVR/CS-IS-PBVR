#include "transferfunceditor_IS.h"
#include "ui_transferfunceditor.h"

#include <QFileDialog>
#include <QDebug>
#include "Client/ParamInfo.h"
#include "QGlue/extCommand.h"
#include "QGlue/renderarea.h"
#include <fstream>
#include <QGlue/typedSignalConnect.h>

bool blockEventHandling=false;
kvs::visclient::FrequencyTable ft;

TransferFuncEditor::TransferFuncEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferFuncEditor)
{
    blockEventHandling=true;
    ui->setupUi(this);
    m_opacity_map_palette = ui->qscr_opacity_map;
    m_opacity_map_palette->setEditable(false);
    m_opacity_map_palette->setOpacityMap(m_doc.transferFunction[0].opacityMap());

    m_color_map_palette =  ui->qscr_color_map;
    m_color_map_palette->setEditable(false,0);
    m_color_map_palette->setColorMap(m_doc.transferFunction[0].colorMap());

    m_color_histogram = ui->qscr_color_hist;
    m_color_histogram->setGraphColor(kvs::RGBAColor( 128, 128,128, 1.0f ));
    m_opacity_histogram = ui->qscr_opacity_hist;
    m_opacity_histogram->setGraphColor(kvs::RGBAColor( 128, 128,128, 1.0f ));
    m_doc_initial=m_doc;

    ui->operatingModeStack->setCurrentIndex(1);

    this->populateTransferFuncList(m_doc.transferFunction.size());

    blockEventHandling=false;
}

TransferFuncEditor::~TransferFuncEditor()
{
    this->colorEditor.close();
    this->opacityEditor.close();
    delete ui;
}

void TransferFuncEditor::importExtendTransferFunction( ParamExTransFunc doc){
    m_doc_initial = doc;
    m_doc = doc;
    extCommand->isImportTransferFunctionParam = true;

    displayTransferFunction();
    this->update();
    std::cerr << "TransferFunction parameters are imported from TFS " << std::endl;

}

/**
 * @brief TransferFuncEditor::apply
 */
void TransferFuncEditor::apply()
{
    extCommand->param.paramExTransFunc = m_doc;
    extCommand->screen->update();
}



/**
 * @brief TransferFuncEditor::ConnectSignalsToSlots,
 *        Connects all UI Signals to Slots
 */
void TransferFuncEditor::connectSignalsToSlots()
{

    connect_T(ui->tfResolution,      QSpinBox,  valueChanged,int,  &TransferFuncEditor::onResolutionChanged);

    connect_T(ui->transfer_function_min_color,    QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFminColorChanged);
    connect_T(ui->transfer_function_max_color,    QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFmaxColorChanged);
    connect_T(ui->transfer_function_min_opacity,  QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFminOpacityChanged);
    connect_T(ui->transfer_function_max_opacity,  QDoubleSpinBox,  valueChanged,double, &TransferFuncEditor::onTFmaxOpacityChanged);

    connect_T(ui->tfSelection,  QComboBox, currentIndexChanged, int, &TransferFuncEditor::onTransferFunctionSelected);

    connect_T(ui->colorButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::onColorButtonClicked);
    connect_T(ui->opacityButton,   QAbstractButton, clicked, bool, &TransferFuncEditor::onOpacityButtonClicked);
    connect_T(ui->importButton,    QAbstractButton, clicked, bool, &TransferFuncEditor::onImportButtonClicked);
    connect_T(ui->exportButton,    QAbstractButton, clicked, bool, &TransferFuncEditor::onExportButtonClicked);
    connect_T(ui->resetButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::onResetButtonClicked);
    connect_T(ui->applyButton,     QAbstractButton, clicked, bool, &TransferFuncEditor::apply);

    connect_T(ui->tfColorEquation,  QLineEdit, textEdited, const QString&, &TransferFuncEditor::onTransferFunctionVarColorEdited);
    connect_T(ui->tfOpacityEquation,QLineEdit, textEdited, const QString&, &TransferFuncEditor::onTransferFunctionVarColorEdited);
    connect_T(ui->tfSythesizer,     QLineEdit, textEdited, const QString&, &TransferFuncEditor::onFunctionSynthEdited);

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

    ofs << "TF_RESOLUTION=" << m_doc.ext_transferFunctionResolution << std::endl;
    ofs << "TF_SYNTH=" << m_doc.ext_transferFunctionSynthesis << std::endl;
    for ( size_t n = 0; n < m_doc.transferFunction.size(); n++ )
    {
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        ofs << tag_base << "VAR_C=" << m_doc.transferFunction[n].ColorVar << std::endl;
        ofs << tag_base << "MIN_C=" << m_doc.transferFunction[n].ColorVarMin << std::endl;
        ofs << tag_base << "MAX_C=" << m_doc.transferFunction[n].ColorVarMax << std::endl;
        ofs << tag_base << "VAR_O=" << m_doc.transferFunction[n].OpacityVar << std::endl;
        ofs << tag_base << "MIN_O=" << m_doc.transferFunction[n].OpacityVarMin << std::endl;
        ofs << tag_base << "MAX_O=" << m_doc.transferFunction[n].OpacityVarMax << std::endl;

        kvs::ColorMap::Table color_table = m_doc.transferFunction[n].colorMap().table();
        kvs::OpacityMap::Table opacity_table = m_doc.transferFunction[n].opacityMap().table();

        ofs << tag_base << "TABLE_C=";
        for ( size_t i = 0; i < color_table.size(); i++ )
        {
            ofs << static_cast<int>( color_table.at( i ) ) << ",";
        }
        ofs << std::endl;

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
    ParamExTransFunc doc;
    ParamInfo param;
    bool stat;

    stat = param.loadIN( fname );

    if ( !stat ) return;

    doc.ext_transferFunctionResolution = param.getInt( "tf_resolution" );
    doc.ext_transferFunctionSynthesis = param.getString( "tf_synthesis" );

    for ( size_t n = 0; n < doc.transferFunction.size(); n++ )
    {
        const size_t resolution = doc.ext_transferFunctionResolution;
        std::stringstream ss;
        ss << "TF_NAME" << n + 1 << "_";

        const std::string tag_base = ss.str();
        doc.transferFunction[n].setResolution( resolution );
        doc.transferFunction[n].ColorVar      = param.getString( tag_base + "VAR_C" );
        doc.transferFunction[n].ColorVarMin   = param.getFloat( tag_base + "MIN_C" );
        doc.transferFunction[n].ColorVarMax   = param.getFloat( tag_base + "MAX_C" );
        doc.transferFunction[n].OpacityVar    = param.getString( tag_base + "VAR_O" );
        doc.transferFunction[n].OpacityVarMin = param.getFloat( tag_base + "MIN_O" );
        doc.transferFunction[n].OpacityVarMax = param.getFloat( tag_base + "MAX_O" );

        std::string s_color = param.getString( tag_base + "TABLE_C" );
        std::string s_opacity = param.getString( tag_base + "TABLE_O" );

        std::replace( s_color.begin(), s_color.end(), ',', ' ' );
        std::replace( s_opacity.begin(), s_opacity.end(), ',', ' ' );

        std::stringstream ss_color( s_color );
        std::stringstream ss_opacity( s_opacity );

        kvs::ColorMap::Table color_table( resolution * 3 );
        kvs::OpacityMap::Table opacity_table( resolution );

        for ( size_t i = 0; i < resolution; i++ )
        {
            for ( size_t c = 0; c < 3; c++ )
            {
                int color_e;
                ss_color >> color_e;
                color_table.at( i * 3 + c ) = color_e;
            }
        }

        for ( size_t i = 0; i < resolution; i++ )
        {
            float opacity;
            ss_opacity >> opacity;
            opacity_table.at( i ) = opacity;
        }

        kvs::ColorMap color_map( color_table );
        kvs::OpacityMap opacity_map( opacity_table );
        kvs::TransferFunction tf( color_map, opacity_map );
        kvs::TransferFunction& tt = doc.transferFunction[n];
        tt = tf;
    }

    m_doc_initial = doc;
    m_doc = doc;
    extCommand->isImportTransferFunctionParam = true;
    displayTransferFunction();
    //    display_transferFunction( m_current_tf );
    this->update();
    //  this->apply();
    std::cerr << "TransferFunction parameters are imported from " << fname << std::endl;
}



/**
 * @brief TransferFuncEditor::populateColorFunctionLists
 *        Popuplates the color function list with C1 .. Cn
 * @param n
 */
void TransferFuncEditor::populateTransferFuncList(int n){

    QStringList itemList;
    for (int i=1;i <=n;i++ ){
        itemList.append(QString("t%1").arg(i));
    }
    ui->tfSelection->blockSignals(true);
    ui->tfSelection->clear();
    ui->tfSelection->addItems(itemList);
    ui->tfSelection->blockSignals(false);
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

    int n = ui->tfSelection->currentIndex();
    kvs::ColorMap color_map = m_doc.transferFunction[n].colorMap();
    kvs::OpacityMap opacity_map = m_doc.transferFunction[n].opacityMap();

    m_color_map_palette->setColorMap( color_map );
    m_opacity_map_palette->setOpacityMap( opacity_map );

    if ( extCommand->result.c_bins.size() > n )
    {
        m_color_histogram->setTable( extCommand->result.c_bins[n] );
    }
    if ( extCommand->result.o_bins.size() > n )
    {
        m_opacity_histogram->setTable( extCommand->result.o_bins[n] );
    }

    ui->tfColorEquation->setText( m_doc.transferFunction[n].ColorVar.c_str() );
    ui->tfOpacityEquation->setText( m_doc.transferFunction[n].OpacityVar.c_str() );
    ui->transfer_function_min_color->setValue( m_doc.transferFunction[n].ColorVarMin );
    ui->transfer_function_max_color->setValue( m_doc.transferFunction[n].ColorVarMax );
    ui->transfer_function_min_opacity->setValue( m_doc.transferFunction[n].OpacityVarMin );
    ui->transfer_function_max_opacity->setValue( m_doc.transferFunction[n].OpacityVarMax );

    ui->tfResolution->setValue( m_doc.ext_transferFunctionResolution );
    ui->tfSythesizer->setText( m_doc.ext_transferFunctionSynthesis.c_str() );
    m_color_histogram->update();
    m_opacity_histogram->update();
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
    ft= kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() );
    m_opacity_histogram->setTable( ft );

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
    int n_select_color = ui->colormapFunction->currentIndex()+1;
    int n_select_opacity =ui->opacitymapFunction->currentIndex()+1;
    char color_function_name[8] = {0x00};
    char opacity_function_name[8] = {0x00};
    sprintf(color_function_name, "C%d", n_select_color);
    sprintf(opacity_function_name, "O%d", n_select_opacity);
    char tag_c[16] = {0x00};
    char tag_o[16] = {0x00};
    sprintf(tag_c, "%s_var_c", ui->colormapFunction->currentText().toStdString().c_str());
    sprintf(tag_o, "%s_var_o", ui->opacitymapFunction->currentText().toStdString().c_str());

    const NamedTransferFunctionParameter *trans_color = this->m_doc.getTransferFunction(n_select_color);


    if (trans_color != NULL) {
        kvs::ColorMap color_map = trans_color->colorMap();
        m_color_map_palette->setColorMap( color_map );

        std::vector<size_t> table( 1024, 0);
        m_color_histogram->setTable( kvs::visclient::FrequencyTable( 0.0, 1.0, table.size(), table.data() ) );
        const kvs::visclient::FrequencyTable* freq_table = extCommand->result.findColorFrequencyTable(std::string(color_function_name));
        if ( freq_table != NULL ) {
            m_color_histogram->setTable( *freq_table );
        }
    }

    ui->range_min_color->setText(  QString::number( extCommand->result.varRange.min( tag_c )));
    ui->range_max_color->setText(  QString::number( extCommand->result.varRange.max( tag_c )));
    ui->range_min_opacity->setText(QString::number( extCommand->result.varRange.min( tag_o )));
    ui->range_max_opacity->setText(QString::number( extCommand->result.varRange.max( tag_o )));

    m_color_histogram->update();
    m_opacity_histogram->update();

    return;
}

/**
 * @brief TransferFuncEditor::updateRangeEdit
 */
void TransferFuncEditor::updateRangeEdit()
{
    onTransferFunctionSelected(ui->colormapFunction->currentIndex());
    return;
}

/**
 * @brief
 *
 * @return kvs::visclient::ExtendedTransferFunctionMessage
 */
kvs::visclient::ParamExTransFunc& TransferFuncEditor::doc()
{
    return m_doc;
}

/**
 * @brief
 *
 * @return const kvs::visclient::ExtendedTransferFunctionMessage
 */
const kvs::visclient::ParamExTransFunc& TransferFuncEditor::doc() const
{
    return m_doc;
}



/**
 * @brief TransferFuncEditor::on_colorButton_clicked
 */
void TransferFuncEditor::onColorButtonClicked()
{
    qInfo("Showing CED");
    int choice=this->colorEditor.exec();
    if (choice==QDialog::Accepted){
        const kvs::ColorMap cmap = this->colorEditor.m_color_map_palette->colorMap();
        m_color_map_palette->setColorMap(cmap);
        int n_select_color = ui->tfSelection->currentIndex()+1;
        NamedTransferFunctionParameter *tc = this->m_doc.getTransferFunction(n_select_color);
        tc->setColorMap(cmap);
    }
}


/**
 * @brief TransferFuncEditor::on_color_function_synth_currentTextChanged
 * @param arg1
 */
void TransferFuncEditor::onFunctionSynthEdited(const QString &arg1)
{
    extCommand->isImportTransferFunctionParam = true;
}




/**
 * @brief TransferFuncEditor::on_colormap_function_currentIndexChanged
 * @param index
 */
void TransferFuncEditor::onTransferFunctionSelected(int index)
{
    qInfo("TransferFuncEditor::on_colormap_function_currentIndexChanged, %d", index);
    const NamedTransferFunctionParameter *trans_func = this->m_doc.getTransferFunction(index+1);
    if (trans_func != NULL) {
        qInfo(" trans_func not null");
        kvs::ColorMap color_map = trans_func->colorMap();
        m_color_map_palette->setColorMap( color_map );
        const kvs::visclient::FrequencyTable* freq_table_c = extCommand->result.findColorFrequencyTable(trans_func->Name);
        if ( freq_table_c != NULL ) {
            m_color_histogram->setTable( *freq_table_c );
        }

        kvs::OpacityMap opacity_map = trans_func->opacityMap();
        m_opacity_map_palette->setOpacityMap( opacity_map );
        const kvs::visclient::FrequencyTable* freq_table_o = extCommand->result.findOpacityFrequencyTable(trans_func->Name);
        if ( freq_table_o != NULL ) {
            m_color_histogram->setTable( *freq_table_o );
        }

        ui->tfColorEquation->setText(trans_func->ColorVar.c_str());
        ui->transfer_function_min_color->setValue( trans_func->ColorVarMin );
        ui->transfer_function_max_color->setValue( trans_func->ColorVarMax );

        ui->tfOpacityEquation->setText(trans_func->OpacityVar.c_str());
        ui->transfer_function_min_opacity->setValue( trans_func->OpacityVarMin );
        ui->transfer_function_max_opacity->setValue( trans_func->OpacityVarMax );
    }
//    updateParamExTransFuncDoc();
}

/**
 * @brief TransferFuncEditor::on_exportButton_clicked
 */
void TransferFuncEditor::onExportButtonClicked()
{
    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Export Current Settings to Parameter File"),
                                         ".", tr("Parameter Files (*.tfe *.TFE)")  );
    this->exportFile(fileName.toStdString(), false);
}

/**
 * @brief TransferFuncEditor::on_importButton_clicked
 */
void TransferFuncEditor::onImportButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Transfer Function File"),
                                                    ".", tr("Transfer Function Files (*.tfe *.TFE)") );
    this->importFile(fileName.toStdString());
}


/**
 * @brief TransferFuncEditor::on_num_transfer_valueChanged
 * @param val
 */
void TransferFuncEditor::onNumTransferChanged(int val)
{
    int num_transfer_function =val;
    this->populateTransferFuncList(val);
    //    this->populateOpacityFunctionLists(val);

    int n;
    int current_size;
    char name[8];
    // 色関数を追加、削除する。
    current_size = this->m_doc.transferFunction.size();

    return;
}




/**
 * @brief TransferFuncEditor::on_opacity_function_synth_currentTextChanged
 * @param arg1
 */
void TransferFuncEditor::onOpacityFunctionSynthEdited(const QString &arg1)
{
    extCommand->isImportTransferFunctionParam = true;
}



/**
 * @brief TransferFuncEditor::on_opacityButton_clicked
 */
void TransferFuncEditor::onOpacityButtonClicked()
{
    qInfo("Showing OED");
    int choice=this->opacityEditor.exec();
    if (choice==QDialog::Accepted){
        const kvs::OpacityMap omap = this->opacityEditor.m_opacity_map_palette->opacityMap();
        m_opacity_map_palette->setOpacityMap(omap);
        int n_select_opacity = ui->opacitymapFunction->currentIndex()+1;
    }
}


/**
 * @brief TransferFuncEditor::on_resolution_valueChanged
 * @param val
 */
void TransferFuncEditor::onResolutionChanged(int val)
{
    size_t resolution = val;
    m_doc.ext_transferFunctionResolution = resolution;

    for ( size_t n = 0; n < m_doc.transferFunction.size(); n++ ) {
        m_doc.transferFunction[n].setResolution( resolution );
    }


    this->displayTransferFunction();
    this->update();
}

/**
 * @brief TransferFuncEditor::on_resetButton_clicked
 */
void TransferFuncEditor::onResetButtonClicked()
{
    size_t resolution = m_doc.ext_transferFunctionResolution;

    for ( size_t n = 0; n < m_doc.transferFunction.size(); n++ ) {
        kvs::TransferFunction* ptf = &m_doc.transferFunction[n];
        *ptf = kvs::TransferFunction( resolution );

    }

    this->displayTransferFunction();
}
void TransferFuncEditor::updateParamExTransFuncDoc()
{
    int n = ui->tfSelection->currentIndex();

    m_doc.transferFunction[n].ColorVar      = ui->tfColorEquation->text().toStdString();
    m_doc.transferFunction[n].OpacityVar    = ui->tfOpacityEquation->text().toStdString();
    m_doc.transferFunction[n].ColorVarMin   = ui->transfer_function_min_color->value();
    m_doc.transferFunction[n].ColorVarMax   = ui->transfer_function_max_color->value();
    m_doc.transferFunction[n].OpacityVarMin = ui->transfer_function_min_opacity->value();
    m_doc.transferFunction[n].OpacityVarMax = ui->transfer_function_max_opacity->value();
    m_doc.ext_transferFunctionSynthesis     = ui->tfSythesizer->text().toStdString();
    extCommand->isImportTransferFunctionParam = true;
}
/**
 * @brief TransferFuncEditor::on_transfer_function_var_color_textEdited
 * @param arg1
 */
void TransferFuncEditor::onTransferFunctionVarColorEdited(const QString &arg1)
{
    updateParamExTransFuncDoc();
}

/**
 * @brief TransferFuncEditor::on_transfer_function_var_opacity_textEdited
 * @param arg1
 */
void TransferFuncEditor::onTransferFunctionVarOpacityEdited(const QString &arg1)
{
    updateParamExTransFuncDoc();
}

/**
 * @brief TransferFuncEditor::on_transfer_function_min_color_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFminColorChanged(double arg1)
{
    updateParamExTransFuncDoc();
}

/**
 * @brief TransferFuncEditor::on_transfer_function_max_color_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFmaxColorChanged(double arg1)
{
    updateParamExTransFuncDoc();
}

/**
 * @brief TransferFuncEditor::on_transfer_function_min_opacity_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFminOpacityChanged(double arg1)
{
    updateParamExTransFuncDoc();
}

/**
 * @brief TransferFuncEditor::on_transfer_function_max_opacity_valueChanged
 * @param arg1
 */
void TransferFuncEditor::onTFmaxOpacityChanged(double arg1)
{
    updateParamExTransFuncDoc();
}






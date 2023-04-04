#include "particlepanel.h"
#include "ui_particlepanel.h"

#include <QFileDialog>
#include <QGlue/extCommand.h>
#include <QGlue/typedSignalConnect.h>
#define  ARRAY_TOVECTOR(X) std::vector<int> X(std::begin(X), std::end(X))

ParticlePanel::ParticlePanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ParticlePanel)
{
    ui->setupUi(this);
    this->hide();
//    ui->exportButton->setDisabled(true);

    m_particle_value= initializeDisplayValues( );
    m_keep_initial_value= initializeKeepInitial( );
    m_keep_final_value= initializeKeepFinal( );
    m_particle_value[0]->setChecked(true);
    //    MOD BY)T.Osaki 2020.03.16
    m_keep_initial_value[0]->setChecked(false);
    m_keep_final_value[0]->setChecked(false);
    m_particle_value[0]->setEnabled(true);
    m_polygon_color_edit = new ColorEditDialog(this);
    //Osaki QTISPBVR update(2020/11/16) IS版ではServer側のkeep_initialとfinalは操作できないはずなので非活性にしました。
#ifdef IS_MODE
    m_keep_initial_value[0]->setEnabled(false);
    m_keep_final_value[0]->setEnabled(false);
#else
    m_keep_initial_value[0]->setEnabled(true);
    m_keep_final_value[0]->setEnabled(true);
#endif
    for ( int i = 0; i < 11; i++ )
    {
        if ( m_file_path_strings[i].empty() != false )
        {
            m_file_path_strings[i].clear();
        }
        m_file_path_changed[i] = false;
    }
    m_file_path_strings[0] = "server";
    m_total_item = 0;
    m_do_export = false;
    m_start_export_time = -1;
    m_changed = false;

    connect(ui->delete_btn,&QPushButton::clicked,
            this, &ParticlePanel::onDeleteButtonClick);
    connect(ui->exportButton,&QPushButton::clicked,
            this, &ParticlePanel::onExportButtonClick);
    connect_T(ui->itemToAdd,QComboBox,currentIndexChanged, int,
              &ParticlePanel::onItemToAddIndexChange);
    connect(ui->browseButton,&QPushButton::clicked,
            this, &ParticlePanel::onBrowseButtonClick);
    connect(ui->setButton,&QPushButton::clicked,
            this, &ParticlePanel::onSetButtonClick);
    connect(ui->closeButton,&QPushButton::clicked,
            this, &ParticlePanel::onCloseButtonClick);
    connect(ui->pushButton,&QPushButton::clicked,this,&ParticlePanel::onDivColorChanged);
    connect_T(ui->doubleSpinBox,  QDoubleSpinBox,  valueChanged,double, &ParticlePanel::changeOpacityValue);

    onItemToAddIndexChange(ui->itemToAdd->currentIndex());
//    ui->keepInitial6->setVisible(false);
//    ui->keepInitial7->setVisible(false);
//    ui->keepInitial8->setVisible(false);
//    ui->keepInitial9->setVisible(false);
//    ui->keepInitial10->setVisible(false);
//    ui->keepFinal6->setVisible(false);
//    ui->keepFinal7->setVisible(false);
//    ui->keepFinal8->setVisible(false);
//    ui->keepFinal9->setVisible(false);
//    ui->keepFinal10->setVisible(false);
#ifdef CPU_MODE
    for(int i = 1; i < 6;i++)
    {
        ui->itemToAdd->removeItem(ui->itemToAdd->findText("(Polygon " + QString::number(i) + ")"));
    }
#endif
}
/*===============================================================================*/
/**
 * @brief ParticlePanel::initializeDisplayValues
 *        Iterate through children of groupbox. If child is type QCheckBox
 *        then initialize and append to return list.
 * @return QList<QCheckBox*> list of checkboxes
 */
/*===============================================================================*/
QList<QCheckBox*> ParticlePanel::initializeDisplayValues( )
{
    QList<QCheckBox*> gbchildren;
    QListIterator<QObject *> i(ui->groupDisplay->children());
    while (i.hasNext()){
        QCheckBox* cb= qobject_cast<QCheckBox*>(i.next());
        if (cb){
            connect(cb, &QCheckBox::toggled, this, &ParticlePanel::on_particle_changeFlag);
            cb->setDisabled(true);
            gbchildren.append(cb);
        }
    }
    return gbchildren;
}
/*===============================================================================*/
/**
 * @brief ParticlePanel::initializeKeepInitial
 *        Iterate through children of groupbox. If child is type QCheckBox
 *        then initialize and append to return list.
 * @return QList<QCheckBox*> list of checkboxes
 */
/*===============================================================================*/
QList<QCheckBox*> ParticlePanel::initializeKeepInitial(  )
{
    QList<QCheckBox*> gbchildren;
    QListIterator<QObject *> i(ui->groupKeepInitial->children());
    while (i.hasNext()){
        QCheckBox* cb= qobject_cast<QCheckBox*>(i.next());
        if (cb){
            //            connect(cb, &QCheckBox::toggled, this, &ParticlePanel::on_groupKeepInitial_toggled);
            cb->setDisabled(true);
            gbchildren.append(cb);
        }
    }
    return gbchildren;
}
/*===============================================================================*/
/**
 * @brief ParticlePanel::initializeKeepFinal
 *        Iterate through children of groupbox. If child is type QCheckBox
 *        then initialize and append to return list.
 * @return QList<QCheckBox*> list of checkboxes
 */
/*===============================================================================*/
QList<QCheckBox*> ParticlePanel::initializeKeepFinal()
{
    QList<QCheckBox*> gbchildren;
    QListIterator<QObject *> i(ui->groupKeepFinal->children());
    while (i.hasNext()){
        QCheckBox* cb= qobject_cast<QCheckBox*>(i.next());
        if (cb){
            //            connect(cb, &QCheckBox::toggled, this, &ParticlePanel::on_groupKeepFinal_toggled);
            cb->setDisabled(true);
            gbchildren.append(cb);
        }
    }
    return gbchildren;
}

void ParticlePanel::updateUI2Cmd()
{
    this->setParam(&extCommand->m_parameter.m_particle_merge_param);
}
ParticlePanel::~ParticlePanel()
{
    delete ui;
}

void ParticlePanel::particleExportButtonActive()
{
    ui->exportButton->setEnabled(true);
}
void ParticlePanel::particleResetChangedFlag(){
    m_changed = false;
}
bool ParticlePanel::checkChangeFlag(){
    return m_changed;
}
void ParticlePanel::setParam( kvs::visclient::ParticleMergeParameter* param )
{
    // printf( "In func ParticleMergerUI::setParam\n" );
    this->m_param = param;
    // 粒子の統合情報を設定
    // Set integration information of particles
    for ( int i = 1; i < 11; i++ )
    {
        // 表示、非表示のフラグを設定
        // Set display / non-display flag
        if ( m_particle_value[i]->checkState() == Qt::Checked )
        {
            m_param->m_particles[i].m_enable = true;
//            if(i >= 6 && i <= 10 && m_polygon_changed[i-6] == false){
//                extCommand->registerPolygonModel(m_file_path_strings[i],i,m_polygon_opacity[i-6],m_polygon_color[i-6]);
//                m_polygon_changed[i-6] = true;
//            }
//            if(m_color_opacity_changed[i-6] == true){
//                std::cout << "m_color_opacity_changed" << i-6 << std::endl;
//                extCommand->deletePolygonModel(i);
//                extCommand->registerPolygonModel(m_file_path_strings[i],i,m_polygon_opacity[i-6],m_polygon_color[i-6]);
//                m_color_opacity_changed[i-6] = false;
//            }
        }
        else
        {
            m_param->m_particles[i].m_enable = false;
//            if(i >= 6 && i <= 10 && m_polygon_changed[i-6] == true){
//                extCommand->deletePolygonModel(i);
//                m_polygon_changed[i-6] = false;
//            }
        }
        // 粒子ファイルのパスを設定。ただし未設定の場合、表示フラグはfalseにする
        // Set the path of particle file. However, if not set, the display flag is set to false
        if ( m_file_path_strings[i].empty() != true )
        {
            printf( "Set filepat\n" );
            printf( "Input m_file_path %s\n", m_file_path_strings[i].c_str() );
            m_param->m_particles[i].m_file_path.clear();
            m_param->m_particles[i].m_file_path.append( m_file_path_strings[i] );
            printf( "Input m_file_path %s\n", m_param->m_particles[i].m_file_path.c_str() );
        }
        else
        {
            m_param->m_particles[i].m_enable = false;
            m_param->m_particles[i].m_file_path.clear();
        }
    }
    // check on the first list
    if ( m_particle_value[0]->checkState() == Qt::Checked )
    {
        m_param->m_particles[0].m_enable = true;
        m_param->m_particles[0].m_file_path = m_file_path_strings[0];
    }
    else
    {
        m_param->m_particles[0].m_enable = false;
    }

    for ( int i = 0; i < 11; i++ )
    {
        //    MOD BY)T.Osaki 2020.03.16
        if(m_keep_initial_value[i]->checkState() == Qt::Checked)
        {
            m_param->m_particles[i].m_keep_initial_step = true;
        }else{
            m_param->m_particles[i].m_keep_initial_step = false;
        }
        if(m_keep_final_value[i]->checkState() == Qt::Checked)
        {
            m_param->m_particles[i].m_keep_final_step   = true;
        }else{
            m_param->m_particles[i].m_keep_final_step   = false;
        }
    }

    // Exportが押された時の処理
    // processing when Export is pressed
    if ( m_do_export )
    {
        if (!m_text_output_file_path.isNull()  )

        {
            // Exportフラグを設定
            m_param->m_do_export = true;
            // 粒子の出力情報を設定
            m_param->m_export_file_path.clear();
            m_param->m_export_file_path.append( m_text_output_file_path.toStdString() );
            // 出力時刻を記録（実際には書き出し１つ前の時刻）
            m_start_export_time = m_param->m_circuit_time;
            // Exportが押された時のフラグを元に戻す
            m_do_export = false;
        }
    }
}


void ParticlePanel::onDeleteButtonClick()
{
    qInfo( "on_delete_btn_clicked");
    int item_num = ui->itemToAdd->currentIndex()+1;
    QString item_text=ui->itemToAdd->currentText();

//    printf( "delete item %s.(%d)\n",item_text.toStdString() , item_num );
    if ( m_particle_value[item_num]->checkState() == Qt::Checked)
    {
        m_changed = true;; // 表示されていたものが消えるのでViewerの初期化
    }
    // パーティクルをリストをParticleに戻す
//    QString buf = QString("(Particle%1)").arg(item_num);
    QString buf;
    if(item_num >= 1 && ui->itemToAdd->currentIndex() <= 5  )
    {
        buf = QString("(Particle %1)").arg(item_num);
    }
    if(item_num >= 6 && ui->itemToAdd->currentIndex() <= 10  )
    {
        buf = QString("(Polygon %1)").arg(item_num-5);
    }

    m_particle_value[item_num]->setText(buf);
//    m_keep_initial_value[item_num]->setText(buf);
//    m_keep_final_value[item_num]->setText(buf);
    // 内部テーブルも初期化
    m_item_strings[item_num].erase( 0 );
    m_item_strings[item_num].append( buf.toStdString() );
    m_file_path_strings[item_num].clear();
    // チェックを外す
    m_particle_value[item_num]->setChecked(false );
    m_keep_initial_value[item_num]->setChecked(false );
    m_keep_final_value[item_num]->setChecked(false );
    // 操作不可能にする
    m_particle_value[item_num]->setEnabled(false);
    m_keep_initial_value[item_num]->setEnabled(false);
    m_keep_final_value[item_num]->setEnabled(false);
    m_total_item--;
    // 削除リストを初期化
    ui->itemToAdd->clear();
#ifdef CPU_MODE
            for ( int i = 1; i < 6; i++ )
#endif
#ifdef GPU_MODE
            for ( int i = 1; i < 11; i++ )
#endif
    {
        ui->itemToAdd->addItem(m_particle_value[i]->text());
        //            m_list_add->add_item( i, m_checkbox_particle[i]->name.c_str() );
    }
    ui->itemToAdd->setCurrentIndex(item_num-1);
}


void ParticlePanel::onExportButtonClick()
{
    QDir dir(".");
    QString absolute_path=QFileDialog::getSaveFileName(this, tr("Select Export Folder and prefix for dataset name"), "", "");

    if(!absolute_path.isEmpty() && !absolute_path.isNull())
    {
    m_text_output_file_path = dir.relativeFilePath(absolute_path);

    int check = 0;
    // 表示しているものがあるか確認
    for ( int i = 0; i < 11; i++ )
    {
        if ( m_particle_value[i]->checkState() == Qt::Checked )
        {
            check = 1;
        }
    }

    // 表示しているものがあれば粒子出力をする
    if ( check == 1 )
    {
        // Exportボタンを不活性に切り替える
        ui->exportButton->setDisabled(true);
        // flagを立てる
        m_do_export = true;
    }
    }
}

void ParticlePanel::onItemToAddIndexChange(int index)
{
    int item_num=index+1;
    QString fp=(m_file_path_strings[item_num].c_str());
    QString pn=(m_item_strings[item_num].c_str());

    ui->particleFile->setText(fp);
    ui->particleName->setText(pn);
    if(item_num >= 6 && item_num <= 10){
        ui->pushButton->setVisible(true);
        ui->doubleSpinBox->setVisible(true);
        ui->label->setVisible(true);
    }else{
        ui->pushButton->setVisible(false);
        ui->doubleSpinBox->setVisible(false);
        ui->label->setVisible(false);
    }
}

void ParticlePanel::onBrowseButtonClick()
{
    QString  particle_file = QFileDialog::getOpenFileName(this, tr("Particle File"), ".", tr("Particle Files (*.* *.*)"));
    if(!particle_file.isEmpty()&& !particle_file.isNull()){
        //ADD BY)T.Osaki 2020.04.24
        particle_file = QDir::toNativeSeparators(particle_file);
        ui->particleFile->setText(particle_file);

    }
}

void ParticlePanel::onSetButtonClick()
{

    int item_num = ui->itemToAdd->currentIndex()+1;// m_list_add->get_int_val();

    qInfo( "In func particle_add_func %d",item_num );
    QString particleFileName=ui->particleFile->text().trimmed();
    QString particleName=ui->particleName->text().trimmed();

    if (!particleFileName.isEmpty() )
    {
        if (particleName.isEmpty()){
            QFileInfo qfi(particleFileName);
            particleName=qfi.fileName();
        }

            m_file_path_strings[item_num].clear();
            m_file_path_strings[item_num].append(particleFileName.toStdString());
            printf( "Set m_file_path : %s\n", m_file_path_strings[item_num].c_str() );

            // 削除リストを初期化
            //        ui->itemToDelete->clear();
            m_total_item++;
            m_particle_value[item_num]->setText(particleName);
//            m_keep_initial_value[item_num]->setText( particleName );
//            m_keep_final_value[item_num]->setText(particleName);
            m_item_strings[item_num].erase( 0 );
            m_item_strings[item_num].append( particleName.toStdString());

            // 操作可能にする
            m_particle_value[item_num]->setEnabled(true);
            m_keep_initial_value[item_num]->setEnabled(true);
            m_keep_final_value[item_num]->setEnabled(true);


            // addリストに追加
            ui->itemToAdd->clear();
#ifdef CPU_MODE
            for ( int i = 1; i < 6; i++ )
#endif
#ifdef GPU_MODE
            for ( int i = 1; i < 11; i++ )
#endif
            {
                ui->itemToAdd->addItem(m_particle_value[i]->text());
                //            m_list_add->add_item( i, m_checkbox_particle[i]->name.c_str() );
            }
            ui->itemToAdd->setCurrentIndex(item_num-1);
            ui->delete_btn->setEnabled(true);
//        }
    }
    this->setParam(this->m_param);
}

void ParticlePanel::onCloseButtonClick()
{
    qInfo("CLOSING PARTICLE PANEL");
    this->close();
}

void ParticlePanel::onDivColorChanged()
{
    qDebug("on_divColor_btn_clicked");
    if(ui->itemToAdd->currentIndex() >= 5 && ui->itemToAdd->currentIndex() <= 9  ){
        m_param->m_particles[ui->itemToAdd->currentIndex()+1].m_polygon_color=this->m_polygon_color_edit->getColorSelection();;
    }
}

void ParticlePanel::changeOpacityValue()
{
    qDebug("on_divColor_btn_clicked");
    if(ui->itemToAdd->currentIndex() >= 5 && ui->itemToAdd->currentIndex() <= 9  ){
        m_param->m_particles[ui->itemToAdd->currentIndex()+1].m_polygon_opacity=ui->doubleSpinBox->value();
    }
}

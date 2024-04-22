#include "MergePanel.h"
#include "ui_MergePanel.h"

#include <QColorDialog>
#include <QXmlStreamReader>
#include <QCheckBox>
#include <QDoubleSpinBox>

#include <kvs/ObjectBase>

#include <kvs/ObjectManager>

#include <kvs/PolygonImporter>
#include <kvs/PolygonObject>
#include <kvs/StochasticPolygonRenderer>

#include <kvs/PointImporter>
//#include <kvs/PointObject>
#include <kvs/ParticleBasedRenderer>

#include <kvs/IDManager>
#include "ExtendedKVS/CustomObjectManager.h"
#include <kvs/KVSMLPointObject>
#include <kvs/PointExporter>

#include "ExtendedKVS/LASImporter.h"
#include "ExtendedKVS/PTSImporter.h"

#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
#include "ExtendedKVS/TexturedPolygonImporter.h"
#include "ExtendedKVS/TexturedPolygonObject.h"
#include "ExtendedKVS/StochasticTexturedPolygonRenderer.h"
#endif

MergePanel::MergePanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::MergePanel),
    m_files_manager(),
    m_time_control( nullptr ),
    m_current_time_step( -1 ),
    m_is_worker_thread_running( false ),
    m_is_export( false )
{
    ui->setupUi(this);
    connect(ui->filesTWidget, &QTableWidget::cellDoubleClicked, this, &MergePanel::onFilesTWidgetCellDoubleClicked);
    connect(ui->importFilesBrowsePBtn, &QPushButton::clicked, this, &MergePanel::onBrowserButtonClicked );
    connect(ui->importFilesAddPBtn, &QPushButton::clicked, this, &MergePanel::onAddButtonClicked );
    connect(ui->exportPBtn, &QPushButton::clicked, this, &MergePanel::onExportButtonClicked );
    connect(ui->centeringPBtn, &QPushButton::clicked, this, &MergePanel::onCenteringButtonClicked );
    connect(ui->applyPBtn, &QPushButton::clicked, this, &MergePanel::onApplyButtonClicked );
}

MergePanel::~MergePanel()
{
    qDeleteAll(m_files_manager);
    delete ui;
}

void MergePanel::onFilesTWidgetCellDoubleClicked(int row, int column)
{
    if (column == 4) // Colorのセルをダブルクリックしているか。
    {
        if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML ||
            m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
        {
            QTableWidgetItem* formatItem = ui->filesTWidget->item( row, 4 );
            formatItem->setBackground(QColorDialog::getColor(Qt::gray));
        }
    }
}

void MergePanel::onBrowserButtonClicked()
{
    QFileDialog fileDialog( this );
    fileDialog.setFileMode( QFileDialog::ExistingFile );

#if defined( PBVR_SUPPORT_FBX ) && defined( PBVR_SUPPORT_3DS )
    fileDialog.setNameFilter("*.kvsml *.stl *.fbx *.3ds *.las *.pts");
#elif PBVR_SUPPORT_FBX
    fileDialog.setNameFilter("*.kvsml *.stl *.fbx *.las *.pts");
#elif PBVR_SUPPORT_3DS
    fileDialog.setNameFilter("*.kvsml *.stl *.3ds *.las *.pts");
#else
    fileDialog.setNameFilter("*.kvsml *.stl *.las *.pts");
#endif

    if( fileDialog.exec() )
    {
        QString filePath = fileDialog.selectedFiles().at( 0 );
        ui->importFilesPathLEdit->setText( filePath );
    }
}

void MergePanel::onAddButtonClicked()
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( ui->importFilesPathLEdit->text() ) );
    newFile->setRGBColor( QColor( 128, 128, 128 ) );
    newFile->setOpacity( 0.5 );
    checkMinMaxTimeStep( newFile );
    newFile->setCurrentDisplayedStep( -1 );
    checkFileFormat( newFile );
    newFile->setObject( nullptr );
    addRowToFilesTableWidget( newFile );
    m_files_manager.append( newFile );
    calculateTotalMinMaxTimeStep();
}

void MergePanel::checkMinMaxTimeStep( FilesManager *newFile )
{
    const QRegularExpression regex( newFile->getFileInfo().baseName().left( newFile->getFileInfo().baseName().indexOf('_')) + "_([0-9]+)\\.*");

    int minStep = std::numeric_limits<int>::max();
    int maxStep = std::numeric_limits<int>::min();

    //    foreach( fileInfo, fileInfo.dir().entryInfoList( QDir::Files ) )
    foreach( const QFileInfo &fileInfo, newFile->getFileInfo().dir().entryInfoList( QDir::Files ) )
    {
        QRegularExpressionMatch match = regex.match( fileInfo.fileName() );

        if( match.hasMatch() )
        {
            int nummericalValue = match.captured(1).toInt();
            if( nummericalValue < minStep )
            {
                minStep = nummericalValue;
            }
            if( nummericalValue > maxStep )
            {
                maxStep = nummericalValue;
            }
        }
    }

    if( minStep != std::numeric_limits<int>::max() && maxStep != std::numeric_limits<int>::min() )
    {
        //        qInfo() << minStep << "," << maxStep;
        newFile->setMinTimeStep( minStep );
        newFile->setMaxTimeStep( maxStep );
    }
    else//CAN NOT FIND MIN MAX TIME STEP
    {
    }
}

void MergePanel::checkFileFormat( FilesManager *newFile )
{
    QFile file(newFile->getFileInfo().filePath());
    QXmlStreamReader xml( &file );
    QString fileSuffix = newFile->getFileInfo().suffix().toLower();
    newFile->setFormat( FilesManager::Unknown );

    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )//CAN NOT OPEN FILE
    {
        return;
    }

    if( fileSuffix == "kvsml" )
    {
        while ( !xml.atEnd() && !xml.hasError() )
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            if( token == QXmlStreamReader::StartElement )
            {
                QString tagName = xml.name().toString();
                if( tagName == "PointObject" )
                {
                    newFile->setFormat( FilesManager::PointObjectKVSML );
                    break;
                }
                else if( tagName == "PolygonObject" )
                {
                    newFile->setFormat( FilesManager::NonTexturedPolygonObjectKVSML );
                    break;
                }
            }
        }

        if( xml.hasError() )//XML PARSE ERROR
        {
        }
    }
    else if( fileSuffix == "stl" )
    {
        newFile->setFormat( FilesManager::NonTexturedPolygonObjectSTL );
    }
#ifdef PBVR_SUPPORT_FBX
    else if( fileSuffix == "fbx" )
    {
        newFile->setFormat( FilesManager::TexturedPolygonObjectFBX );
    }
#endif
#ifdef PBVR_SUPPORT_3DS
    else if( fileSuffix == "3ds" )
    {
        newFile->setFormat( FilesManager::TexturedPolygonObject3DS );
    }
#endif
    else if( fileSuffix == "las" )
    {
        newFile->setFormat( FilesManager::PointObjectLAS );
    }
    else if( fileSuffix == "pts" )
    {
        newFile->setFormat( FilesManager::PointObjectPTS );
    }
    else //NOT SUPPORTED FORMAT
    {
    }
}

void MergePanel::addRowToFilesTableWidget( FilesManager *newFile )
{
    int row = ui->filesTWidget->rowCount();
    ui->filesTWidget->setRowCount( row + 1 );

    QTableWidgetItem *headerItem = new QTableWidgetItem( newFile->getFileInfo().baseName().left( newFile->getFileInfo().baseName().indexOf('_') ) );
    ui->filesTWidget->setVerticalHeaderItem(row, headerItem);

    QCheckBox *displayCheckBox = new QCheckBox();
    displayCheckBox->setCheckState( Qt::Checked );

    QCheckBox *keepInitialCheckBox = new QCheckBox();
    keepInitialCheckBox->setCheckState( Qt::Unchecked );

    QCheckBox *keepFinalCheckBox = new QCheckBox();
    keepFinalCheckBox->setCheckState( Qt::Unchecked );

    QTableWidgetItem* format = new QTableWidgetItem;
    format->setFlags( format->flags() & ~Qt::ItemIsEditable );
    format->setText( newFile->formatTypeToString( newFile->getFormat() ) );

    if( newFile->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML || newFile->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
    {
        QTableWidgetItem *colorValue = new QTableWidgetItem;
        colorValue->setFlags( format->flags() & ~Qt::ItemIsEditable );
        colorValue->setBackground( newFile->getRGBColor() );

        QDoubleSpinBox *opacityValue = new QDoubleSpinBox();
        opacityValue->setRange( 0.0, 1.0 );
        opacityValue->setSingleStep( 0.01 );
        opacityValue->setValue( newFile->getOpacity() );
        ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 4, colorValue );
        ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 5, opacityValue);
    }
    else
    {
        QTableWidgetItem* empCell1 = new QTableWidgetItem;
        QTableWidgetItem* empCell2 = new QTableWidgetItem;
        empCell1->setFlags( empCell1->flags() & ~Qt::ItemIsEditable );
        empCell2->setFlags( empCell2->flags() & ~Qt::ItemIsEditable );
        ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 4, empCell1 );
        ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 5, empCell2 );

    }

    QCheckBox *deleteCheckBox = new QCheckBox;
    deleteCheckBox->setCheckState( Qt::Unchecked );

    ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 0, displayCheckBox );
    ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 1, keepInitialCheckBox );
    ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 2, keepFinalCheckBox );
    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 3, format );

    ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 6, deleteCheckBox );
}

void MergePanel::calculateTotalMinMaxTimeStep()
{
    int totalMinTimeStep = std::numeric_limits<int>::max();
    int totalMaxTimeStep = std::numeric_limits<int>::min();
    bool isSingleObject = true;

    for( int i = 0; i < m_files_manager.size(); i++ )
    {
        FilesManager* fileManager = m_files_manager[i];
        totalMinTimeStep = std::min( totalMinTimeStep, fileManager->getMinTimeStep() );
        totalMaxTimeStep = std::max( totalMaxTimeStep, fileManager->getMaxTimeStep() );
        if(i >= 1)
        {
            isSingleObject = false;
        }
    }
    m_time_control->updateTimeStepMinMax( totalMinTimeStep, totalMaxTimeStep, isSingleObject);
}

void MergePanel::onExportButtonClicked()
{
    m_export_file_path = QFileDialog::getSaveFileName(this, tr("Save Server-Side Point Object"), QDir::homePath(), tr("すべてのファイル (*.*)"));
    if ( !m_export_file_path.isEmpty() )
    {
        ui->exportPBtn->setEnabled( false );
        m_is_export = true;
    }
    else
    {
        m_is_export = false;
    }
}

void MergePanel::onApplyButtonClicked()
{
    removeChecked();
    updatePolygonColorOpacity();
    mergeObjects();
}

void MergePanel::removeChecked()
{
    for (int row = 0; row < m_files_manager.size(); row++)
    {
        QWidget *widget = ui->filesTWidget->cellWidget(row, 6); // deleteCheckBox の列を指定
        QCheckBox *deleteCheckBox = qobject_cast<QCheckBox*>(widget);
        if (deleteCheckBox && deleteCheckBox->checkState() == Qt::Checked)
        {
            m_screen->scene()->removeObject( m_files_manager[row]->getIds().first );

            m_files_manager[row]->setIds( std::pair<int,int>(-1,-1) );
            ui->filesTWidget->removeRow(row);
            delete deleteCheckBox;

            // m_files_managerからも対応する要素を削除する
            if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObject )
            {
                m_connect->deletedServerObject();
            }
            delete m_files_manager[row];
            m_files_manager.removeAt(row);

            row--; // 行が削除されたので、ループのインデックスを調整する
        }
    }
    calculateTotalMinMaxTimeStep();
}

void MergePanel::updatePolygonColorOpacity()
{
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML ||
            m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
        {
            QTableWidgetItem* colorItem = ui->filesTWidget->item(row, 4);
            QDoubleSpinBox* opacitySpinBox = dynamic_cast<QDoubleSpinBox*>(ui->filesTWidget->cellWidget(row, 5));
            if( colorItem != nullptr && opacitySpinBox != nullptr )
            {
                QColor rgbColor = colorItem->background().color();
                double opacity = opacitySpinBox->value();
                if( m_files_manager[row]->getRGBColor() != rgbColor )
                {
                    m_files_manager[row]->setRGBColor( rgbColor );
                }
                if( m_files_manager[row]->getOpacity() != opacity )
                {
                    m_files_manager[row]->setOpacity( opacity );
                }
            }
        }
    }
}

void MergePanel::mergeObjects()
{
    if( m_is_worker_thread_running )//スレッド処理実行中は何もしない。
    {
        return;
    }
    WorkerThread* workerThread = new WorkerThread( this );
    connect( workerThread, &QThread::finished, workerThread, &QObject::deleteLater );
    connect(workerThread, &WorkerThread::workFinished, this, &MergePanel::onWorkerThreadFinished);
    workerThread->start();
    m_is_worker_thread_running = true;

//    m_time_control->setCurrentTimeStep( m_time_control->getNextTimeStep() );
//    m_preference->setCurrentTimeStep( m_time_control->getNextTimeStep() );
//    m_preference->loadShadingSettings();
//    m_preference->applyShadingSettings();
//    totalParticles();
//    m_screen->update();
}

//void MergePanel::exportingServerSidePointObject( FilesManager& filesManager, const kvs::PointObject& server_point_object )
void MergePanel::exportingServerSidePointObject( FilesManager& filesManager )
{
    const int nextTimeStep = m_time_control->getNextTimeStep();
    QFileInfo fileInfo( m_export_file_path + "_" + QString( "%1" ).arg( nextTimeStep, 5, 10, QChar('0') ) + ".kvsml" );

    if( fileInfo.exists() ) //ファイルが存在している場合はエクスポート済みとする。
    {
        //何もしない。
    }
    else //ファイルが存在しない場合はエクスポートする。
    {
        auto* pointObject = dynamic_cast<kvs::PointObject*>( m_screen->scene()->objectManager()->object( filesManager.getIds().first ) );
        kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>( pointObject );
        if( !kvsml )
        {
            ui->exportPBtn->setEnabled( true );
            m_is_export = false;
        }
        else
        {
            kvsml->setWritingDataTypeToExternalBinary();
            kvsml->write( fileInfo.filePath().toStdString() );
        }
        delete kvsml;
    }
}

void MergePanel::isExportDone( FilesManager& filesManager )
{
    QFileInfo fileInfo( m_export_file_path );
    QStringList files = fileInfo.dir().entryList(QStringList( fileInfo.fileName() + "_*.kvsml"), QDir::Files);
    int count = 0;
    for (const auto& file : files)
    {
        count++;
    }

    if( filesManager.getMaxTimeStep() - filesManager.getMinTimeStep() + 1 == count )
    {
        ui->exportPBtn->setEnabled( true );
        m_is_export = false;
    }
}

void MergePanel::onWorkerThreadFinished()
{
    kvs::Xform before_object_manager_xform = m_screen->scene()->objectManager()->xform();
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        if( m_files_manager[row]->getIds().first == -1 && m_files_manager[row]->getIds().second == -1 ) //オブジェクトが登録されていない
        {
            //            if( nextObject != nullptr ) //オブジェクトが生成されていれば登録
            if( m_files_manager[row]->getObject() != nullptr ) //オブジェクトが生成されていれば登録
            {
                qInfo() << "オブジェクトの生成があるため登録を行います。";
                //                nextObject->setXform( before_object_manager_xform );
                m_files_manager[row]->getObject()->setXform( before_object_manager_xform );
                if( kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>(m_files_manager[row]->getObject()) )
                {
                    polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue() ) );
                    polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_polygon_renderer = new kvs::StochasticPolygonRenderer;
                    m_preference->applyShading( stochastic_polygon_renderer );
                    m_files_manager[row]->setIds( m_screen->scene()->registerObject( polygon_object, stochastic_polygon_renderer ) );
                }
                else if( kvs::PointObject* point_object = dynamic_cast<kvs::PointObject*>(m_files_manager[row]->getObject()) )
                {
                    kvs::RendererBase* particle_based_renderer = new kvs::glsl::ParticleBasedRenderer;
                    m_preference->applyShading( particle_based_renderer );
                    m_files_manager[row]->setIds( m_screen->scene()->registerObject( point_object, particle_based_renderer ) );

//                    if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_is_export == true ) //サーバサイドポイントオブジェクトを保存する場合
//                    {
//                        exportingServerSidePointObject( *m_files_manager[row] ,*point_object );
//                    }
                }
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
                else if( kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>(m_files_manager[row]->getObject()) )
                {
                    textured_polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue() ) );
                    textured_polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_textured_polygon_renderer = new kvs::StochasticTexturedPolygonRenderer;
                    m_preference->applyShading( stochastic_textured_polygon_renderer );
                    m_files_manager[row]->setIds( m_screen->scene()->registerObject( textured_polygon_object, stochastic_textured_polygon_renderer ) );
                }
#endif
                //                RendererType* polygonRenderer = new RendererType();
                //                m_merge->m_files_manager[row]->setIds( m_merge->m_screen->scene()->registerObject( nextObject, polygonRenderer ) );
            }
        }
        else //オブジェクトが登録されている。
        {
            auto* object = m_screen->scene()->object( m_files_manager[row]->getIds().first );
            if( m_files_manager[row]->getObject() != nullptr ) //オブジェクトが生成されていれば交換
            {
                if( kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>(m_files_manager[row]->getObject()) )
                {
                    polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue() ) );
                    polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    m_screen->scene()->replaceObject(m_files_manager[row]->getIds().first, polygon_object );
                }
                else if( kvs::PointObject* point_object = dynamic_cast<kvs::PointObject*>(m_files_manager[row]->getObject()) )
                {
                    m_screen->scene()->replaceObject(m_files_manager[row]->getIds().first, point_object );
//                    if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_is_export == true ) //サーバサイドポイントオブジェクトを保存する場合
//                    {
//                        exportingServerSidePointObject( *m_files_manager[row] ,*point_object );
//                    }
                }
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
                else if( kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>(m_files_manager[row]->getObject()) )
                {
                    m_screen->scene()->replaceObject(m_files_manager[row]->getIds().first, textured_polygon_object );
                }
#endif
            }
//            else if ( auto* polygonObject = static_cast<kvs::PolygonObject*>( object ) )
            else if ( auto* polygonObject = dynamic_cast<kvs::PolygonObject*>( object ) )
            {
                kvs::PolygonObject* copiedObject = new kvs::PolygonObject();
                copiedObject->deepCopy( *polygonObject );
                copiedObject->setColor(kvs::RGBColor(m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue()));
                copiedObject->setOpacity(m_files_manager[row]->getOpacity() * 255);
                m_screen->scene()->replaceObject(m_files_manager[row]->getIds().first, copiedObject);
            }
        }
        if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_is_export == true ) //サーバサイドポイントオブジェクトを保存する場合
        {
            exportingServerSidePointObject( *m_files_manager[row] );
//            m_screen->scene()->objectManager()->object( m_files_manager[row]->getIds().first );
            isExportDone( *m_files_manager[row] );
        }
    }   
    m_time_control->setCurrentTimeStep( m_time_control->getNextTimeStep() );
    m_preference->setCurrentTimeStep( m_time_control->getNextTimeStep() );    
    totalParticles();
    m_is_worker_thread_running = false;
    m_screen->update();
    m_time_control->incrementSpinBox();
}

void MergePanel::totalParticles()
{
    int totalParticles = 0;
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObject ||
            m_files_manager[row]->getFormat() == FilesManager::PointObjectKVSML ||
            m_files_manager[row]->getFormat() == FilesManager::PointObjectLAS ||
            m_files_manager[row]->getFormat() == FilesManager::PointObjectPTS )
        {
            auto* object = m_screen->scene()->object( m_files_manager[row]->getIds().first );
            if( object->isVisible() )
            {
                if (auto* pointObject = dynamic_cast<kvs::PointObject*>(object))
                {
                    totalParticles += pointObject->numberOfVertices();
                }
            }
        }
    }
    m_data_summary->setTotalParticles( totalParticles );
}

void MergePanel::onCenteringButtonClicked()
{
    CustomObjectManager* object_manager = static_cast<CustomObjectManager*>( m_screen->scene()->objectManager() );
    kvs::Vec3 min_obj;
    kvs::Vec3 max_obj;
    int counter = 0;

    for (int row = 0; row < m_files_manager.size(); ++row)
    {
        if (m_screen->scene()->object(m_files_manager[row]->getIds().first)->isVisible())
        {
            ++counter;
            if (counter >= 2)
            {
                break;
            }
        }
    }

    if( counter == 1 )
    {
        kvs::Vec3 init_object_manager_min_object( 1e+06, 1e+06, 1e+06 );
        kvs::Vec3 init_object_manager_max_object( -1e+06, -1e+06, -1e+06 );
        for( int row = 0; row < m_files_manager.size(); row++ )
        {
            if( m_files_manager[row]->getIds().first != -1 && m_files_manager[row]->getIds().second != -1 )
            {
                auto* object = m_screen->scene()->object( m_files_manager[row]->getIds().first );
                if( object->isVisible() )
                {
                    min_obj.x() = kvs::Math::Min( init_object_manager_min_object.x(), object->minExternalCoord().x() );
                    min_obj.y() = kvs::Math::Min( init_object_manager_min_object.y(), object->minExternalCoord().y() );
                    min_obj.z() = kvs::Math::Min( init_object_manager_min_object.z(), object->minExternalCoord().z() );

                    max_obj.x() = kvs::Math::Max( init_object_manager_max_object.x(), object->maxExternalCoord().x() );
                    max_obj.y() = kvs::Math::Max( init_object_manager_max_object.y(), object->maxExternalCoord().y() );
                    max_obj.z() = kvs::Math::Max( init_object_manager_max_object.z(), object->maxExternalCoord().z() );
                }
            }
        }
        kvs::Vec3 diff = max_obj - min_obj;
        float max_diff = kvs::Math::Max( diff.x(), diff.y(), diff.z() );
        float normalize = 6.0f / max_diff;

        object_manager->setMinMaxObjectCoords( min_obj, max_obj );
        object_manager->setMinMaxExternalCoords( kvs::Vec3( -3, -3, -3 ), kvs::Vec3( 3, 3, 3 ) );
        object_manager->setNormalize( kvs::Vec3( normalize, normalize, normalize) );
    }
    else
    {
        kvs::Vec3 init_object_manager_min_object = m_screen->scene()->objectManager()->minObjectCoord();
        kvs::Vec3 init_object_manager_max_object = m_screen->scene()->objectManager()->maxObjectCoord();
        for( int row = 0; row < m_files_manager.size(); row++ )
        {
            if( m_files_manager[row]->getIds().first != -1 && m_files_manager[row]->getIds().second != -1 )
            {
                auto* object = m_screen->scene()->object( m_files_manager[row]->getIds().first );
                if( object->isVisible() )
                {
                    min_obj.x() = kvs::Math::Min( init_object_manager_min_object.x(), object->minExternalCoord().x() );
                    min_obj.y() = kvs::Math::Min( init_object_manager_min_object.y(), object->minExternalCoord().y() );
                    min_obj.z() = kvs::Math::Min( init_object_manager_min_object.z(), object->minExternalCoord().z() );

                    max_obj.x() = kvs::Math::Max( init_object_manager_max_object.x(), object->maxExternalCoord().x() );
                    max_obj.y() = kvs::Math::Max( init_object_manager_max_object.y(), object->maxExternalCoord().y() );
                    max_obj.z() = kvs::Math::Max( init_object_manager_max_object.z(), object->maxExternalCoord().z() );
                }
            }
        }
        kvs::Vec3 diff = max_obj - min_obj;
        float max_diff = kvs::Math::Max( diff.x(), diff.y(), diff.z() );
        float normalize = 6.0f / max_diff;

        object_manager->setMinMaxObjectCoords( min_obj, max_obj );
        object_manager->setMinMaxExternalCoords( kvs::Vec3( -3, -3, -3 ), kvs::Vec3( 3, 3, 3 ) );
        object_manager->setNormalize( kvs::Vec3( normalize, normalize, normalize) );
    }
    m_screen->scene()->reset();
    m_screen->update();
}

void MergePanel::serverObject( QString volumeDataFilePath, int min, int max )
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setCurrentDisplayedStep( -1 );
    newFile->setFormat( FilesManager::ServerPointObject );
    newFile->setObject( nullptr );
    addRowToFilesTableWidget( newFile );
    m_files_manager.append( newFile );
    calculateTotalMinMaxTimeStep();
}

MergePanel::WorkerThread::WorkerThread(MergePanel* gui) : m_merge(gui)
{
}

void MergePanel::WorkerThread::run()
{
    for( int row = 0; row < m_merge->m_files_manager.size(); row++ )
    {
        switch ( m_merge->m_files_manager[row]->getFormat() )
        {
        case FilesManager::ServerPointObject:
            timeStepCheckAndImport<void, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager::PointObjectKVSML:
            timeStepCheckAndImport<kvs::PointImporter, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager::PointObjectLAS:
            timeStepCheckAndImport<LASImporter, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager::PointObjectPTS:
            timeStepCheckAndImport<PTSImporter, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager::NonTexturedPolygonObjectKVSML:
        case FilesManager::NonTexturedPolygonObjectSTL:
            timeStepCheckAndImport<kvs::PolygonImporter, kvs::PolygonObject, kvs::StochasticPolygonRenderer>( row );
            break;
#ifdef PBVR_SUPPORT_FBX
        case FilesManager::TexturedPolygonObjectFBX:
#endif
#ifdef PBVR_SUPPORT_3DS
        case FilesManager::TexturedPolygonObject3DS:
#endif
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
            timeStepCheckAndImport<kvs::TexturedPolygonImporter, kvs::TexturedPolygonObject, kvs::StochasticTexturedPolygonRenderer>(row);
            break;
#endif
        default:
            break;
        }
    }
    emit workFinished();
}

MergePanel::WorkerThread::CheckPattern MergePanel::WorkerThread::checkPattern( int row )
{
    QCheckBox *keepInitialCheckBox = qobject_cast<QCheckBox*>( m_merge->ui->filesTWidget->cellWidget(row, 1) );
    QCheckBox *keepFinalCheckBox = qobject_cast<QCheckBox*>( m_merge->ui->filesTWidget->cellWidget(row, 2) );
    bool keepInitialChecked = keepInitialCheckBox->isChecked();
    bool keepFinalChecked = keepFinalCheckBox->isChecked();

    if (keepInitialChecked && !keepFinalChecked)
    {
        return KeepInitialChecked;
    }
    else if (!keepInitialChecked && keepFinalChecked)
    {
        return KeepFinalChecked;
    }
    else if (keepInitialChecked && keepFinalChecked)
    {
        return BothChecked;
    }
    else
    {
        return NoneChecked;
    }
}

template <typename Importer, typename ObjectType, typename RendererType>
void MergePanel::WorkerThread::timeStepCheckAndImport( int row )
{
//    kvs::Xform before_object_manager_xform = m_merge->m_screen->scene()->objectManager()->xform();
    QCheckBox *displayCheckBox = qobject_cast<QCheckBox*>( m_merge->ui->filesTWidget->cellWidget( row, 0 ) );
    MergePanel::WorkerThread::CheckPattern pattern = checkPattern( row );
    const int minTimeStep  = m_merge->m_files_manager[row]->getMinTimeStep();
    const int maxTimeStep  = m_merge->m_files_manager[row]->getMaxTimeStep();
    const int currentTimeStep = m_merge->m_time_control->getCurrentTimeStep();
    const int nextTimeStep = m_merge->m_time_control->getNextTimeStep();
    const QString filePath = m_merge->m_files_manager[row]->getFileInfo().filePath();
    //    ObjectType* nextObject = nullptr;
    m_merge->m_files_manager[row]->setObject( nullptr );

    if( m_merge->m_files_manager[row]->getIds().first == -1 && m_merge->m_files_manager[row]->getIds().second == -1 ) //オブジェクトが登録されていない
    {
        qInfo() << "オブジェクトの登録が行われていません。" << __LINE__;
        if( displayCheckBox->isChecked() ) //表示の要求がある。
        {
            qInfo() << "表示にチェックがついています。" << __LINE__;
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep ) //次のステップがローカルファイルの最小最大の範囲である場合
            {
                qInfo() << "次のタイムステップがローカルタイムステップの範囲内です。" << __LINE__;
                qInfo() << "次ステップをインポートします。" << __LINE__;
                if constexpr (!std::is_same_v<Importer, void>)
                {
//                    nextObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ); //次のステップのファイルを表示
                    m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ) );
                    m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                }
                else
                {
//                    nextObject = m_merge->m_connect->connect2( nextTimeStep );
                    m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                    m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                }
                m_merge->m_files_manager[row]->setCurrentDisplayedStep( nextTimeStep );
            }
            else if( nextTimeStep < minTimeStep ) //次のステップがローカルファイルの最小よりも低い場合
            {
                qInfo() << "次のタイムステップがローカルの最小ステップよりも小さいです。" << __LINE__;
                if( pattern == MergePanel::WorkerThread::KeepInitialChecked || pattern == MergePanel::WorkerThread::BothChecked ) //KeepInitialがついている場合は最小のファイルを表示
                {
                    qInfo() << "KeepInitialにチェックがついています。" << __LINE__;
                    qInfo() << "最小ステップをインポートします。" << __LINE__;
                    if constexpr (!std::is_same_v<Importer, void>)
                    {
//                        nextObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ) );
                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                    }
                    else
                    {
//                        nextObject = m_merge->m_connect->connect2( minTimeStep );
                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );

                    }
                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( minTimeStep );
                }
            }
            else if( nextTimeStep > maxTimeStep ) //次のステップがローカルファイルの最大よりも大きい場合
            {
                qInfo() << "次のタイムステップがローカルの最大ステップよりも大きいです。" << __LINE__;
                if( pattern == MergePanel::WorkerThread::KeepFinalChecked || pattern == MergePanel::WorkerThread::BothChecked ) //KeepFinalがついている場合は最大のファイルを表示
                {
                    qInfo() << "KeepFinalにチェックがついています。" << __LINE__;
                    qInfo() << "最大ステップをインポートします。" << __LINE__;
                    if constexpr (!std::is_same_v<Importer, void>)
                    {
//                        nextObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ) );
                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                    }
                    else
                    {
//                        nextObject = m_merge->m_connect->connect2( maxTimeStep );
                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                    }
                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( maxTimeStep );
                }
            }

//            if( nextObject != nullptr ) //オブジェクトが生成されていれば登録
//            {
//                qInfo() << "オブジェクトの生成があるため登録を行います。";
//                nextObject->setXform( before_object_manager_xform );
//                if constexpr (std::is_same_v<Importer, kvs::PolygonImporter>)
//                {
//                    nextObject->setColor( kvs::RGBColor( m_merge->m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue() ) );
//                    nextObject->setOpacity( m_merge->m_files_manager[row]->getOpacity() * 255 );
//                }

//                RendererType* polygonRenderer = new RendererType();
//                m_merge->m_files_manager[row]->setIds( m_merge->m_screen->scene()->registerObject( nextObject, polygonRenderer ) );
//            }
        }
    }
    else //オブジェクトが登録されている。
    {
        auto* object = m_merge->m_screen->scene()->object( m_merge->m_files_manager[row]->getIds().first );
        if( object->isVisible() ) //オブジェクトが表示されている場合
        {
            qInfo() << "オブジェクトが表示状態です。" << __LINE__;
            if( displayCheckBox->isChecked() ) //表示の要求がある。
            {
                qInfo() << "表示にチェックがついています。" << __LINE__;
                if( nextTimeStep != currentTimeStep ) //タイムステップの更新がある場合
                {
                    qInfo() << "タイムステップの更新があります。" << __LINE__;
                    if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep ) //次のステップがローカルファイルの範囲内である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲内です。" << __LINE__;
                        if( m_merge->m_files_manager[row]->getCurrentDisplayedStep() == nextTimeStep ) //表示中のオブジェクトが次のステップと一致する場合
                        {
                            qInfo() << "表示中のオブジェクトと次のステップが一致しているため何もしません。"  << __LINE__;
                            if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                            {
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                m_merge->setIsParticleGenerationNeeded( false );
                            }
                        }
                        else //一致しなかった場合
                        {
                            qInfo() << "表示中のオブジェクトと次のステップが一致しないため次のステップをインポートします。" << __LINE__;
                            if constexpr (!std::is_same_v<Importer, void>)
                            {
//                                nextObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ); //次のステップのファイルを表示
                                m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            else
                            {
//                                nextObject = m_merge->m_connect->connect2( nextTimeStep );
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            m_merge->m_files_manager[row]->setCurrentDisplayedStep( nextTimeStep );
                        }
                    }
                    else //次のステップがローカルファイルの範囲外である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲外です。" << __LINE__;
                        if( nextTimeStep < minTimeStep ) //次のステップがローカルの最小ステップよりも低い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最小ステップよりも小さいです。" << __LINE__;
                            if( pattern == KeepInitialChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepInitialにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() <= minTimeStep ) //表示中のオブジェクトがローカル最小ステップ以下である場合。
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最小ステップ以下であるため何もしません。"  << __LINE__;
                                    if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                                    {
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                        m_merge->setIsParticleGenerationNeeded( false );
                                    }
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最小ステップ以下ではないため最小ステップをインポートします。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( minTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( minTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepInitialにチェックがついていないため非表示にします。" << __LINE__;
                                object->hide();
                            }
                        }
                        else if( nextTimeStep > maxTimeStep ) //次のステップがローカルの最大よりも高い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最大ステップよりも大きいです。" << __LINE__;
                            if( pattern == KeepFinalChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepFinalにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() >= maxTimeStep ) //表示中のオブジェクトがローカル最大ステップ以上である場合。
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最大ステップ以上であるため何もしません。"  << __LINE__;
                                    if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                                    {
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                        m_merge->setIsParticleGenerationNeeded( false );
                                    }
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最大ステップ以上ではないため最大ステップをインポートします。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( maxTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( maxTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepFinalにチェックがついていないため非表示にします。" << __LINE__;
                                object->hide();
                            }
                        }
                    }
                }
                else //タイムステップの更新がない場合
                {
                    qInfo() << "タイムステップの更新がありません。" << __LINE__;
                    if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep ) //次のステップがローカルファイルの範囲内である。
                    {
                        if( m_merge->m_files_manager[row]->getCurrentDisplayedStep() == nextTimeStep ) //表示中のオブジェクトが次のステップと一致する場合
                        {
                            qInfo() << "表示中のオブジェクトと次のステップが一致しているため何もしません。"  << __LINE__;
                            if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                            {
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                m_merge->setIsParticleGenerationNeeded( false );
                            }
                        }
                        else //一致しなかった場合
                        {
                            qInfo() << "表示中のオブジェクトと次のステップが一致しないため次のステップをインポートします。" << __LINE__;
                            if constexpr (!std::is_same_v<Importer, void>)
                            {
//                                nextObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ); //次のステップのファイルを表示
                                m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            else
                            {
//                                nextObject = m_merge->m_connect->connect2( nextTimeStep );
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            m_merge->m_files_manager[row]->setCurrentDisplayedStep( nextTimeStep );
                        }
                    }
                    else //次のステップがローカルファイルの範囲外である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲外です。" << __LINE__;
                        if( nextTimeStep < minTimeStep ) //次のステップがローカルの最小ステップよりも低い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最小ステップよりも小さいです。" << __LINE__;
                            if( pattern == KeepInitialChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepInitialにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() <= minTimeStep ) //表示中のオブジェクトがローカル最小ステップ以下である場合。
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最小ステップ以下であるため何もしません。"  << __LINE__;
                                    if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                                    {
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                        m_merge->setIsParticleGenerationNeeded( false );
                                    }
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最小ステップ以下ではないため最小ステップをインポートします。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( minTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( minTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepInitialにチェックがついていないため非表示にします。" << __LINE__;
                                object->hide();
                            }
                        }
                        else if( nextTimeStep > maxTimeStep ) //次のステップがローカルの最大よりも高い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最大ステップよりも大きいです。" << __LINE__;
                            if( pattern == KeepFinalChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepFinalにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() >= maxTimeStep ) //表示中のオブジェクトがローカル最大ステップ以上である場合。
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最大ステップ以上であるため何もしません。"  << __LINE__;
                                    if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                                    {
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                        m_merge->setIsParticleGenerationNeeded( false );
                                    }
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "表示中のオブジェクトがローカル最大ステップ以上ではないため最大ステップをインポートします。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( maxTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( maxTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepFinalにチェックがついていないため非表示にします。" << __LINE__;
                                object->hide();
                            }
                        }
                    }
                }
            }
            else //表示の要求無し
            {
                qInfo() << "表示の要求がないため非表示にします。" << __LINE__;
                object->hide();
            }
        }
        else //オブジェクトが表示されていない
        {
            qInfo() << "オブジェクトが非表示状態です。" << __LINE__;
            if( displayCheckBox->isChecked() ) //表示の要求がある。
            {
                qInfo() << "表示にチェックがついています。" << __LINE__;
                if( nextTimeStep != currentTimeStep ) //タイムステップの更新がある場合
                {
                    qInfo() << "タイムステップの更新があります。" << __LINE__;
                    if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep ) //次のステップがローカルファイルの範囲内である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲内です。" << __LINE__;
                        if( m_merge->m_files_manager[row]->getCurrentDisplayedStep() == nextTimeStep ) //表示中のオブジェクトが次のステップと一致する場合
                        {
                            qInfo() << "非表示中のオブジェクトと次のステップが一致しているため再表示します。"  << __LINE__;
                            object->show();
                            if( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObject && m_merge->getIsParticleGenerationNeeded() ) //サーバの更新が必要な場合
                            {
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                m_merge->setIsParticleGenerationNeeded( false );
                            }
                        }
                        else //一致しなかった場合
                        {
                            qInfo() << "非表示中のオブジェクトと次のステップが一致しないため次のステップをインポートし再表示します。" << __LINE__;
                            if constexpr (!std::is_same_v<Importer, void>)
                            {
//                                nextObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ); //次のステップのファイルを表示
                                m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            else
                            {
//                                nextObject = m_merge->m_connect->connect2( nextTimeStep );
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            m_merge->m_files_manager[row]->setCurrentDisplayedStep( nextTimeStep );
                        }
                    }
                    else //次のステップがローカルファイルの範囲外である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲外です。" << __LINE__;
                        if( nextTimeStep < minTimeStep ) //次のステップがローカルの最小ステップよりも低い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最小ステップよりも小さいです。" << __LINE__;
                            if( pattern == KeepInitialChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepInitialにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() <= minTimeStep ) //表示中のオブジェクトがローカル最小ステップ以下である場合。
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最小ステップ以下であるため再表示します。"  << __LINE__;
                                    object->show();
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最小ステップ以下ではないため最小ステップをインポートし再表示します。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( minTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( minTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepInitialにチェックがついていないため何もしません。" << __LINE__;
                            }
                        }
                        else if( nextTimeStep > maxTimeStep ) //次のステップがローカルの最大よりも高い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最大ステップよりも大きいです。" << __LINE__;
                            if( pattern == KeepFinalChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepFinalにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() >= maxTimeStep ) //表示中のオブジェクトがローカル最大ステップ以上である場合。
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最大ステップ以上であるため再表示します。"  << __LINE__;
                                    object->show();
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最大ステップ以上ではないため最大ステップをインポートし再表示します。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( maxTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( maxTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepFinalにチェックがついていないため何もしません。" << __LINE__;
                            }
                        }
                    }
                }
                else //タイムステップの更新がない場合
                {
                    qInfo() << "タイムステップの更新がありません。" << __LINE__;
                    if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep ) //次のステップがローカルファイルの範囲内である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲内です。" << __LINE__;
                        if( m_merge->m_files_manager[row]->getCurrentDisplayedStep() == nextTimeStep ) //表示中のオブジェクトが次のステップと一致する場合
                        {
                            qInfo() << "非表示中のオブジェクトと次のステップが一致しているため再表示します。"  << __LINE__;
                            object->show();
                        }
                        else //一致しなかった場合
                        {
                            qInfo() << "非表示中のオブジェクトと次のステップが一致しないため次のステップをインポートし再表示します。" << __LINE__;
                            if constexpr (!std::is_same_v<Importer, void>)
                            {
//                                nextObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ); //次のステップのファイルを表示
                                m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            else
                            {
//                                nextObject = m_merge->m_connect->connect2( nextTimeStep );
                                m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( nextTimeStep ) );
                                m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                            }
                            m_merge->m_files_manager[row]->setCurrentDisplayedStep( nextTimeStep );
                        }
                    }
                    else //次のステップがローカルファイルの範囲外である。
                    {
                        qInfo() << "次のタイムステップがローカルタイムステップの範囲外です。" << __LINE__;
                        if( nextTimeStep < minTimeStep ) //次のステップがローカルの最小ステップよりも低い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最小ステップよりも小さいです。" << __LINE__;
                            if( pattern == KeepInitialChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepInitialにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() <= minTimeStep ) //表示中のオブジェクトがローカル最小ステップ以下である場合。
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最小ステップ以下であるため再表示します。"  << __LINE__;
                                    object->show();
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最小ステップ以下ではないため最小ステップをインポートし再表示します。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( minTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( minTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( minTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepInitialにチェックがついていないため何もしません。" << __LINE__;
                            }
                        }
                        else if( nextTimeStep > maxTimeStep ) //次のステップがローカルの最大よりも高い場合
                        {
                            qInfo() << "次のタイムステップがローカルの最大ステップよりも大きいです。" << __LINE__;
                            if( pattern == KeepFinalChecked || pattern == BothChecked ) //KeepInitialにチェックがついている場合
                            {
                                qInfo() << "KeepFinalにチェックがついています。" << __LINE__;
                                if ( m_merge->m_files_manager[row]->getCurrentDisplayedStep() >= maxTimeStep ) //表示中のオブジェクトがローカル最大ステップ以上である場合。
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最大ステップ以上であるため再表示します。"  << __LINE__;
                                    object->show();
                                }
                                else //一致しなかった場合
                                {
                                    qInfo() << "非表示中のオブジェクトがローカル最大ステップ以上ではないため最大ステップをインポートし再表示します。" << __LINE__;
                                    if constexpr (!std::is_same_v<Importer, void>)
                                    {
//                                        nextObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ); //次のステップのファイルを表示
                                        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    else
                                    {
//                                        nextObject = m_merge->m_connect->connect2( maxTimeStep );
                                        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->connect2( maxTimeStep ) );
                                        m_merge->m_files_manager[row]->setIsReplacementNeeded( true );
                                    }
                                    m_merge->m_files_manager[row]->setCurrentDisplayedStep( maxTimeStep );
                                }
                            }
                            else
                            {
                                qInfo() << "KeepFinalにチェックがついていないため何もしません。" << __LINE__;
                            }
                        }
                    }
                }
            }
            else //表示の要求無し
            {
                qInfo() << "表示の要求がないため何もしません。" << __LINE__;
            }
        }

//        if ( nextObject != nullptr ) //オブジェクトが生成されていれば交換
//        {
//            qInfo() << "オブジェクトの生成があるため更新を行います。";
//            nextObject->setXform(before_object_manager_xform);
//            if constexpr (std::is_same_v<Importer, kvs::PolygonImporter>)
//            {
//                nextObject->setColor( kvs::RGBColor( m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue() ) );
//                nextObject->setOpacity( m_files_manager[row]->getOpacity() * 255 );
//            }
//            m_screen->scene()->replaceObject(m_files_manager[row]->getIds().first, nextObject);
//        }
//        else if (auto* polygonObject = static_cast<kvs::PolygonObject*>(object))
//        {
//            if constexpr (std::is_same_v<Importer, kvs::PolygonImporter>)
//            {
//                kvs::PolygonObject* copiedObject = new kvs::PolygonObject();
//                copiedObject->deepCopy( *polygonObject );
//                copiedObject->setColor(kvs::RGBColor(m_files_manager[row]->getRGBColor().red(), m_files_manager[row]->getRGBColor().green(), m_files_manager[row]->getRGBColor().blue()));
//                copiedObject->setOpacity(m_files_manager[row]->getOpacity() * 255);
//                m_screen->scene()->replaceObject(m_files_manager[row]->getIds().first, copiedObject);
//            }
//        }
    }    
}

QString MergePanel::WorkerThread::updateTimeStepInFileName(QString fileName, int nextTimeStep)
{
    // 正規表現パターン: 5桁の数字
    QRegularExpression regex(R"(\d{5})");
    QRegularExpressionMatch match = regex.match(fileName);

    if (match.hasMatch()) {
        // futureTimeの値を考慮して新しい5桁の数字を生成
        int newNumber = nextTimeStep;

        // 新しい5桁の数字をQStringに変換し、0埋めして格納
        QString extractedNumber = QString::number(newNumber).rightJustified(5, '0');

        // 5桁の数字を含む前後の文字列を抜き取り
        int startPos = match.capturedStart();
        int endPos = match.capturedEnd();

        return fileName.left(startPos) + extractedNumber + fileName.mid(endPos);        
    }
    else
    {
        return fileName;
    }
}

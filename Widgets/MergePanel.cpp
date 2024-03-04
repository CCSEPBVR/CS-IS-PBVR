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

MergePanel::MergePanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::MergePanel),
    m_files_manager(),
    m_time_control( nullptr ),
    m_current_time_step( -1 )
{
    ui->setupUi(this);
    connect(ui->filesTWidget, &QTableWidget::cellDoubleClicked, this, &MergePanel::onFilesTWidgetCellDoubleClicked);
    connect(ui->importFilesBrowsePBtn, &QPushButton::clicked, this, &MergePanel::onBrowserButtonClicked );
    connect(ui->importFilesAddPBtn, &QPushButton::clicked, this, &MergePanel::onAddButtonClicked );
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
    fileDialog.setNameFilter("*.kvsml *.stl *.3ds *.fbx *.las");
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
    checkFileFormat( newFile );    
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
    else if( fileSuffix == "3ds" )
    {
        newFile->setFormat( FilesManager::TexturedPolygonObject3DS );
    }
    else if( fileSuffix == "fbx" )
    {
        newFile->setFormat( FilesManager::TexturedPolygonObjectFBX );
    }
    else if( fileSuffix == "las" )
    {
        newFile->setFormat( FilesManager::PointObjectLAS );
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

void MergePanel::onApplyButtonClicked()
{
    removeChecked();
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
            delete m_files_manager[row];
            m_files_manager.removeAt(row);

            row--; // 行が削除されたので、ループのインデックスを調整する
        }
    }
    calculateTotalMinMaxTimeStep();
}

void MergePanel::mergeObjects()
{
    kvs::Xform before_object_manager_xform = m_screen->scene()->objectManager()->xform();
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        QCheckBox *displayCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 0 ) );
        if( m_files_manager[row]->getIds().first == -1 && m_files_manager[row]->getIds().second == -1 )
        {
            kvs::PolygonObject* pobj = new kvs::PolygonImporter( m_files_manager[row]->getFileInfo().filePath().toStdString() );
            pobj->setXform( before_object_manager_xform );
            kvs::StochasticPolygonRenderer* sprnd = new kvs::StochasticPolygonRenderer();
            m_files_manager[row]->setIds( m_screen->scene()->registerObject( pobj, sprnd ) );
            displayCheckBox->checkState() == Qt::Checked ? pobj->setVisible(true) : pobj->setVisible(false);
        }
        else
        {
            auto* object = m_screen->scene()->object( m_files_manager[row]->getIds().first );
            displayCheckBox->checkState() == Qt::Checked ? object->setVisible(true) : object->setVisible(false);
        }
        auto* object = m_screen->scene()->object( m_files_manager[row]->getIds().first );
    }
    m_screen->update();
}



void MergePanel::onCenteringButtonClicked()
{
    CustomObjectManager* object_manager = static_cast<CustomObjectManager*>( m_screen->scene()->objectManager() );
    kvs::Vec3 min_obj;
    kvs::Vec3 max_obj;
    int counter = 0;
    for (int row = 0; row < m_files_manager.size() && counter < 2; row++)
    {
        if (m_files_manager[row]->getIds().first != -1 && m_files_manager[row]->getIds().second != -1)
        {
            auto* object = m_screen->scene()->object(m_files_manager[row]->getIds().first);
            if (object->isVisible())
            {
                counter++;
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

}

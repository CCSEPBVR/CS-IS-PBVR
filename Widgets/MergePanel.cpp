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
            removeObject( m_files_manager[row] );
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
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        kvs::ObjectBase* object = nullptr;
        if( m_files_manager[row]->getFormat() == FilesManager::PointObjectKVSML )
        {
            object = selectPattern<kvs::PointImporter, kvs::PointObject>( m_files_manager[row], row );
        }
        else if(   m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML ||
                   m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
        {
            object = selectPattern<kvs::PolygonImporter, kvs::PolygonObject>( m_files_manager[row], row );
        }
        else if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObject )
        {
            object = selectPattern( m_files_manager[row], row );
        }
        if( object != nullptr )
        {
            updateObject( m_files_manager[row], object );
        }
    }
    m_current_time_step = m_time_control->getNextTimeStep();
    m_time_control->setCurrentTimeStep( m_current_time_step );
    m_preference->setCurrentTimeStep( m_current_time_step );
    m_preference->loadShadingSettings();
    m_preference->applyShadingSettings();
    m_screen->redraw();

    totalParticles();
}

template <typename Importer, typename ObjectType>
ObjectType* MergePanel::selectPattern( FilesManager* filesManager, int row )
{
    QCheckBox *displayCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 0 ) );
    QCheckBox *keepInitialCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 1 ) );
    QCheckBox *keepFinalCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 2 ) );
    if( displayCheckBox->checkState() == Qt::Checked )//Displayにチェックが付いている場合
    {
        if( keepInitialCheckBox->checkState() == Qt::Checked)//KeepInitialにチェックがついている場合
        {
            if( keepFinalCheckBox->checkState() == Qt::Checked )//KeepFinalにチェックがついている場合
            {
                qInfo() << "both";
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, BothChecked );
            }
            else//KeepFinalにチェックがついていない場合
            {
                qInfo() << "only Init";
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, KeepInitialChecked );
            }
        }
        else//KeepInitialにチェックがついていない場合
        {
            if( keepFinalCheckBox->checkState() == Qt::Checked )//KeepFinalにチェックがついている場合
            {
                qInfo() << "only Final";
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, KeepFinalChecked );
            }
            else//KeepFinalにチェックがついていない場合
            {
                qInfo() << "None";
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, NoneChecked );
            }
        }
    }
    else
    {
        removeObject( filesManager );
    }
    return nullptr;
}

template <typename Importer, typename ObjectType>
ObjectType* MergePanel::timeStepCheckAndImport( FilesManager* filesManager, CheckBoxPattern pattern )
{
    ObjectType* importObject = nullptr;
    const int minTimeStep = filesManager->getMinTimeStep();
    const int maxTimeStep = filesManager->getMaxTimeStep();
//    const int currentTimeStep = m_time_control->getCurrentTimeStep();
    const int nextTimeStep = m_time_control->getNextTimeStep();
    const QString filePath = filesManager->getFileInfo().filePath();
    const bool already_registerd = (filesManager->getIds().first == 0 && filesManager->getIds().second == 0) ? false : true;
    const QString fileName = filesManager->getFileInfo().baseName().left( filesManager->getFileInfo().baseName().indexOf('_') );

    if( already_registerd == false )
    {
        if( pattern == KeepInitialChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importObject = nullptr;
            }
        }

        if( pattern == KeepFinalChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importObject = nullptr;
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
            }
        }

        if( pattern == BothChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
            }
        }

        if( pattern == NoneChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importObject = nullptr;
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importObject = nullptr;
            }
        }
    }

    if( already_registerd == true )
    {
        if( pattern == KeepInitialChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    if( nextTimeStep < minTimeStep )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importObject = importObject = nullptr;
                    }
                    else if( nextTimeStep > maxTimeStep )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importObject = importObject = nullptr;
                    }
                    else if( nextTimeStep == minTimeStep )
                    {
                        if(m_current_time_step > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( nextTimeStep == maxTimeStep )
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else if( nextTimeStep < minTimeStep )//
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( m_current_time_step > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

        if( pattern == KeepFinalChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    if( nextTimeStep < minTimeStep )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importObject = importObject = nullptr;
                    }
                    else if( nextTimeStep > maxTimeStep )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importObject = importObject = nullptr;
                    }
                    else if( nextTimeStep == minTimeStep )
                    {
                        if(m_current_time_step > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( nextTimeStep == maxTimeStep )
                    {
                        if(m_current_time_step < nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                if( m_current_time_step < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

        if( pattern == BothChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    if (nextTimeStep < minTimeStep)
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importObject = importObject = nullptr;
                    }
                    else if( nextTimeStep > maxTimeStep )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importObject = importObject = nullptr;
                    }
                    else if( nextTimeStep == minTimeStep )
                    {
                        if(m_current_time_step > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( nextTimeStep == maxTimeStep )
                    {
                        if(m_current_time_step < nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else if( nextTimeStep < minTimeStep )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( m_current_time_step > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else if( nextTimeStep > maxTimeStep )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( m_current_time_step < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

        if( pattern == NoneChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                    importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
                }
                else
                {
                    if( ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML && filesManager->getIsModified() == true ) ||
                        ( filesManager->getFormat() == FilesManager::NonTexturedPolygonObjectSTL && filesManager->getIsModified() == true ) )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                    }
                }
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

    }
    return importObject;
}

kvs::PointObject* MergePanel::selectPattern( FilesManager* filesManager, int row )
{
    QCheckBox *displayCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 0 ) );
    QCheckBox *keepInitialCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 1 ) );
    QCheckBox *keepFinalCheckBox = qobject_cast<QCheckBox*>( ui->filesTWidget->cellWidget( row, 2 ) );
    if( displayCheckBox->checkState() == Qt::Checked )//Displayにチェックが付いている場合
    {
        if( keepInitialCheckBox->checkState() == Qt::Checked)//KeepInitialにチェックがついている場合
        {
            if( keepFinalCheckBox->checkState() == Qt::Checked )//KeepFinalにチェックがついている場合
            {
                qInfo() << "both";
                return timeStepCheckAndImport( filesManager, BothChecked );
            }
            else//KeepFinalにチェックがついていない場合
            {
                qInfo() << "only Init";
                return timeStepCheckAndImport( filesManager, KeepInitialChecked );
            }
        }
        else//KeepInitialにチェックがついていない場合
        {
            if( keepFinalCheckBox->checkState() == Qt::Checked )//KeepFinalにチェックがついている場合
            {
                qInfo() << "only Final";
                return timeStepCheckAndImport( filesManager, KeepFinalChecked );
            }
            else//KeepFinalにチェックがついていない場合
            {
                qInfo() << "None";
                return timeStepCheckAndImport( filesManager, NoneChecked );
            }
        }
    }
    else
    {
        removeObject( filesManager );
    }
    return nullptr;

}

kvs::PointObject* MergePanel::timeStepCheckAndImport( FilesManager* filesManager, CheckBoxPattern pattern )
{
    kvs::PointObject* importedObject = nullptr;
    const int minTimeStep  = filesManager->getMinTimeStep();
    const int maxTimeStep  = filesManager->getMaxTimeStep();
    const int nextTimeStep = m_time_control->getNextTimeStep();
    //    const int currentTimeStep = m_time_control->getCurrentTimeStep();
    const QString filePath = filesManager->getFileInfo().filePath();
    const bool already_registerd = (filesManager->getIds().first == 0 && filesManager->getIds().second == 0) ? false : true;;

    if( already_registerd == false )
    {
        if( pattern == KeepInitialChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( nextTimeStep );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( minTimeStep );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = importedObject = nullptr;
            }
        }

        if( pattern == KeepFinalChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( nextTimeStep );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( maxTimeStep );
            }
        }

        if( pattern == BothChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( nextTimeStep );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( minTimeStep );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( maxTimeStep );
            }
        }

        if( pattern == NoneChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = m_connect->connect2( nextTimeStep );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
        }
    }

    if( already_registerd == true )
    {
        if( pattern == KeepInitialChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    if( nextTimeStep < minTimeStep )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( nextTimeStep > maxTimeStep )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( nextTimeStep == minTimeStep )
                    {
                        if(m_current_time_step > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = m_connect->connect2( nextTimeStep );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( nextTimeStep == maxTimeStep )
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
                    }
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( nextTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else if( nextTimeStep < minTimeStep )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( m_current_time_step > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( minTimeStep );
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( minTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

        if( pattern == KeepFinalChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    if( nextTimeStep < minTimeStep )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( nextTimeStep > maxTimeStep )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( nextTimeStep == minTimeStep )
                    {
                        if(m_current_time_step > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = m_connect->connect2( nextTimeStep );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( nextTimeStep == maxTimeStep )
                    {
                        if(m_current_time_step < nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = m_connect->connect2( nextTimeStep );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
                    }
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( nextTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                if( m_current_time_step < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( maxTimeStep );
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( maxTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

        if( pattern == BothChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    if (nextTimeStep < minTimeStep)
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( nextTimeStep > maxTimeStep )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( nextTimeStep == minTimeStep )
                    {
                        if(m_current_time_step > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = m_connect->connect2( nextTimeStep );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( nextTimeStep == maxTimeStep )
                    {
                        if(m_current_time_step < nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = m_connect->connect2( nextTimeStep );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
                    }
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( nextTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else if( nextTimeStep < minTimeStep )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( m_current_time_step > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( minTimeStep );
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( minTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else if( nextTimeStep > maxTimeStep )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( m_current_time_step < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( maxTimeStep );
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( maxTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

        if( pattern == NoneChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                if( nextTimeStep != m_current_time_step )
                {
                    qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( nextTimeStep );
                }
                else
                {
//                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
//                    {
//                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
//                        importedObject = m_connect->connect2( nextTimeStep );
//                    }
//                    else
//                    {
//                        qInfo() << "Does nothing.[" << __LINE__ << "]";
//                    }
                }
            }
            else
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }

    }
    return importedObject;
}

QString MergePanel::updateTimeStepInFileName(QString fileName, int nextTimeStep) {
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

void MergePanel::removeObject( FilesManager* filesManager )
{
    //Sceneにオブジェクトが登録されている場合
    if( filesManager->getIds().first != 0 && filesManager->getIds().second != 0 )
    {
        m_screen->scene()->IDManager()->erase(filesManager->getIds().first, filesManager->getIds().second);
        filesManager->setIds(std::pair<int, int>(0, 0));
    }
}

void MergePanel::updateObject( FilesManager* filesManager, kvs::ObjectBase* object )
{
    object->setXform( m_screen->scene()->objectManager()->xform() );
    //Sceneにオブジェクトが登録されていない場合
    if( filesManager->getIds().first == 0 && filesManager->getIds().second == 0 )
    {
        kvs::RendererBase* renderer = nullptr;

        if( dynamic_cast<kvs::PointObject*>(object) != nullptr )
        {
            renderer = new kvs::glsl::ParticleBasedRenderer();
        }
        else if( dynamic_cast<kvs::PolygonObject*>(object) != nullptr )
        {
            kvs::PolygonObject* polygonObject = dynamic_cast<kvs::PolygonObject*>(object);
            polygonObject->setColor(kvs::RGBColor(filesManager->getRGBColor().red(), filesManager->getRGBColor().green(), filesManager->getRGBColor().blue()));
            polygonObject->setOpacity(filesManager->getOpacity() * 255);
            renderer = new kvs::StochasticPolygonRenderer();
        }
        filesManager->setIds(m_screen->registerObject(object, renderer));
    }
    //Sceneにオブジェクトが登録されている場合
    else
    {
        if( dynamic_cast<kvs::PolygonObject*>(object) != nullptr )
        {
            kvs::PolygonObject* polygonObject = dynamic_cast<kvs::PolygonObject*>(object);
            polygonObject->setColor(kvs::RGBColor(filesManager->getRGBColor().red(), filesManager->getRGBColor().green(), filesManager->getRGBColor().blue()));
            polygonObject->setOpacity(filesManager->getOpacity() * 255);
        }
        m_screen->scene()->replaceObject(filesManager->getIds().first, object);
    }
}

void MergePanel::serverObject( QString volumeDataFilePath, int min, int max )
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setFormat( FilesManager::ServerPointObject );
    addRowToFilesTableWidget( newFile );
    m_files_manager.append( newFile );
    calculateTotalMinMaxTimeStep();
}

void MergePanel::totalParticles()
{
    int totalParticles = 0;
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        const bool already_registerd = ( m_files_manager[row]->getIds().first == 0 && m_files_manager[row]->getIds().second == 0 ) ? false : true;
        if( already_registerd )
        {
            qInfo() << m_files_manager[row]->getFormat();
            if( m_files_manager[row]->getFormat() == FilesManager::PointObjectKVSML || m_files_manager[row]->getFormat() == FilesManager::ServerPointObject )
            {
                totalParticles += dynamic_cast<kvs::PointObject*>( m_screen->scene()->object( m_files_manager[row]->getIds().first) )->numberOfVertices();
            }
        }
    }
    m_data_summary->setTotalParticles( totalParticles );
}

void MergePanel::updatePolygonColorOpacity()
{
    for( int row = 0; row < m_files_manager.size(); row++ )
    {

        if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML || m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
        {
            QTableWidgetItem* colorItem = ui->filesTWidget->item(row, 4);
            QDoubleSpinBox* opacitySpinBox = dynamic_cast<QDoubleSpinBox*>(ui->filesTWidget->cellWidget(row, 5));
            if( colorItem != nullptr && opacitySpinBox != nullptr )
            {
                qInfo() << "1";
                m_files_manager[row]->setIsModified( false );
                QColor rgbColor = colorItem->background().color();
                double opacity = opacitySpinBox->value();
                if( m_files_manager[row]->getRGBColor() != rgbColor )
                {
                    qInfo() << "1";
                    m_files_manager[row]->setRGBColor( rgbColor );
                    m_files_manager[row]->setIsModified( true );
                }
                if( m_files_manager[row]->getOpacity() != opacity )
                {
                    qInfo() << "1";
                    m_files_manager[row]->setOpacity( opacity );
                    m_files_manager[row]->setIsModified( true );
                }
            }
        }
    }
}

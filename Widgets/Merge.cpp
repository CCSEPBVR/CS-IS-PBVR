// .cpp
#include "Merge.h"
#include "ui_Merge.h"

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QColorDialog>
#include <QCheckBox>

#include <kvs/ObjectManager>
#include <kvs/RendererManager>
#include <kvs/PointImporter>
#include <kvs/PointObject>
#include <kvs/ParticleBasedRenderer>

#include <kvs/IDManager>
#include <kvs/PolygonImporter>
#include <kvs/PolygonObject>
#include <kvs/StochasticPolygonRenderer>

Merge::Merge(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Merge)
{
    ui->setupUi(this);
    connect(ui->filesTWidget, &QTableWidget::cellDoubleClicked, this, &Merge::onFilesTWidgetCellDoubleClicked);
    connect(ui->importFilesBrowsePBtn, &QPushButton::clicked, this, &Merge::onBrowserButtonClicked);
    connect(ui->importFilesAddPBtn, &QPushButton::clicked, this, &Merge::onAddButtonClicked);
    connect(ui->applyBtn, &QPushButton::clicked, this, &Merge::onApplyButtonClicked);
}

Merge::~Merge()
{
    qDeleteAll(m_files_manager);
    delete ui;
}

void Merge::registerFile( FilesManager* filesManager )
{
    headerLabels << filesManager->getFileName();
    ui->filesTWidget->setRowCount( ui->filesTWidget->rowCount() + 1);
    ui->filesTWidget->setVerticalHeaderLabels( headerLabels );


    QCheckBox *displayCheckBox = new QCheckBox();
    displayCheckBox->setCheckState ( filesManager->getVisible() ) ;

    QCheckBox *keepInitialCheckBox = new QCheckBox();
    keepInitialCheckBox->setCheckState ( filesManager->getKeepInitial() ) ;

    QCheckBox *keepFinalCheckBox = new QCheckBox();
    keepFinalCheckBox->setCheckState ( filesManager->getKeepFinal()  ) ;

    QTableWidgetItem* format = new QTableWidgetItem;
    format->setFlags( format->flags() & ~Qt::ItemIsEditable );
    format->setText( filesManager->getFileSuffix() );

    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon )
    {
        QTableWidgetItem *colorValue = new QTableWidgetItem;
        colorValue->setFlags( format->flags() & ~Qt::ItemIsEditable );
        colorValue->setBackground( filesManager->getRGBColor() );

        QDoubleSpinBox *opacityValue = new QDoubleSpinBox();
        opacityValue->setRange( 0.0, 1.0 );
        opacityValue->setSingleStep( 0.01 );
        opacityValue->setValue( filesManager->getOpacity() );

        ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 4, colorValue);
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
    deleteCheckBox->setCheckState ( Qt::Unchecked ) ;

    ui->filesTWidget->setCellWidget(ui->filesTWidget->rowCount() - 1, 0, displayCheckBox);
    ui->filesTWidget->setCellWidget(ui->filesTWidget->rowCount() - 1, 1, keepInitialCheckBox);
    ui->filesTWidget->setCellWidget(ui->filesTWidget->rowCount() - 1, 2, keepFinalCheckBox);
    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 3, format );
    ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 6, deleteCheckBox );
}

void Merge::onBrowserButtonClicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter("*.kvsml *.stl *.3ds *.fbx *.las");

    if (fileDialog.exec())
    {
        QString selectedFilePath = fileDialog.selectedFiles().at(0);
        ui->importFilesPathLEdit->setText(selectedFilePath);
    }
}

void Merge::onAddButtonClicked()
{
    QFileInfo fileInfo(ui->importFilesPathLEdit->text());
    QDir directory(fileInfo.absolutePath());

    FilesManager *newFilesManager = new FilesManager;
    checkMinMaxTimeStep(&fileInfo, &directory, newFilesManager);
    checkFileFormat(&fileInfo, newFilesManager);
    registerFile( newFilesManager );

    // 新しい FilesManager を m_files_manager に追加
    m_files_manager.append(newFilesManager);
    calculateMinMaxTimeStep();
}

void Merge::checkMinMaxTimeStep(QFileInfo *fileInfo, QDir *directory, FilesManager *filesManager)
{
    QRegularExpression regex(fileInfo->baseName().left(fileInfo->baseName().indexOf('_')) + "_([0-9]+)\\.*");
    int minValue = std::numeric_limits<int>::max();
    int maxValue = std::numeric_limits<int>::min();

    foreach (const QFileInfo &fileInfo, directory->entryInfoList(QDir::Files))
    {
        QString fileName = fileInfo.fileName();
        QRegularExpressionMatch match = regex.match(fileName);

        if (match.hasMatch())
        {
            QString numericalPart = match.captured(1);
            int numericalValue = numericalPart.toInt();

            if (numericalValue < minValue)
            {
                minValue = numericalValue;
            }

            if (numericalValue > maxValue)
            {
                maxValue = numericalValue;
            }
        }
    }

    if (minValue != std::numeric_limits<int>::max() && maxValue != std::numeric_limits<int>::min())
    {
        //        qInfo() << fileInfo->baseName().left(fileInfo->baseName().indexOf('_'));
        //        qInfo() << minValue;
        //        qInfo() << maxValue;

        // 新しい FilesManager に最小最大値をセット
        filesManager->setFileInfo( *fileInfo );
        filesManager->setFileName( fileInfo->baseName().left(fileInfo->baseName().indexOf('_')) );
        filesManager->setMinTimeStep(minValue);
        filesManager->setMaxTimeStep(maxValue);
    }
    else
    {
        qInfo() << "該当する数字が見つかりませんでした。";
    }
}

void Merge::checkFileFormat(QFileInfo *fileInfo,  FilesManager *filesManager)
{
    QFile file(fileInfo->filePath());
    QXmlStreamReader xml(&file);
    QString fileSuffix = fileInfo->suffix().toLower();
    filesManager->setFileFormat( FilesManager::Unknown );

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "ファイルを開けませんでした。";
        return;
    }

    if (fileSuffix == "kvsml")
    {
        while (!xml.atEnd() && !xml.hasError())
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::StartElement)
            {
                QString tagName = xml.name().toString();
                if (tagName == "PointObject")
                {
                    qDebug() << "KVSML(PointObject)です。";
                    filesManager->setFileFormat( FilesManager::PointObject );
                    filesManager->setFileSuffix( "KVSML(PointObject)" );
                    break;
                }
                else if (tagName == "PolygonObject")
                {
                    qDebug() << "KVSML(PolygonObject)です。";
                    filesManager->setFileFormat( FilesManager::NonTexturedPolygon );
                    filesManager->setFileSuffix( "KVSML(PolygonObject)" );
                    filesManager->setRGBColor( QColor(128, 128, 128));
                    filesManager->setOpacity( 0.5 );
                    break;
                }
            }
        }

        if (xml.hasError())
        {
            qDebug() << "XML パースエラー: " << xml.errorString();
        }
    }
    else if (fileSuffix == "stl")
    {
        qDebug() << "stlファイルです.";
        filesManager->setFileFormat( FilesManager::NonTexturedPolygon);
        filesManager->setFileSuffix( "stl" );
        filesManager->setRGBColor( QColor(128, 128, 128));
        filesManager->setOpacity( 0.5 );

    }
    else if (fileSuffix == "3ds")
    {
        qDebug() << "3dsファイルです.";
        filesManager->setFileFormat( FilesManager::TexturedPolygon);
        filesManager->setFileSuffix( "3ds" );

    }
    else if (fileSuffix == "fbx")
    {
        qDebug() << "fbxファイルです.";
        filesManager->setFileFormat( FilesManager::TexturedPolygon );
        filesManager->setFileSuffix( "fbx" );

    }
    else if (fileSuffix == "las")
    {
        qDebug() << "lasファイルです.";
        filesManager->setFileFormat( FilesManager::PointObject );
        filesManager->setFileSuffix( "las" );

    }
    else
    {
        qDebug() << "サポートされていないフォーマットです。";
    }
    filesManager->setVisible( Qt::CheckState::Checked );
    filesManager->setKeepInitial( Qt::CheckState::Unchecked );
    filesManager->setKeepFinal( Qt::CheckState::Unchecked );
}

void Merge::onApplyButtonClicked()
{
    removeChecker();
    mergeObjects();
    //    showFilesManager();
}

void Merge::removeChecker()
{
    // 削除する行のインデックスを保存するリスト
    QList<int> rowsToRemove;

    // m_files_manager に保持されている各 FilesManager の情報を表示
    for (int row = 0; row < ui->filesTWidget->rowCount(); ++row)
    {
        // Delete チェックボックスの状態を確認し、チェックされている行を削除リストに追加
        QCheckBox* deleteCheckBox = dynamic_cast<QCheckBox*>(ui->filesTWidget->cellWidget(row, 6));
        if (deleteCheckBox && deleteCheckBox->isChecked())
        {
            rowsToRemove.append(row);
        }
    }

    // 削除リストにある行を後ろから削除
    for (int i = rowsToRemove.size() - 1; i >= 0; --i)
    {
        int rowToRemove = rowsToRemove[i];
        // 対応する FilesManager オブジェクトを削除
        FilesManager* filesManager = m_files_manager.takeAt(rowToRemove);
        delete filesManager;

        //削除する行のオブジェクトがスクリーンに登録されて場合削除する。
        if( filesManager->getIds().first != 0 && filesManager->getIds().second != 0)
        {
            m_screen->scene()->IDManager()->erase(filesManager->getIds().first,filesManager->getIds().second);
            filesManager->setIds(std::pair<int,int>(0,0));
        }

        // 行を削除
        ui->filesTWidget->removeRow(rowToRemove);
    }

    // 削除後にデータの更新を行う関数を呼び出す
    updateFiles();
    calculateMinMaxTimeStep();
}

void Merge::updateFiles()
{
    // m_files_manager に保持されている各 FilesManager の情報を表示
    for (int row = 0; row < ui->filesTWidget->rowCount(); ++row)
    {
        // 対応する FilesManager オブジェクトを取得
        FilesManager* filesManager = m_files_manager[row];
        filesManager->setIsModified( false );

        // Is Visible、Keep Initial、Keep Final は CheckBox から取得
        QCheckBox* isVisibleCheckBox = dynamic_cast<QCheckBox*>(ui->filesTWidget->cellWidget(row, 0));
        QCheckBox* isKeepInitialCheckBox = dynamic_cast<QCheckBox*>(ui->filesTWidget->cellWidget(row, 1));
        QCheckBox* isKeepFinalCheckBox = dynamic_cast<QCheckBox*>(ui->filesTWidget->cellWidget(row, 2));

        if (isVisibleCheckBox && isKeepInitialCheckBox && isKeepFinalCheckBox)
        {
            Qt::CheckState isVisible = static_cast<Qt::CheckState>(isVisibleCheckBox->isChecked());
            Qt::CheckState isKeepInitial = static_cast<Qt::CheckState>(isKeepInitialCheckBox->isChecked());
            Qt::CheckState isKeepFinal = static_cast<Qt::CheckState>(isKeepFinalCheckBox->isChecked());

            filesManager->setVisible(isVisible);
            filesManager->setKeepInitial(isKeepInitial);
            filesManager->setKeepFinal(isKeepFinal);
        }

        // RGB Color と Opacity は NonTexturedPolygon の場合にのみ更新
        if (filesManager->getFileFormat() == FilesManager::NonTexturedPolygon)
        {
            QTableWidgetItem* colorItem = ui->filesTWidget->item(row, 4);
            QDoubleSpinBox* opacitySpinBox = dynamic_cast<QDoubleSpinBox*>(ui->filesTWidget->cellWidget(row, 5));

            if (colorItem != nullptr && opacitySpinBox != nullptr)
            {
                QColor rgbColor = colorItem->background().color();
                double opacity = opacitySpinBox->value();

                if( filesManager->getRGBColor() != rgbColor)
                {
                    filesManager->setRGBColor(rgbColor);
                    filesManager->setIsModified( true );
                }

                if( filesManager->getOpacity() != opacity)
                {
                    filesManager->setOpacity(opacity);
                    filesManager->setIsModified( true );
                }
            }
        }
    }
}

void Merge::showFilesManager()
{
    for (int i = 0; i < m_files_manager.size(); ++i)
    {
        FilesManager* filesManager = m_files_manager[i];

        qInfo() << "=== FilesManager Contents ===";
        qInfo() << "Index: " << i;
        qInfo() << "    File Name: " << filesManager->getFileName();
        qInfo() << "Min Time Step: " << filesManager->getMinTimeStep();
        qInfo() << "Max Time Step: " << filesManager->getMaxTimeStep();
        qInfo() << "Is Visible: " << filesManager->getVisible();
        qInfo() << "Keep Initial: " << filesManager->getKeepInitial();
        qInfo() << "Keep Final: " << filesManager->getKeepFinal();
        qInfo() << "File Suffix: " << filesManager->getFileSuffix();
        qInfo() << "File Format: " << filesManager->getFileFormat();

        // RGB Color と Opacity は NonTexturedPolygon の場合にのみ出力
        if (filesManager->getFileFormat() == FilesManager::NonTexturedPolygon)
        {
            qInfo() << "RGB Color: " << filesManager->getRGBColor();
            qInfo() << "Opacity: " << filesManager->getOpacity();
        }

        qInfo() << "=============================";
    }
}

void Merge::onFilesTWidgetCellDoubleClicked(int row, int column)
{
    if (column == 4) // Colorのセルをダブルクリックしているか。
    {
        FilesManager* filesManager = m_files_manager.value(row, nullptr);

        if (filesManager != nullptr && filesManager->getFileFormat() == FilesManager::NonTexturedPolygon)
        {
            QTableWidgetItem* formatItem = ui->filesTWidget->item( row, 4 );
            formatItem->setBackground(QColorDialog::getColor(Qt::gray));
        }
    }
}


QString Merge::updateTimeStepInFileName(QString fileName, int nextTimeStep) {
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

void Merge::calculateMinMaxTimeStep()
{
    int overallMinTimeStep = INT_MAX;
    int overallMaxTimeStep = INT_MIN;
    bool isSingleObject = true;

    for (int i = 0; i < m_files_manager.size(); i++)
    {
        FilesManager* filesManager = m_files_manager[i];
        int minTimeStep = filesManager->getMinTimeStep();
        int maxTimeStep = filesManager->getMaxTimeStep();

        overallMinTimeStep = std::min(overallMinTimeStep, minTimeStep);
        overallMaxTimeStep = std::max(overallMaxTimeStep, maxTimeStep);
        if(i >= 1)
        {
            isSingleObject = false;
        }
    }
    m_time_control->updateTimeStepMinMax( overallMinTimeStep, overallMaxTimeStep, isSingleObject );
}

void Merge::mergeObjects()
{
    //FilesManagerに登録されているitem分ループ
    for( FilesManager* filesManager : m_files_manager )
    {
        //Vosibleにチェックボックスがついている場合
        if( filesManager->getVisible() == Qt::PartiallyChecked )
        {
            kvs::ObjectBase* object = nullptr;
            //PointObjectの場合
            if( filesManager->getFileFormat() == FilesManager::PointObject )
            {
                object = import<kvs::PointImporter, kvs::PointObject>(filesManager, selectPattern( filesManager ) );
            }
            //PolygonObjectの場合
            else if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon )
            {
                object = import<kvs::PolygonImporter, kvs::PolygonObject>(filesManager, selectPattern( filesManager));
            }

            //オブジェクトがnullptrではない場合
            if( object != nullptr )
            {
                updateObject( filesManager, object );
            }
        }
        else
        {
            removeObject( filesManager );
        }
    }

    currentTimeStep = m_time_control->getNextTimeStep();
    m_time_control->setCurrentTimeStep( currentTimeStep );
    m_screen->redraw();
}

// ┌──────────────────────────────────────────────────────────┐
// │ If KeepInitial is partially checked:                     │
// │    ┌──────────────────────────────────────────────────┐  │
// │    │ If KeepFinal is partially checked:               │  │
// │    │    Return Merge::A                               │  │
// │    └──────────────────────────────────────────────────┘  │
// │    ┌──────────────────────────────────────────────────┐  │
// │    │ If KeepFinal is not partially checked:           │  │
// │    │    Return Merge::B                               │  │
// │    └──────────────────────────────────────────────────┘  │
// └──────────────────────────────────────────────────────────┘
// │ If KeepInitial is not partially checked:                 │
// │    ┌──────────────────────────────────────────────────┐  │
// │    │ If KeepFinal is partially checked:               │  │
// │    │    Return Merge::C                               │  │
// │    └──────────────────────────────────────────────────┘  │
// │    ┌──────────────────────────────────────────────────┐  │
// │    │ If KeepFinal is not partially checked:           │  │
// │    │    Return Merge::D                               │  │
// │    └──────────────────────────────────────────────────┘  │
// └──────────────────────────────────────────────────────────┘
Merge::pattern Merge::selectPattern(FilesManager* filesManager)
{
    //KeepInitialにチェックがついている場合
    if( filesManager->getKeepInitial() == Qt::PartiallyChecked )
    {
        //KeepFinalにチェックがついている場合
        if( filesManager->getKeepFinal() == Qt::PartiallyChecked )
        {
            return Merge::BothChecked;
        }
        //KeepFinalにチェックがついていない場合
        else
        {
            return Merge::KeepInitialChecked;
        }
    }
    //KeepInitialにチェックがついていない場合
    else
    {
        //KeepFinalにチェックがついている場合
        if( filesManager->getKeepFinal() == Qt::PartiallyChecked )
        {
            return Merge::KeepFinalChecked;
        }
        //KeepFinalにチェックがついていない場合
        else
        {
            return Merge::NoneChecked;
        }
    }
}

template <typename Importer, typename ObjectType>
ObjectType* Merge::import(FilesManager* filesManager, pattern pattern)
{
    ObjectType* importedObject = nullptr;
    int minTimeStep  = filesManager->getMinTimeStep();
    int maxTimeStep  = filesManager->getMaxTimeStep();
    int nextTimeStep = m_time_control->getNextTimeStep();
    QString filePath = filesManager->getFileInfo().filePath();
    bool already_registerd = (filesManager->getIds().first == 0 && filesManager->getIds().second == 0) ? false : true;;

    //オブジェクトが登録されていない場合
    if( already_registerd == false )
    {
        //次のタイムステップがファイルの最小最大タイムステップの範囲内である場合
        if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
        {
            if( pattern == Merge::KeepInitialChecked )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
            }
            else if( pattern == Merge::KeepFinalChecked )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
            }
            else if( pattern == Merge::BothChecked )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
            }
            else if( pattern == Merge::NoneChecked )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
            }
        }
        //次のタイムステップがファイルの最小タイムステップよりも小さい場合
        else if( nextTimeStep < minTimeStep)
        {
            if( pattern == Merge::KeepInitialChecked )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
            }
            else if( pattern == Merge::KeepFinalChecked )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
            else if( pattern == Merge::BothChecked )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
            }
            else if( pattern == Merge::NoneChecked )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
        }
        //次のタイムステップがファイルの最大タイムステップよりも大きい場合
        else if( nextTimeStep > maxTimeStep )
        {
            if( pattern == Merge::KeepInitialChecked )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = importedObject = nullptr;
            }
            else if( pattern == Merge::KeepFinalChecked )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
            }
            else if( pattern == Merge::BothChecked )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
            }
            else if( pattern == Merge::NoneChecked )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
        }
    }

    //既に登録されている場合
    if( already_registerd == true )
    {
        if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
        {
            //次のタイムステップと現在表示されているタイムステップが異なる場合
            if( nextTimeStep != currentTimeStep )
            {
                //次のタイムステップがファイルの最小タイムステップよりも小さくKeepInitialにチェックがついている場合
                if (nextTimeStep < minTimeStep)
                {
                    if( pattern == Merge::KeepInitialChecked )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( pattern == Merge::KeepFinalChecked )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( pattern == Merge::BothChecked )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( pattern == Merge::NoneChecked )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                }
                else if( nextTimeStep > maxTimeStep )
                {
                    if( pattern == Merge::KeepInitialChecked )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( pattern == Merge::KeepFinalChecked )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( pattern == Merge::BothChecked )
                    {
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                    else if( pattern == Merge::NoneChecked )
                    {
                        //この条件に入ることはないかもしれません。
                        qInfo() << "Does nothing.[" << __LINE__ << "]";
                        importedObject = importedObject = nullptr;
                    }
                }
                else if( nextTimeStep == minTimeStep )
                {
                    if( pattern == Merge::KeepInitialChecked )
                    {
                        if(currentTimeStep > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( pattern == Merge::KeepFinalChecked )
                    {
                        if(currentTimeStep > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( pattern == Merge::BothChecked )
                    {
                        if(currentTimeStep > nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( pattern == Merge::NoneChecked )
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else if( nextTimeStep == maxTimeStep )
                {
                    if( pattern == Merge::KeepInitialChecked )
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                    else if( pattern == Merge::KeepFinalChecked )
                    {
                        if(currentTimeStep < nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( pattern == Merge::BothChecked )
                    {
                        if(currentTimeStep < nextTimeStep && minTimeStep != maxTimeStep )
                        {
                            qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                            importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                        }
                        //当てはまらない場合
                        else
                        {
                            qInfo() << "Does nothing.[" << __LINE__ << "]";
                        }
                    }
                    else if( pattern == Merge::NoneChecked )
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                }
            }
            else
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
            }
        }
        else if( nextTimeStep < minTimeStep )
        {
            if( pattern == Merge::KeepInitialChecked )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( currentTimeStep > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
                }
                else
                {
                    qInfo() << "Does nothing.[" << __LINE__ << "]";
                }
            }
            else if( pattern == Merge::KeepFinalChecked )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
            else if( pattern == Merge::BothChecked )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( currentTimeStep > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
                }
                else
                {
                    qInfo() << "Does nothing.[" << __LINE__ << "]";
                }
            }
            else if( pattern == Merge::NoneChecked )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }
        else if( nextTimeStep > maxTimeStep )
        {
            if( pattern == Merge::KeepInitialChecked )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
            else if( pattern == Merge::KeepFinalChecked )
            {
                if( currentTimeStep < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
                }
                else
                {
                    qInfo() << "Does nothing.[" << __LINE__ << "]";
                }
            }
            else if( pattern == Merge::BothChecked )
            {
                //現在表示されているタイムステップがファイルの最小タイムステップよりも大きく、単一ステップのデータではない場合
                if( currentTimeStep < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
                }
                else
                {
                    qInfo() << "Does nothing.[" << __LINE__ << "]";
                }
            }
            else if( pattern == Merge::NoneChecked )
            {
                qInfo() << "Delete the object.[" << __LINE__ << "]";
                removeObject( filesManager );
            }
        }
        else
        {
            qInfo() << "Delete the object.[" << __LINE__ << "]";
            removeObject( filesManager );
        }
    }


    return importedObject;
}

void Merge::updateObject( FilesManager* filesManager, kvs::ObjectBase* object )
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

void Merge::removeObject( FilesManager* filesManager )
{
    //Sceneにオブジェクトが登録されている場合
    if( filesManager->getIds().first != 0 && filesManager->getIds().second != 0 )
    {
        m_screen->scene()->IDManager()->erase(filesManager->getIds().first, filesManager->getIds().second);
        filesManager->setIds(std::pair<int, int>(0, 0));
    }
}

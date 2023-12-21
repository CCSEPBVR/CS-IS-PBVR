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

/*===========================================================================*/
/**
 *  @brief ファイルを選択してそのパスをUIに表示するメソッド
 * このメソッドは、QFileDialogを使用してユーザーにファイルの選択を促し、
 * 選択されたファイルのパスをUIのテキストエディットに表示します。
 */
/*===========================================================================*/
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

/*===========================================================================*/
/**
 * @brief ファイルを追加するメソッド
 * このメソッドは、指定されたファイルパスからファイル情報を取得し、新しい FilesManager インスタンスを作成します。
 * その後、ファイルのタイムステップ範囲を確認し、ファイルのフォーマットをチェックします。
 * 最後に、新しい FilesManager を登録し、m_files_manager メンバ変数に追加します。
 * また、全体のタイムステップ範囲を再計算します。
 */
/*===========================================================================*/
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

/*===========================================================================*/
/**
 * @brief ファイルディレクトリ内のファイル名から最小および最大の数値タイムステップを検出し、FilesManagerに設定するメソッド
 * このメソッドは、指定されたファイルディレクトリ内のファイル名を解析し、
 * ファイル名が特定のパターンに一致する場合に、その数値部分を抽出して最小および最大の
 * タイムステップ値を計算します。そして、新しい FilesManager インスタンスにこれらの値を設定します。
 * @param fileInfo 解析対象のファイル情報へのポインタ
 * @param directory 解析対象のディレクトリへのポインタ
 * @param filesManager タイムステップ情報を設定する FilesManager インスタンスへのポインタ
 */
/*===========================================================================*/
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

/*===========================================================================*/
/**
 * @brief ファイルの形式を検査し、FilesManagerに関連する情報を設定するメソッド
 * このメソッドは、指定されたファイルの形式を検査し、各フォーマットに応じて FilesManager インスタンスに
 * 関連する情報を設定します。サポートされるフォーマットは、KVSML、STL、3DS、FBX、LASです。
 * KVSML フォーマットの場合、PointObject および PolygonObject を識別し、それぞれに応じた設定を行います。
 * STL、3DS、FBX、LAS フォーマットの場合も対応する情報を設定します。
 * @param fileInfo ファイル情報へのポインタ
 * @param filesManager ファイルフォーマット関連情報を設定する FilesManager インスタンスへのポインタ
 */
/*===========================================================================*/
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

/*===========================================================================*/
/**
 * @brief FilesManager を登録し、UI上のファイルテーブルに関連する情報を追加するメソッド
 * このメソッドは、FilesManager インスタンスを受け取り、その情報を使用して UI 上のファイルテーブルに
 * 関連する行とセルを追加します。具体的には、ファイル名、可視性、KeepInitialの保持、KeepFinalの保持、ファイルフォーマットなど
 * の情報をセットします。また、テクスチャのないポリゴンの場合は、RGBカラーと透明度の情報もセットされます。
 * @param filesManager FilesManager インスタンスへのポインタ
 */
/*===========================================================================*/
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

/*===========================================================================*/
/**
 * @brief 全体の FilesManager インスタンスにおける最小および最大のタイムステップを計算し、
 *        TimeControl インスタンスを更新するメソッド
 *
 * このメソッドは、各 FilesManager インスタンスに設定された最小および最大のタイムステップを走査し、
 * 全体の最小および最大のタイムステップを計算します。また、単一オブジェクトかどうかを判断し、
 * TimeControl インスタンスを更新します。
 */
/*===========================================================================*/
void Merge::calculateMinMaxTimeStep()
{
    int overAllMinTimeStep = INT_MAX;
    int overAllMaxTimeStep = INT_MIN;
    bool isSingleObject = true;

    for (int i = 0; i < m_files_manager.size(); i++)
    {
        FilesManager* filesManager = m_files_manager[i];
        int minTimeStep = filesManager->getMinTimeStep();
        int maxTimeStep = filesManager->getMaxTimeStep();

        overAllMinTimeStep = std::min(overAllMinTimeStep, minTimeStep);
        overAllMaxTimeStep = std::max(overAllMaxTimeStep, maxTimeStep);
        if(i >= 1)
        {
            isSingleObject = false;
        }
    }
    m_time_control->updateTimeStepMinMax( overAllMinTimeStep, overAllMaxTimeStep, isSingleObject );
}

void Merge::mergeObjects()
{
    //FilesManagerに登録されているitem分ループ
    for( FilesManager* filesManager : m_files_manager )
    {
        kvs::ObjectBase* object = nullptr;
        //PointObjectの場合
        if( filesManager->getFileFormat() == FilesManager::PointObject )
        {
            object = selectPattern<kvs::PointImporter, kvs::PointObject>( filesManager );
        }
        //PolygonObjectの場合
        else if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon )
        {
            object = selectPattern<kvs::PolygonImporter, kvs::PolygonObject>( filesManager );
        }
        else if( filesManager->getFileFormat() == FilesManager::ServerPointObject )
        {
            object = selectPattern( filesManager );
        }

        //オブジェクトがnullptrではない場合
        if( object != nullptr )
        {
            updateObject( filesManager, object );
        }
    }

    currentTimeStep = m_time_control->getNextTimeStep();
    m_time_control->setCurrentTimeStep( currentTimeStep );
    m_preference->setCurrentTimeStep( currentTimeStep );
    m_preference->loadShadingSettings();
    m_preference->applyShadingSettings();
    m_screen->redraw();
}

//FOR LOCAL FILE
template <typename Importer, typename ObjectType>
ObjectType* Merge::selectPattern(FilesManager* filesManager)
{
    //Visibleにチェックボックスがついている場合
    if( filesManager->getVisible() == Qt::PartiallyChecked )
    {
        //keep initialにチェックがついている場合
        if( filesManager->getKeepInitial() == Qt::PartiallyChecked )
        {
            //Keep finalにチェックがついている場合
            if( filesManager->getKeepFinal() == Qt::PartiallyChecked )
            {
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, BothChecked );
            }
            //Keep finalにチェックがついていない場合
            else
            {
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, KeepInitialChecked );
            }
        }

        //keep initialにチェックがついていない場合
        if( filesManager->getKeepInitial() == Qt::Unchecked )
        {
            //Keep finalにチェックがついている場合
            if( filesManager->getKeepFinal() == Qt::PartiallyChecked )
            {
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, KeepFinalChecked );
            }
            //Keep finalにチェックがついていない場合
            else
            {
                return timeStepCheckAndImport<Importer, ObjectType>( filesManager, NoneChecked );
            }
        }
    }
    else
    {
        qInfo() << "Delete the object.[" << __LINE__ << "]";
        removeObject( filesManager );
    }
    return nullptr;
}

template <typename Importer, typename ObjectType>
ObjectType* Merge::timeStepCheckAndImport( FilesManager* filesManager, pattern pattern )
{
    ObjectType* importedObject = nullptr;
    const int minTimeStep  = filesManager->getMinTimeStep();
    const int maxTimeStep  = filesManager->getMaxTimeStep();
    const int nextTimeStep = m_time_control->getNextTimeStep();
    const QString filePath = filesManager->getFileInfo().filePath();
    const bool already_registerd = (filesManager->getIds().first == 0 && filesManager->getIds().second == 0) ? false : true;;

    if( already_registerd == false )
    {
        if( pattern == KeepInitialChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
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
                importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Does nothing.[" << __LINE__ << "]";
                importedObject = nullptr;
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
            }
        }

        if( pattern == BothChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
            }
            else if( nextTimeStep < minTimeStep )
            {
                qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
            }
            else if( nextTimeStep > maxTimeStep )
            {
                qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
            }
        }

        if( pattern == NoneChecked )
        {
            if( nextTimeStep >= minTimeStep && nextTimeStep <= maxTimeStep )
            {
                qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
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
                if( nextTimeStep != currentTimeStep )
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
                    else if( nextTimeStep == maxTimeStep )
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
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
                if( currentTimeStep > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
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
                if( nextTimeStep != currentTimeStep )
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
                    else if( nextTimeStep == maxTimeStep )
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
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
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
                if( currentTimeStep < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
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
                if( nextTimeStep != currentTimeStep )
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
                    else if( nextTimeStep == maxTimeStep )
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
                    else
                    {
                        qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), nextTimeStep ).toStdString() );
                    }
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
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
                if( currentTimeStep > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), minTimeStep ).toStdString() );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, minTimeStep ).toStdString() );
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
                if( currentTimeStep < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filesManager->getFileInfo().filePath(), maxTimeStep ).toStdString() );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, maxTimeStep ).toStdString() );
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
                if( nextTimeStep != currentTimeStep )
                {
                    qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                    importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = new Importer( updateTimeStepInFileName( filePath, nextTimeStep ).toStdString() );
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
    return importedObject;
}

//FOR SERVER POINT OBJECT
kvs::PointObject* Merge::selectPattern(FilesManager* filesManager)
{
    //Visibleにチェックボックスがついている場合
    if( filesManager->getVisible() == Qt::PartiallyChecked )
    {
        //keep initialにチェックがついている場合
        if( filesManager->getKeepInitial() == Qt::PartiallyChecked )
        {
            //Keep finalにチェックがついている場合
            if( filesManager->getKeepFinal() == Qt::PartiallyChecked )
            {
                return timeStepCheckAndImport( filesManager, BothChecked );
            }
            //Keep finalにチェックがついていない場合
            else
            {
                return timeStepCheckAndImport( filesManager, KeepInitialChecked );
            }
        }

        //keep initialにチェックがついていない場合
        if( filesManager->getKeepInitial() == Qt::Unchecked )
        {
            //Keep finalにチェックがついている場合
            if( filesManager->getKeepFinal() == Qt::PartiallyChecked )
            {
                return timeStepCheckAndImport( filesManager, KeepFinalChecked );
            }
            //Keep finalにチェックがついていない場合
            else
            {
                return timeStepCheckAndImport( filesManager, NoneChecked );
            }
        }
    }
    else
    {
        qInfo() << "Delete the object.[" << __LINE__ << "]";
        removeObject( filesManager );
    }
    return nullptr;
}

kvs::PointObject* Merge::timeStepCheckAndImport( FilesManager* filesManager, pattern pattern )
{
    kvs::PointObject* importedObject = nullptr;
    const int minTimeStep  = filesManager->getMinTimeStep();
    const int maxTimeStep  = filesManager->getMaxTimeStep();
    const int nextTimeStep = m_time_control->getNextTimeStep();
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
                if( nextTimeStep != currentTimeStep )
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
                        if(currentTimeStep > nextTimeStep && minTimeStep != maxTimeStep )
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
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
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
                if( currentTimeStep > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( minTimeStep );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( minTimeStep );
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
                if( nextTimeStep != currentTimeStep )
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
                        if(currentTimeStep > nextTimeStep && minTimeStep != maxTimeStep )
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
                        if(currentTimeStep < nextTimeStep && minTimeStep != maxTimeStep )
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
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
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
                if( currentTimeStep < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( maxTimeStep );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( maxTimeStep );
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
                if( nextTimeStep != currentTimeStep )
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
                        if(currentTimeStep > nextTimeStep && minTimeStep != maxTimeStep )
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
                        if(currentTimeStep < nextTimeStep && minTimeStep != maxTimeStep )
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
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
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
                if( currentTimeStep > minTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the minimum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( minTimeStep );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( minTimeStep );
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
                if( currentTimeStep < maxTimeStep && minTimeStep != maxTimeStep )
                {
                    qInfo() << "Imported the file for the maximum time step.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( maxTimeStep );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( maxTimeStep );
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
                if( nextTimeStep != currentTimeStep )
                {
                    qInfo() << "Imported the file that matches the Next Time Step value.[" << __LINE__ << "]";
                    importedObject = m_connect->connect2( nextTimeStep );
                }
                else
                {
                    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon && filesManager->getIsModified() == true )
                    {
                        qInfo() << "The color or opacity value has been modified.[" << __LINE__ << "]";
                        importedObject = m_connect->connect2( nextTimeStep );
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
    return importedObject;
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

void Merge::removeObject( FilesManager* filesManager )
{
    //Sceneにオブジェクトが登録されている場合
    if( filesManager->getIds().first != 0 && filesManager->getIds().second != 0 )
    {
        m_screen->scene()->IDManager()->erase(filesManager->getIds().first, filesManager->getIds().second);
        filesManager->setIds(std::pair<int, int>(0, 0));
    }
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

void Merge::serverObject( QString volumeDataFilePath, int min, int max )
{
    QFileInfo fileInfo( volumeDataFilePath );
    FilesManager *newFilesManager = new FilesManager;
    newFilesManager->setFileInfo( fileInfo );
    newFilesManager->setFileName( "Server" );
    newFilesManager->setMinTimeStep(min);
    newFilesManager->setMaxTimeStep(max);
    newFilesManager->setFileFormat( FilesManager::ServerPointObject );
    newFilesManager->setFileSuffix( "Server(PointObject)" );
    newFilesManager->setVisible( Qt::CheckState::Checked );
    newFilesManager->setKeepInitial( Qt::CheckState::Unchecked );
    newFilesManager->setKeepFinal( Qt::CheckState::Unchecked );
    registerFile( newFilesManager );
    m_files_manager.append(newFilesManager);
    calculateMinMaxTimeStep();
}

/*===========================================================================*/
/**
 * @brief FilesManager インスタンスの内容をコンソールに表示するメソッド
 * このメソッドは、各 FilesManager インスタンスの詳細な情報をコンソールに出力します。
 * インデックス、ファイル名、タイムステップの範囲、可視性、初期状態の保持、最終状態の保持、
 * ファイルサフィックス、ファイルフォーマットなどが表示されます。非テクスチャポリゴンの場合、
 * RGBカラーと透明度も表示されます。
 */
/*===========================================================================*/
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

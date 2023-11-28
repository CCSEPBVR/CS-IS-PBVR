#include "Merge.h"
#include "ui_Merge.h"

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QColorDialog>
#include <QCheckBox>

Merge::Merge(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Merge)
{
    ui->setupUi(this);
    connect( ui->filesTWidget, &QTableWidget::cellDoubleClicked, this, &Merge::tableItemClicked);
    connect(ui->importFilesBrowsePBtn, &QPushButton::clicked, this, &Merge::onBrowserButtonClicked);
    connect(ui->importFilesAddPBtn, &QPushButton::clicked, this, &Merge::onAddButtonClicked);
    connect( ui->applyBtn, &QPushButton::clicked, this, &Merge::onApplyButtonClicked );
}

Merge::~Merge()
{
    delete ui;
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
    FilesManager* fileManager = new FilesManager();
    QFileInfo fileInfo( ui->importFilesPathLEdit->text() );
    QDir directory( fileInfo.absolutePath() );


    checkMinMaxTimeStep( &fileInfo, &directory, fileManager );
    checkFileFormat( &fileInfo, fileManager );
    if(fileManager->getFileFormat() != FilesManager::Unknown )
    {
        registerFile( fileManager );
        m_files_manager.append( fileManager );
    }
    //HERE
    delete fileManager;
}

void Merge::registerFile( FilesManager* filesManager )
{
    item_name << filesManager->getFileName();
    ui->filesTWidget->setRowCount( ui->filesTWidget->rowCount() + 1);
    ui->filesTWidget->setVerticalHeaderLabels( item_name );

    //フォーマットに関係なく共通のウィジェット。
    //    QTableWidgetItem *displayCheckBox = new QTableWidgetItem();
    QCheckBox *displayCheckBox = new QCheckBox();
//    displayCheckBox->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ) ;
    displayCheckBox->setCheckState ( filesManager->getVisible() ) ;

    QCheckBox *keepInitialCheckBox = new QCheckBox();
//    keepInitialCheckBox->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ) ;
    keepInitialCheckBox->setCheckState ( filesManager->getKeepInitial() ) ;

    QCheckBox *keepFinalCheckBox = new QCheckBox();
//    keepFinalCheckBox->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ) ;
    keepFinalCheckBox->setCheckState ( filesManager->getKeepFinal()  ) ;

    QTableWidgetItem* format = new QTableWidgetItem;
    format->setFlags( format->flags() & ~Qt::ItemIsEditable );
    format->setText( filesManager->getFileSuffix() );

    if( filesManager->getFileFormat() == FilesManager::NonTexturedPolygon )
    {
        QTableWidgetItem *colorValue = new QTableWidgetItem;
        colorValue->setFlags( format->flags() & ~Qt::ItemIsEditable );
        colorValue->setBackground( Qt::gray );

        QDoubleSpinBox *opacityValue = new QDoubleSpinBox();
        opacityValue->setRange( 0.0, 1.0 );
        opacityValue->setSingleStep( 0.01 );
        opacityValue->setValue( 0.5 );

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

//    QTableWidgetItem *deleteCheckBox = new QTableWidgetItem();
    QCheckBox *deleteCheckBox = new QCheckBox;
//    deleteCheckBox->setFlags ( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled ) ;
    deleteCheckBox->setCheckState ( Qt::Unchecked ) ;

    //    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 0, displayCheckBox );
    //    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 1, keepInitialCheckBox );
    //    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 2, keepFinalCheckBox );
    ui->filesTWidget->setCellWidget(ui->filesTWidget->rowCount() - 1, 0, displayCheckBox);
    ui->filesTWidget->setCellWidget(ui->filesTWidget->rowCount() - 1, 1, keepInitialCheckBox);
    ui->filesTWidget->setCellWidget(ui->filesTWidget->rowCount() - 1, 2, keepFinalCheckBox);
    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 3, format );
    //    ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 6, deleteCheckBox );
    ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 6, deleteCheckBox );

}

void Merge::checkMinMaxTimeStep( QFileInfo* fileInfo, QDir* directory, FilesManager* filesManager )
{
    QRegularExpression regex(fileInfo->baseName().left( fileInfo->baseName().indexOf('_') ) + "_([0-9]+)\\.*");
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

            // 最小値と最大値を更新
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
        filesManager->setFileName( fileInfo->baseName().left( fileInfo->baseName().indexOf('_') ) );
        filesManager->setMinTimeStep( minValue );
        filesManager->setMaxTimeStep( maxValue );
    }
    else
    {
        qInfo() << "該当する数字が見つかりませんでした。";
    }
}

void Merge::checkFileFormat( QFileInfo* fileInfo, FilesManager* filesManager )
{
    QFile file( fileInfo->filePath() );
    QXmlStreamReader xml(&file);
    QString fileSuffix = fileInfo->suffix().toLower(); // 拡張子を取得して小文字に変換
    filesManager->setFileFormat( FilesManager::Unknown ); // デフォルトは Unknown
    //そのファイルが開けるかを確認
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "ファイルを開けませんでした。";
    }

    //拡張子によるフォーマットの判別を行う。
    //kvsmlフォーマットに関してはxmlを解析しPointObjectかPolygonObjectかを判別する。
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
                    filesManager->setFileFormat( FilesManager::PointObject);
                    filesManager->setFileSuffix( "KVSML(PointObject)" );
                    break; // ファイルをクローズしてループを終了
                }
                else if (tagName == "PolygonObject")
                {
                    qDebug() << "KVSML(PolygonObject)です。";
                    filesManager->setFileFormat( FilesManager::NonTexturedPolygon );
                    filesManager->setFileSuffix( "KVSML(PolygonObject)" );
                    filesManager->setRGBColor( QColor(128, 128, 128));
                    filesManager->setOpacity( 0.5 );
                    break; // ファイルをクローズしてループを終了
                }
            }
        }

        if (xml.hasError())
        {
            qDebug() << "XML パースエラー: " << xml.errorString();
        }
    }
    else if ( fileSuffix == "stl" )
    {
        qDebug() << "stlファイルです。";
        filesManager->setFileFormat( FilesManager::NonTexturedPolygon );
        filesManager->setFileSuffix( "stl" );
        filesManager->setRGBColor( QColor(128, 128, 128));
        filesManager->setOpacity( 0.5 );
    }else if( fileSuffix == "3ds")
    {
        qDebug() << "3dsファイルです。";
        filesManager->setFileFormat( FilesManager::TexturedPolygon );
        filesManager->setFileSuffix( "3ds" );
    }else if( fileSuffix == "fbx")
    {
        qDebug() << "fbxファイルです。";
        filesManager->setFileFormat( FilesManager::TexturedPolygon );
        filesManager->setFileSuffix( "fbx" );
    }else if( fileSuffix == "las")
    {
        qDebug() << "lasファイルです。";
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

    // ファイルはスコープを抜ける際に自動的にクローズされる
}

void Merge::tableItemClicked(int row, int column)
{
    //4列目でかつstl形式である場合はカラーパレットを開く。
    if(column == 4)
    {
        if( "stl" == ui->filesTWidget->item(row,column-1)->text() || "KVSML(PolygonObject)" == ui->filesTWidget->item(row,column-1)->text() )
        {
            //ポリゴン用色選択(RGBの値を入れる機能がないのでお勧めできない)
            QColor color = QColorDialog::getColor(Qt::gray);
            ui->filesTWidget->item(row,column)->setBackground( color );
        }
    }
}

void Merge::removeSelectedRows()
{
    // List to store indices of rows to be deleted
    QList<int> rowsToDelete;

    for (int row = 0; row < ui->filesTWidget->rowCount(); ++row)
    {
        // Delete CheckBox
        QCheckBox* deleteCheckBox = qobject_cast<QCheckBox*>(ui->filesTWidget->cellWidget(row, 6));

        if (deleteCheckBox && deleteCheckBox->checkState() == Qt::Checked)
        {
            // If the Delete CheckBox is checked, mark the row for deletion
            rowsToDelete << row;
        }
    }

    // Output the values before removal
    qInfo() << "Rows before removal: " << ui->filesTWidget->rowCount();
    for (int row = 0; row < ui->filesTWidget->rowCount(); ++row)
    {
        qInfo() << "Row " << row << ": " << ui->filesTWidget->verticalHeaderItem(row)->text();
    }

    // Remove the marked rows in reverse order to avoid index issues
    for (int i = rowsToDelete.size() - 1; i >= 0; --i)
    {
        int rowToRemove = rowsToDelete[i];

        // Remove the entry from m_files_manager
        if (rowToRemove >= 0 && rowToRemove < m_files_manager.size())
        {
            // delete m_files_manager[rowToRemove]; // Uncomment if memory needs to be freed
            m_files_manager.removeAt(rowToRemove);
        }

        // Remove the row from item_name
        item_name.removeAt(rowToRemove);

        ui->filesTWidget->removeRow(rowToRemove);
    }

    // Output the values after removal
    qInfo() << "Rows after removal: " << ui->filesTWidget->rowCount();
    for (int row = 0; row < ui->filesTWidget->rowCount(); ++row)
    {
        qInfo() << "Row " << row << ": " << ui->filesTWidget->verticalHeaderItem(row)->text();
    }
}

void Merge::updateFilesManagerFromTable()
{
    for (int row = 0; row < ui->filesTWidget->rowCount(); row++)
    {
//        // 各セルの値をfileManagerに設定
//        QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
//        QPoint checkBoxPos = checkBox->pos();
//        QModelIndex index = ui->filesTWidget->indexAt(checkBoxPos);
        // チェックボックスのセルを取得
        QWidget* widget = ui->filesTWidget->cellWidget(row, 0);
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget);
        if (checkBox)
        {
            m_files_manager[row]->setVisible(checkBox->checkState());
        }
//        m_files_manager[row]->setKeepInitial(static_cast<Qt::CheckState>(ui->filesTWidget->item(row, 1)->checkState()));
//        m_files_manager[row]->setKeepFinal(static_cast<Qt::CheckState>(ui->filesTWidget->item(row, 2)->checkState()));
//        m_files_manager[row]->setFileName(ui->filesTWidget->item(row, 3)->text());
//        m_files_manager[row]->setFileSuffix(ui->filesTWidget->item(row, 4)->text());
//        m_files_manager[row]->setFileFormat(static_cast<FilesManager::FileType>(ui->filesTWidget->item(row, 5)->data(Qt::UserRole).toInt()));
//        m_files_manager[row]->setRGBColor(ui->filesTWidget->item(row, 6)->data(Qt::UserRole).value<QColor>());
//        m_files_manager[row]->setOpacity(ui->filesTWidget->item(row, 10)->data(Qt::UserRole).toDouble());

    }
}

void Merge::onApplyButtonClicked()
{
    removeSelectedRows();
    updateFilesManagerFromTable();
    for(int i = 0; i < m_files_manager.size(); i++)
    {
        qInfo() << m_files_manager[i]->getMinTimeStep();
        qInfo() << m_files_manager[i]->getMaxTimeStep();
        qInfo() << m_files_manager[i]->getVisible();
        qInfo() << m_files_manager[i]->getKeepInitial();
        qInfo() << m_files_manager[i]->getKeepFinal();
    }
}



#include "MergePanel.h"
#include "ui_MergePanel.h"

#include <QColorDialog>
#include <QXmlStreamReader>
#include <QCheckBox>
#include <QDoubleSpinBox>

MergePanel::MergePanel(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::MergePanel)
{
    ui->setupUi(this);
    connect(ui->filesTWidget, &QTableWidget::cellDoubleClicked, this, &MergePanel::onFilesTWidgetCellDoubleClicked);
    connect(ui->importFilesBrowsePBtn, &QPushButton::clicked, this, &MergePanel::onBrowserButtonClicked );
    connect(ui->importFilesAddPBtn, &QPushButton::clicked, this, &MergePanel::onAddButtonClicked );
    connect(ui->applyPBtn, &QPushButton::clicked, this, &MergePanel::onApplyButtonClicked );
}

MergePanel::~MergePanel()
{
    delete ui;
}

void MergePanel::onFilesTWidgetCellDoubleClicked(int row, int column)
{
    if (column == 4) // Colorのセルをダブルクリックしているか。
    {
        QTableWidgetItem* formatItem = ui->filesTWidget->item( row, 4 );
        formatItem->setBackground(QColorDialog::getColor(Qt::gray));
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
    QFileInfo fileInfo( ui->importFilesPathLEdit->text() );
    checkMinMaxTimeStep( fileInfo );
    checkFileFormat( fileInfo );
    {
        int row = ui->filesTWidget->rowCount();
        ui->filesTWidget->setRowCount( row + 1 );

        QTableWidgetItem *headerItem = new QTableWidgetItem(QString(ui->importFilesPathLEdit->text()).arg(row));
        ui->filesTWidget->setVerticalHeaderItem(row, headerItem);

        QCheckBox *displayCheckBox = new QCheckBox();
        displayCheckBox->setCheckState( Qt::Checked );

        QCheckBox *keepInitialCheckBox = new QCheckBox();
        keepInitialCheckBox->setCheckState( Qt::Unchecked );

        QCheckBox *keepFinalCheckBox = new QCheckBox();
        keepFinalCheckBox->setCheckState( Qt::Unchecked );

        QTableWidgetItem* format = new QTableWidgetItem;
        format->setFlags( format->flags() & ~Qt::ItemIsEditable );
        format->setText( "FORMAT" );

        //ONLY NON TEXTURED POLYGON
        QTableWidgetItem *colorValue = new QTableWidgetItem;
        colorValue->setFlags( format->flags() & ~Qt::ItemIsEditable );
        colorValue->setBackground( Qt::red );

        QDoubleSpinBox *opacityValue = new QDoubleSpinBox();
        opacityValue->setRange( 0.0, 1.0 );
        opacityValue->setSingleStep( 0.01 );
        opacityValue->setValue( 0.5 );
        //ONLY NON TEXTURED POLYGON

        QCheckBox *deleteCheckBox = new QCheckBox;
        deleteCheckBox->setCheckState( Qt::Unchecked );

        ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 0, displayCheckBox );
        ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 1, keepInitialCheckBox );
        ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 2, keepFinalCheckBox );
        ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 3, format );
        ui->filesTWidget->setItem( ui->filesTWidget->rowCount() - 1, 4, colorValue);
        ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 5, opacityValue);
        ui->filesTWidget->setCellWidget( ui->filesTWidget->rowCount() - 1, 6, deleteCheckBox );
    }
}

void MergePanel::checkMinMaxTimeStep( QFileInfo &fileInfo )
{
    const QRegularExpression regex( fileInfo.baseName().left( fileInfo.baseName().indexOf('_')) + "_([0-9]+)\\.*");

    int minStep = std::numeric_limits<int>::max();
    int maxStep = std::numeric_limits<int>::min();

    foreach( fileInfo, fileInfo.dir().entryInfoList( QDir::Files ) )
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
        qInfo() << minStep << "," << maxStep;
    }
    else//CAN NOT FIND MIN MAX TIME STEP
    {
    }
}

void MergePanel::checkFileFormat( QFileInfo &fileInfo )
{
    QFile file(fileInfo.filePath());
    QXmlStreamReader xml( &file );
    QString fileSuffix = fileInfo.suffix().toLower();

    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )//CAN NOT OPEN FILE
    {
        return;
    }

    if( fileSuffix == "kvsml" )
    {
        while ( !xml.atEnd() && !xml.hasError() )
        {
            QString tagName = xml.name().toString();
            if( tagName == "PointObject" )
            {
                break;
            }
            else if( tagName == "PolygonObject" )
            {
                break;
            }
        }

        if( xml.hasError() )//XML PARSE ERROR
        {
        }
    }
    else if( fileSuffix == "stl" )
    {
        qInfo() << "stl";
    }
    else if( fileSuffix == "3ds" )
    {
        qInfo() << "3ds";
    }
    else if( fileSuffix == "fbx" )
    {
        qInfo() << "fbx";
    }
    else if( fileSuffix == "las" )
    {
        qInfo() << "las";
    }
    else //NOT SUPPORTED FORMAT
    {
    }
}

void MergePanel::onApplyButtonClicked()
{
    for ( int row = 0; row < ui->filesTWidget->rowCount(); row++ )
    {
        QWidget *widget = ui->filesTWidget->cellWidget(row, 6); // deleteCheckBox の列を指定
        QCheckBox *deleteCheckBox = qobject_cast<QCheckBox*>(widget);
        if (deleteCheckBox && deleteCheckBox->checkState() == Qt::Checked)
        {
            ui->filesTWidget->removeRow(row);
            delete deleteCheckBox;
            row--; // 行が削除されたので、ループのインデックスを調整する
        }
    }
}


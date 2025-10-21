#include "ObjectEditorWIP.h"
#include "ui_ObjectEditorWIP.h"

ObjectEditorWIP::ObjectEditorWIP(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::ObjectEditorWIP)
{
    initialize();
}

ObjectEditorWIP::~ObjectEditorWIP()
{
    delete ui;
}

void ObjectEditorWIP::initialize()
{
    ui->setupUi( this );

    groupCommonObjectWidgets =
        {
            ui->nameLabel, ui->nameLineEdit,
            ui->formatLabel, ui->formatLineEdit,
            ui->directoryLabel, ui->directoryLineEdit,
            ui->timeStepLabel, ui->timeStepMinLineEdit, ui->timeStepMaxLineEdit,
            ui->focusLabel, ui->focusCheckBox,
        };

    groupServerObjectWidgets =
        {
            ui->numberOfVectorLabel, ui->numberOfVectorLineEdit,
            ui->numberOfElementsLabel, ui->numberOfElementsLineEdit,
            ui->numberOfSubvolumeLabel, ui->numberOfSubvolumeLineEdit,
            ui->numberOfNodesLabel, ui->numberOfNodesLineEdit,
            ui->elementTypeLabel, ui->elementTypeLineEdit,
            ui->fileTypeLabel, ui->fileTypeLineEdit,
            ui->stepNumberLabel, ui->stepNumberLineEdit,
            ui->particleLimitLabel, ui->particleLimitSpinBox,
            ui->densityLabel, ui->densityDoubleSpinBox,
            ui->coordinateLabel, ui->coordinateXLineEdit, ui->coordinateYLineEdit, ui->coordinateZLineEdit,
            ui->exportLabel, ui->exportPushButton,
        };

    groupNontexturePolygonObjectWidgets =
        {
            ui->colorLabel, ui->colorClickableLabel,
            ui->opacityLabel, ui->opacityDoubleSpinBox,
        };

    // 起動時はオブジェクトは存在しないため全て非表示
    toggleCommonObjectWidgets( false );
    toggleServerObjectWidgets( false );
    toggleNontexturePolygonObjectWidgets( false );

    connect( ui->browsePushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onBrowse );
}

void ObjectEditorWIP::toggleCommonObjectWidgets( bool isObject )
{
    for( auto w : groupCommonObjectWidgets )
        w->setVisible(isObject);
    ui->objectCoordsGroupBox->setVisible(isObject);
    ui->externalCoordsGroupBox->setVisible(isObject);
}

void ObjectEditorWIP::toggleServerObjectWidgets( bool isServerObject )
{
    for( auto w : groupServerObjectWidgets )
        w->setVisible(false);
}

void ObjectEditorWIP::toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject )
{
    for( auto w : groupNontexturePolygonObjectWidgets )
        w->setVisible(false);
}

void ObjectEditorWIP::onBrowse()
{
    QString filePath;
    /*
     * FIXME
     * ローカルモード(サーバと接続せずにローカルデータのみ閲覧するモード)の場合はQFileDialogでローカルファイルを参照
     * スタンドアロンモード(クライアントとサーバを同じマシンで起動、接続するモード)の場合はQFileDialogでローカルファイルを参照
     * クラサバモード(クライアントとサーバを別マシンで起動、接続するモード)の場合はRemoteFileDialogでリモートファイルを参照
     * In-situモード(クライアントとサーバを別マシンで起動、接続するモード)の場合はRemoteFileDialogでリモートファイルを参照
     */
    filePath = QFileDialog::getOpenFileName( this, tr( "ファイルを選択" ), QString(), tr( "すべてのファイル (*.*)" ) );
}

void ObjectEditorWIP::onDelete()
{

}

void ObjectEditorWIP::onApply()
{

}

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

    m_group_common_object_widgets =
        {
            ui->nameLabel       , ui->nameLineEdit,
            ui->formatLabel     , ui->formatLineEdit,
            ui->directoryLabel  , ui->directoryLineEdit,
            ui->timeStepLabel   , ui->timeStepMinLineEdit, ui->timeStepMaxLineEdit,
            ui->focusLabel      , ui->focusCheckBox,
            ui->objectCoordsGroupBox,
            ui->externalCoordsGroupBox,
        };

    m_group_common_server_point_object_widgets =
        {
            ui->particleLimitLabel  , ui->particleLimitSpinBox,
            ui->densityLabel        , ui->densityDoubleSpinBox,
        };

    m_group_client_server_point_object_widgets =
        {
            ui->numberOfVectorLabel     , ui->numberOfVectorLineEdit,
            ui->numberOfElementsLabel   , ui->numberOfElementsLineEdit,
            ui->numberOfSubvolumeLabel  , ui->numberOfSubvolumeLineEdit,
            ui->numberOfNodesLabel      , ui->numberOfNodesLineEdit,
            ui->elementTypeLabel        , ui->elementTypeLineEdit,
            ui->fileTypeLabel           , ui->fileTypeLineEdit,
            ui->stepNumberLabel         , ui->stepNumberLineEdit,
            ui->coordinateLabel         , ui->coordinateXLineEdit, ui->coordinateYLineEdit, ui->coordinateZLineEdit,
            ui->exportLabel             , ui->exportPushButton,
        };

    m_group_nontexture_polygon_object_widgets =
        {
            ui->colorLabel  , ui->colorClickableLabel,
            ui->opacityLabel, ui->opacityDoubleSpinBox,
        };

    // 起動時はオブジェクトは存在しないため全て非表示
    toggleCommonObjectWidgets( false );
    toggleClientServerObjectWidgets( false );
    toggleCommonServerObjectWidgets( false );
    toggleNontexturePolygonObjectWidgets( false );

    connect( ui->browsePushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onBrowse );
}

void ObjectEditorWIP::toggleCommonObjectWidgets( bool isObject )
{
    for( auto w : m_group_common_object_widgets ) w->setVisible( isObject );
}

void ObjectEditorWIP::toggleClientServerObjectWidgets( bool isClientServerObject )
{
    for( auto w : m_group_client_server_point_object_widgets ) w->setVisible( isClientServerObject );
}

void ObjectEditorWIP::toggleCommonServerObjectWidgets( bool isInsituServerObject )
{
    for( auto w : m_group_common_server_point_object_widgets ) w->setVisible( isInsituServerObject );
}

void ObjectEditorWIP::toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject )
{
    for( auto w : m_group_nontexture_polygon_object_widgets ) w->setVisible( isNonTexturePolygonObject );
}

void ObjectEditorWIP::onBrowse()
{
    QString filePath;
    /*
     * FIXME
     * ローカルモード(サーバと接続せずにローカルデータのみ閲覧するモード)の場合:QFileDialogでローカルファイルを参照
     * スタンドアロンモード(クライアントとサーバを同じマシンで起動、接続するモード)の場合:QFileDialogでローカルファイルを参照
     * クラサバモード(クライアントとサーバを別マシンで起動、接続するモード)の場合:RemoteFileDialogでリモートファイルを参照
     * In-situモード(クライアントとサーバを別マシンで起動、接続するモード)の場合:RemoteFileDialogでリモートファイルを参照
     */
    filePath = QFileDialog::getOpenFileName( this, tr( "ファイルを選択" ), QString(), tr( "すべてのファイル (*.*)" ) );

    if( !filePath.isEmpty() )
    {
        StringProcessor sp( filePath.toUtf8().constData() );
        if( auto objectInfoOpt = sp.extractFromLocalFile() )
        {
            StringProcessor::ObjectInfo objectInfo = *objectInfoOpt;
        }
        else
        {
            // FIXME: MainWinodwのStatusBarで通知した方がいいかも。
        }
    }
}

void ObjectEditorWIP::onDelete()
{

}

void ObjectEditorWIP::onApply()
{

}

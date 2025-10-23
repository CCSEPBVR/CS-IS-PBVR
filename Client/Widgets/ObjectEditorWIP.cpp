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
    m_model = qobject_cast<QStandardItemModel*>( ui->treeView->model() );
    if( !m_model ) // モデルがなかったら作成する。
    {
        m_model = new QStandardItemModel( this );
        // ヘッダーを設定（列名を設定）
        m_model->setHorizontalHeaderLabels( { "Name", "Format", "Display", "Keep Initial", "Keep Final" } );
        ui->treeView->setModel( m_model );
    }

    ui->colorClickableLabel->setAutoFillBackground( true );

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
    bool debug = true;
    toggleCommonObjectWidgets( debug );
    toggleCommonServerObjectWidgets( debug );
    toggleClientServerObjectWidgets( debug );
    toggleNontexturePolygonObjectWidgets( debug );

    connect( ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ObjectEditorWIP::onItemSelection );

    // 全オブジェクト共通
    connect( ui->focusCheckBox, &QCheckBox::toggled, this, &ObjectEditorWIP::onFocusCheckBoxToggled );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    connect( ui->particleLimitSpinBox, &QSpinBox::valueChanged, this, &ObjectEditorWIP::onParticleLimitSpinBoxValueChanged );
    connect( ui->densityDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ObjectEditorWIP::onDensityDoubleSpinBoxValueChanged );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    connect( ui->coordinateXLineEdit, &QLineEdit::textChanged, this, &ObjectEditorWIP::onCoordinateLineEditTextChanged );
    connect( ui->coordinateYLineEdit, &QLineEdit::textChanged, this, &ObjectEditorWIP::onCoordinateLineEditTextChanged );
    connect( ui->coordinateZLineEdit, &QLineEdit::textChanged, this, &ObjectEditorWIP::onCoordinateLineEditTextChanged );

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    connect( ui->colorClickableLabel, &ClickableLabel::doubleClicked, this, &ObjectEditorWIP::onColorLabelDoubleClicked );
    connect( ui->opacityDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &ObjectEditorWIP::onOpacityDoubleSpinBoxValueChanged );

    connect( ui->browsePushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onBrowse );
    connect( ui->deletePushButton, &QPushButton::clicked, this, &ObjectEditorWIP::onDelete );
}

void ObjectEditorWIP::toggleCommonObjectWidgets( bool isObject )
{
    for( auto w : m_group_common_object_widgets ) w->setVisible( isObject );
}

void ObjectEditorWIP::toggleCommonServerObjectWidgets( bool isCommonServerObject )
{
    for( auto w : m_group_common_server_point_object_widgets ) w->setVisible( isCommonServerObject );
}

void ObjectEditorWIP::toggleClientServerObjectWidgets( bool isClientServerObject )
{
    for( auto w : m_group_client_server_point_object_widgets ) w->setVisible( isClientServerObject );
}

void ObjectEditorWIP::toggleNontexturePolygonObjectWidgets( bool isNonTexturePolygonObject )
{
    for( auto w : m_group_nontexture_polygon_object_widgets ) w->setVisible( isNonTexturePolygonObject );
}

void ObjectEditorWIP::addObjectToModel( const StringProcessor::ObjectInfo& objectInfo, StringProcessor& sp )
{
    StringProcessor::ObjectInfo info = objectInfo;

    // 最初に追加されるオブジェクトはフォーカス状態にする
    if( m_model->rowCount() == 0 )
    {
        info.isFocus = true;
    }

    QList<QStandardItem*> rowItems;
    auto nameItem         = new QStandardItem( QString::fromUtf8( info.name ) );
    auto formatItem       = new QStandardItem( QString::fromUtf8( sp.formatToString( info.format ) ) );
    auto displayItem      = new QStandardItem( "" );
    auto keepInitialItem  = new QStandardItem( "" );
    auto keepFinalItem    = new QStandardItem( "" );

    // 編集禁止
    nameItem        ->setEditable( false );
    formatItem      ->setEditable( false );
    displayItem     ->setEditable( false );
    keepInitialItem ->setEditable( false );
    keepFinalItem   ->setEditable( false );

    QVariant var;
    var.setValue( info );
    nameItem->setData( var, Qt::UserRole );

    rowItems << nameItem << formatItem << displayItem << keepInitialItem << keepFinalItem;
    m_model->appendRow( rowItems );

    QModelIndex index = m_model->indexFromItem( rowItems.first() );
    ui->treeView->setCurrentIndex( index );

    calculateTotalMinMaxTimeStep();
}

void ObjectEditorWIP::calculateTotalMinMaxTimeStep()
{
    if( !m_model ) return;

    int totalMin = std::numeric_limits<int>::max();
    int totalMax = std::numeric_limits<int>::min();

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
        if( !nameItem ) continue;

        QVariant var = nameItem->data( Qt::UserRole );
        if( !var.canConvert<StringProcessor::ObjectInfo>() ) continue;

        StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();

        totalMin = std::min( totalMin, info.timeStep.first );
        totalMax = std::max( totalMax, info.timeStep.second );
    }
    // FIXME:ツールバー通知用のシグナルを発火してください。
    qDebug() << totalMin << ", " << totalMax;
}

void ObjectEditorWIP::onItemSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    if (selected.indexes().isEmpty()) return;

    QModelIndex index = selected.indexes().first();
    QVariant var = index.data(Qt::UserRole);
    if (!var.canConvert<StringProcessor::ObjectInfo>()) return;

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();

    // 全オブジェクト共通
    ui->nameLineEdit                ->setText( QString::fromUtf8( info.name.c_str() ) );
    ui->directoryLineEdit           ->setText( QString::fromUtf8( info.directory.c_str() ) );
    ui->formatLineEdit              ->setText( QString::fromUtf8( StringProcessor::formatToString( info.format ) ) );
    ui->timeStepMinLineEdit         ->setText( QString::number( info.timeStep.first ) );
    ui->timeStepMaxLineEdit         ->setText( QString::number( info.timeStep.second ) );
    ui->focusCheckBox               ->setChecked( info.isFocus );

    // サーバポイントオブジェクトである。(ClientServer/Insitu共通)
    ui->particleLimitSpinBox        ->setValue( info.particleLimit );
    ui->densityDoubleSpinBox        ->setValue( info.density );

    // サーバポイントオブジェクト(ClientServer)で表示させるウィジェット群
    ui->numberOfVectorLineEdit      ->setText( QString::number( info.numberOfVector ) );
    ui->numberOfElementsLineEdit    ->setText( QString::number( info.numberOfElements ) );
    ui->numberOfSubvolumeLineEdit   ->setText( QString::number( info.numberOfSubvolume ) );
    ui->numberOfNodesLineEdit       ->setText( QString::number( info.numberOfNodes ) );
    ui->elementTypeLineEdit         ->setText( QString::number( info.elementType ) );
    ui->fileTypeLineEdit            ->setText( QString::number( info.fileType ) );
    ui->stepNumberLineEdit          ->setText( QString::number( info.stepNumber ) );
    ui->coordinateXLineEdit         ->setText( QString::fromUtf8( info.coordinateX ) );
    ui->coordinateYLineEdit         ->setText( QString::fromUtf8( info.coordinateY ) );
    ui->coordinateZLineEdit         ->setText( QString::fromUtf8( info.coordinateZ ) );

    // true:テクスチャ無しポリゴンオブジェクトである。(.stl, .kvsml)
    QPalette palette = ui->colorClickableLabel->palette();
    palette.setColor( QPalette::Window, QColor( info.rgb[0], info.rgb[1], info.rgb[2] ) );
    ui->colorClickableLabel         ->setPalette( palette );
    ui->opacityDoubleSpinBox        ->setValue( info.opacity );

    bool isObject = false;
    bool isCommonServerObject = false;
    bool isClientServerObject = false;
    bool isNonTexturePolygonObject = false;
    // フラグをマップで管理(isObject, isCommonServerObject, isClientServerObject, isNonTexturePolygonObject )
    static const std::map<StringProcessor::Format, std::tuple<bool,bool,bool,bool>> formatFlags =
        {
         { StringProcessor::Format::Unknown,                   { false, false, false, false } },
         { StringProcessor::Format::ClientServerPointObject,   { true,  true,  true,  false } },
         { StringProcessor::Format::InsituServerPointObject,   { true,  true,  false, false } },
         { StringProcessor::Format::ServerGlyphObject,         { true,  false, false, false } },
         { StringProcessor::Format::PointObjectKVSML,          { true,  false, false, false } },
         { StringProcessor::Format::PointObjectLAS,            { true,  false, false, false } },
         { StringProcessor::Format::PointObjectPTS,            { true,  false, false, false } },
         { StringProcessor::Format::PolygonObjectKVSML,        { true,  false, false, true  } },
         { StringProcessor::Format::PolygonObjectSTL,          { true,  false, false, true  } },
         { StringProcessor::Format::PolygonObject3DS,          { true,  false, false, false } },
         { StringProcessor::Format::PolygonObjectFBX,          { true,  false, false, false } },
         { StringProcessor::Format::LineObjectKVSML,           { true,  false, false, false } },
         };

    auto it = formatFlags.find( info.format );
    if( it != formatFlags.end() )
    {
        std::tie( isObject, isCommonServerObject, isClientServerObject, isNonTexturePolygonObject ) = it->second;
    }

    toggleCommonObjectWidgets( isObject );
    toggleCommonServerObjectWidgets( isCommonServerObject );
    toggleClientServerObjectWidgets( isClientServerObject );
    toggleNontexturePolygonObjectWidgets( isNonTexturePolygonObject );
}

void ObjectEditorWIP::onFocusCheckBoxToggled( bool checked )
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
    info.isFocus = checked;

    QVariant newVar;
    newVar.setValue( info );
    ui->treeView->model()->setData( index, newVar, Qt::UserRole );
}

void ObjectEditorWIP::onParticleLimitSpinBoxValueChanged( int value )
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
    info.particleLimit = value;

    QVariant newVar;
    newVar.setValue( info );
    ui->treeView->model()->setData( index, newVar, Qt::UserRole );
}

void ObjectEditorWIP::onDensityDoubleSpinBoxValueChanged( double value )
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
    info.density = value;

    QVariant newVar;
    newVar.setValue( info );
    ui->treeView->model()->setData( index, newVar, Qt::UserRole );
}

void ObjectEditorWIP::onCoordinateLineEditTextChanged()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
    info.coordinateX = ui->coordinateXLineEdit->text().toUtf8().constData();
    info.coordinateY = ui->coordinateYLineEdit->text().toUtf8().constData();
    info.coordinateZ = ui->coordinateZLineEdit->text().toUtf8().constData();

    QVariant newVar;
    newVar.setValue( info );
    ui->treeView->model()->setData( index, newVar, Qt::UserRole );
}

void ObjectEditorWIP::onColorLabelDoubleClicked()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    QColorDialog colorDialog;
    colorDialog.adjustSize();

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();

    if( colorDialog.exec() == QDialog::Accepted )
    {
        QColor color = colorDialog.selectedColor();
        QPalette palette = ui->colorClickableLabel->palette();
        palette.setColor( QPalette::Window, color );
        ui->colorClickableLabel->setPalette( palette );
        info.rgb[0] = color.red();
        info.rgb[1] = color.green();
        info.rgb[2] = color.blue();
    }

    QVariant newVar;
    newVar.setValue( info );
    ui->treeView->model()->setData( index, newVar, Qt::UserRole );
}

void ObjectEditorWIP::onOpacityDoubleSpinBoxValueChanged( double value )
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
    info.opacity = value;

    QVariant newVar;
    newVar.setValue( info );
    ui->treeView->model()->setData( index, newVar, Qt::UserRole );
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
            addObjectToModel( *objectInfoOpt, sp );
        }
        else
        {
            // FIXME: MainWinodwのStatusBarで通知した方がいいかも。
        }
    }
}

void ObjectEditorWIP::onDelete()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) return;

    QModelIndex index = selectedIndexes.first();
    QVariant var = index.data( Qt::UserRole );
    if( !var.canConvert<StringProcessor::ObjectInfo>() ) return;

    bool requireFocusOnOther = false; // 他のオブジェクトをフォーカスするかどうかのフラグ

    StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
    if( info.isFocus )
    {
        bool otherFocusedFound = false;
        for( int row = 0; row < m_model->rowCount(); row++ )
        {
            if( row == index.row() ) continue; // 自分自身はスキップ

            QStandardItem* nameItem = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
            if( !nameItem ) continue;

            QVariant otherVar = nameItem->data( Qt::UserRole );
            if( !otherVar.canConvert<StringProcessor::ObjectInfo>() ) continue;

            StringProcessor::ObjectInfo otherInfo = otherVar.value<StringProcessor::ObjectInfo>();
            if( otherInfo.isFocus )
            {
                otherFocusedFound = true;
                break; // 他にフォーカス中のアイテムがあるので終了
            }
        }
        requireFocusOnOther = !otherFocusedFound; // 他にフォーカス中のアイテムがない場合はtrue
    }

    if( requireFocusOnOther )
    {
        for( int row = 0; row < m_model->rowCount(); ++row )
        {
            QStandardItem* item = m_model->item( row, 0 ); // UserRoleにObjectInfoが格納されている列
            if( !item ) continue;

            QVariant var = item->data( Qt::UserRole );
            if( !var.canConvert<StringProcessor::ObjectInfo>() ) continue;

            StringProcessor::ObjectInfo info = var.value<StringProcessor::ObjectInfo>();
            // 自分自身（削除対象）はスキップ
            if( row == index.row() ) continue;

            // 一番若い（最初に見つかった）アイテムをフォーカス
            info.isFocus = true;
            item->setData( QVariant::fromValue(info), Qt::UserRole );
            break; // 1つだけ更新したら終了
        }
    }
    m_model->removeRow( index.row() );
}

void ObjectEditorWIP::onApply()
{

}

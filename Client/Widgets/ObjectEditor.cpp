#include "ObjectEditor.h"
#include "ui_ObjectEditor.h"

ObjectEditor::ObjectEditor( kvs::qt::jaea::Screen* screen,
                            Connect* connect,
                            QWidget *parent )
    : QDockWidget( parent )
    , ui( new Ui::ObjectEditor )
    , m_screen( screen )
    , m_connect( connect )
{
    initialize();
}

ObjectEditor::~ObjectEditor()
{
    delete ui;
}

void ObjectEditor::initialize()
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

    QPalette palette = ui->colorDisplay->palette();
    palette.setColor( QPalette::Window, QColor( 128, 128, 128 ) );
    ui->colorDisplay->setAutoFillBackground( true );
    ui->colorDisplay->setPalette( palette );

    ui->exportPushButton->setCheckable( true );

    m_label_field_map =
    {
        // 全オブジェクト共通
        { "name"            , { ui->nameLabel, ui->nameDisplay } },
        { "format"          , { ui->formatLabel, ui->formatDisplay } },
        { "directory"       , { ui->directoryLabel, ui->directoryDisplay } },
        { "timeStep"        , { ui->timeStepLabel, ui->timeStepDisplay } },
        { "focus"           , { ui->focusLabel, ui->focusCheckBox } },

        // Client Server(CS)専用
        { "vectors"         , { ui->numberOfVectorsLabel, ui->numberOfVectorsDisplay } },
        { "elements"        , { ui->numberOfElementsLabel, ui->numberOfElementsDisplay } },
        { "subvolume"       , { ui->numberOfSubVolumeLabel, ui->numberOfSubVolumeDisplay } },
        { "nodes"           , { ui->numberOfNodesLabel, ui->numberOfNodesDisplay } },
        { "elementType"     , { ui->elementTypeLabel, ui->elementTypeDisplay } },
        { "fileType"        , { ui->fileTypeLabel, ui->fileTypeDisplay } },
        { "stepNumber"      , { ui->stepNumberLabel, ui->stepNumberDisplay } },
        { "coord1"          , { ui->coordinate1Label, ui->coordinate1LineEdit } },
        { "coord2"          , { ui->coordinate2Label, ui->coordinate2LineEdit } },
        { "coord3"          , { ui->coordinate3Label, ui->coordinate3LineEdit } },
        { "export"          , { ui->exportLabel, ui->exportPushButton } },

        // Client Server(CS),In-Situ(IS)共通
        { "particleLimit"   , { ui->particleLimitLabel, ui->particleLimitSpinBox } },
        { "density"         , { ui->densityLabel, ui->densityDoubleSpinBox } },

        // テクスチャ無しポリゴン専用
        { "color"           ,  { ui->colorLabel, ui->colorDisplay } },
        { "opacity"         ,  { ui->opacityLabel, ui->opacityDoubleSpinBox } }
    };

    hideAllRow();
    connect( ui->treeView->selectionModel(),    &QItemSelectionModel::selectionChanged, this, &ObjectEditor::onItemSelectionChanged );
    connect( ui->browserPushButton,             &QPushButton::clicked,                  this, &ObjectEditor::onBrowser );
    connect( ui->deletePushButton,              &QPushButton::clicked,                  this, &ObjectEditor::onDelete );
    connect( ui->applyPushButton,               &QPushButton::clicked,                  this, &ObjectEditor::onApply );

    connect( ui->focusCheckBox,                 &QCheckBox::clicked,                    this, &ObjectEditor::onFocus );

    connect( ui->coordinate1LineEdit,           &QLineEdit::textChanged,                this, &ObjectEditor::onCoordinate1 );
    connect( ui->coordinate2LineEdit,           &QLineEdit::textChanged,                this, &ObjectEditor::onCoordinate2 );
    connect( ui->coordinate3LineEdit,           &QLineEdit::textChanged,                this, &ObjectEditor::onCoordinate3 );
    connect( ui->exportPushButton,              &QPushButton::clicked,                  this, &ObjectEditor::onExport );

    connect( ui->particleLimitSpinBox,          &QSpinBox::valueChanged,                this, &ObjectEditor::onParticleLimit );
    connect( ui->densityDoubleSpinBox,          &QDoubleSpinBox::valueChanged,          this, &ObjectEditor::onDensity );

    connect( ui->colorDisplay,                  &ClickableLabel::doubleClicked,         this, &ObjectEditor::onPolygonColor );
    connect( ui->opacityDoubleSpinBox,          &QDoubleSpinBox::valueChanged,          this, &ObjectEditor::onPolygonOpacity );
}

void ObjectEditor::doMerge( int requestTimeStep )
{
    MergeWorker* worker = new MergeWorker( requestTimeStep, m_model, m_screen, m_connect );
    QThread* thread = new QThread;

    worker->moveToThread(thread);

    connect( thread, &QThread::started, worker, &MergeWorker::process );
    connect( worker, &MergeWorker::finished, thread, &QThread::quit );
    connect( worker, &MergeWorker::finished, worker, &MergeWorker::deleteLater );
    connect( thread, &QThread::finished, thread, &QThread::deleteLater );
    connect( worker, &MergeWorker::finished, this, [this, requestTimeStep]() { this->onMergeFinished( requestTimeStep ); } );

    thread->start();
}

void ObjectEditor::serverPointObjectCS( QString volumeDataFilePath, ServerPointObjectPropertiesCS serverPointObjectPropertiesCS )
{
    QString name = QFileInfo( volumeDataFilePath ).completeBaseName();

    ObjectItem objectItem;
    QList<QStandardItem*> row = objectItem.createItems();

    if( !row.isEmpty() )
    {
        QStandardItem* nameItem                 = row.at( 0 );
        QStandardItem* formatItem               = row.at( 1 );
        QStandardItem* displayCheckBoxItem      = row.at( 2 );
        QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
        QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

        if( m_model->rowCount() == 0 ) //一番最初に追加されるアイテムはfocusにチェックをいれる。
        {
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::TemporaryFocus );
        }

        QVariant format = ObjectItem::Format::ServerPointObjectCS;

        QPair<int,int> minMaxTimeStep = QPair<int,int>( serverPointObjectPropertiesCS.minTimeStep, serverPointObjectPropertiesCS.maxTimeStep );
        kvs::Vec3f minObjectCoord = serverPointObjectPropertiesCS.minObjectCoords;
        kvs::Vec3f maxObjectCoord = serverPointObjectPropertiesCS.maxObjectCoords;


        nameItem->setData( format                               ,                                       ObjectItem::nameItemRole::Format );
        nameItem->setData( volumeDataFilePath                   ,                                       ObjectItem::nameItemRole::DirectoryPath );
        nameItem->setData( QVariant::fromValue( minMaxTimeStep ),                                       ObjectItem::nameItemRole::MinMaxTimeStep );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinObjectCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxObjectCoord );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinExternalCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxExternalCoord );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.numberOfIngredients ),    ObjectItem::nameItemRole::NumberOfVectors );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.numberOfElements ),       ObjectItem::nameItemRole::NumberOfElements );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.numberOfVolumeDivide ),   ObjectItem::nameItemRole::NumberOfSubVolume );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.elementType ),            ObjectItem::nameItemRole::ElementType );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.numberOfNodes ),          ObjectItem::nameItemRole::NumberOfNodes );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.numberOfStep ),           ObjectItem::nameItemRole::StepNumber );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesCS.fileType ),               ObjectItem::nameItemRole::FileType );

        nameItem->setText( name );
        formatItem->setData( format, ObjectItem::FormatItemRole::FormatValue );
        formatItem->setText( "ServerPointObject(CS)" );

        m_model->appendRow( row );
        calculateTotalMinMaxTimeStep();
    }
}

void ObjectEditor::serverGlyphObjectCS( QString volumeDataFilePath, ServerGlyphObjectPropertiesCS serverGlyphObjectPropertiesCS )
{
    QString name = QFileInfo( volumeDataFilePath ).completeBaseName();

    ObjectItem objectItem;
    QList<QStandardItem*> row = objectItem.createItems();

    if( !row.isEmpty() )
    {
        QStandardItem* nameItem                 = row.at( 0 );
        QStandardItem* formatItem               = row.at( 1 );
        QStandardItem* displayCheckBoxItem      = row.at( 2 );
        QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
        QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

        if( m_model->rowCount() == 0 ) //一番最初に追加されるアイテムはfocusにチェックをいれる。
        {
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::TemporaryFocus );
        }

        QVariant format = ObjectItem::Format::ServerGlyphObjectCS;

        QPair<int,int> minMaxTimeStep = QPair<int,int>( serverGlyphObjectPropertiesCS.minTimeStep, serverGlyphObjectPropertiesCS.maxTimeStep );
        kvs::Vec3f minObjectCoord = serverGlyphObjectPropertiesCS.minObjectCoords;
        kvs::Vec3f maxObjectCoord = serverGlyphObjectPropertiesCS.maxObjectCoords;


        nameItem->setData( format                               ,                                       ObjectItem::nameItemRole::Format );
        nameItem->setData( volumeDataFilePath                   ,                                       ObjectItem::nameItemRole::DirectoryPath );
        nameItem->setData( QVariant::fromValue( minMaxTimeStep ),                                       ObjectItem::nameItemRole::MinMaxTimeStep );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinObjectCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxObjectCoord );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinExternalCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxExternalCoord );

        nameItem->setText( name );
        formatItem->setData( format, ObjectItem::FormatItemRole::FormatValue );
        formatItem->setText( "ServerGlyphObject(CS)" );

        m_model->appendRow( row );
        calculateTotalMinMaxTimeStep();
    }
}

void ObjectEditor::serverPointObjectIS( QString volumeDataFilePath, ServerPointObjectPropertiesIS serverPointObjectPropertiesIS )
{
    QString name = QFileInfo( volumeDataFilePath ).completeBaseName();

    ObjectItem objectItem;
    QList<QStandardItem*> row = objectItem.createItems();

    if( !row.isEmpty() )
    {
        QStandardItem* nameItem                 = row.at( 0 );
        QStandardItem* formatItem               = row.at( 1 );
        QStandardItem* displayCheckBoxItem      = row.at( 2 );
        QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
        QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

        if( m_model->rowCount() == 0 ) //一番最初に追加されるアイテムはfocusにチェックをいれる。
        {
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::TemporaryFocus );
        }

        QVariant format = ObjectItem::Format::ServerPointObjectIS;

        QPair<int,int> minMaxTimeStep = QPair<int,int>( serverPointObjectPropertiesIS.minTimeStep, serverPointObjectPropertiesIS.maxTimeStep );
        kvs::Vec3f minObjectCoord = serverPointObjectPropertiesIS.minObjectCoords;
        kvs::Vec3f maxObjectCoord = serverPointObjectPropertiesIS.maxObjectCoords;
        // float particleLimit = serverPointObjectPropertiesIS.particleLimit;
        // float particleDensity = serverPointObjectPropertiesIS.particleDensity;


        nameItem->setData( format                               ,                                       ObjectItem::nameItemRole::Format );
        nameItem->setData( ""                                   ,                                       ObjectItem::nameItemRole::DirectoryPath );
        nameItem->setData( QVariant::fromValue( minMaxTimeStep ),                                       ObjectItem::nameItemRole::MinMaxTimeStep );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinObjectCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxObjectCoord );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinExternalCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxExternalCoord );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesIS.numberOfIngredients ),    ObjectItem::nameItemRole::NumberOfVectors );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesIS.particleLimit ),          ObjectItem::nameItemRole::TemporaryParticleLimit );
        nameItem->setData( QVariant::fromValue( serverPointObjectPropertiesIS.particleDensity ),        ObjectItem::nameItemRole::TemporaryDensity );

        nameItem->setText( name );
        formatItem->setData( format, ObjectItem::FormatItemRole::FormatValue );
        formatItem->setText( "ServerPointObject(IS)" );

        m_model->appendRow( row );
        calculateTotalMinMaxTimeStep();
    }
}

void ObjectEditor::serverGlyphObjectIS( QString volumeDataFilePath, ServerGlyphObjectPropertiesIS serverGlyphObjectPropertiesIS )
{
    QString name = QFileInfo( volumeDataFilePath ).completeBaseName();

    ObjectItem objectItem;
    QList<QStandardItem*> row = objectItem.createItems();

    if( !row.isEmpty() )
    {
        QStandardItem* nameItem                 = row.at( 0 );
        QStandardItem* formatItem               = row.at( 1 );
        QStandardItem* displayCheckBoxItem      = row.at( 2 );
        QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
        QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

        if( m_model->rowCount() == 0 ) //一番最初に追加されるアイテムはfocusにチェックをいれる。
        {
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::TemporaryFocus );
        }

        QVariant format = ObjectItem::Format::ServerGlyphObjectIS;

        QPair<int,int> minMaxTimeStep = QPair<int,int>( serverGlyphObjectPropertiesIS.minTimeStep, serverGlyphObjectPropertiesIS.maxTimeStep );
        kvs::Vec3f minObjectCoord = serverGlyphObjectPropertiesIS.minObjectCoords;
        kvs::Vec3f maxObjectCoord = serverGlyphObjectPropertiesIS.maxObjectCoords;


        nameItem->setData( format                               ,                                       ObjectItem::nameItemRole::Format );
        nameItem->setData( ""                                   ,                                       ObjectItem::nameItemRole::DirectoryPath );
        nameItem->setData( QVariant::fromValue( minMaxTimeStep ),                                       ObjectItem::nameItemRole::MinMaxTimeStep );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinObjectCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxObjectCoord );
        nameItem->setData( QVariant::fromValue( minObjectCoord ),                                       ObjectItem::nameItemRole::InitialMinExternalCoord );
        nameItem->setData( QVariant::fromValue( maxObjectCoord ),                                       ObjectItem::nameItemRole::InitialMaxExternalCoord );

        nameItem->setText( name );
        formatItem->setData( format, ObjectItem::FormatItemRole::FormatValue );
        formatItem->setText( "ServerGlyphObject(IS)" );

        m_model->appendRow( row );
        calculateTotalMinMaxTimeStep();
    }
}

void ObjectEditor::updateInSituTimeStep( int minTimeStep, int maxTimeStep )
{
    QMetaObject::invokeMethod( this, [=]() {
            for( int row = 0; row < m_model->rowCount(); row++ )
            {
                QStandardItem* nameItem   = m_model->item( row, 0 );
                QStandardItem* formatItem = m_model->item( row, 1 );
                enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
                switch( format )
                {
                case ObjectItem::Format::ServerPointObjectIS:
                case ObjectItem::Format::ServerGlyphObjectIS:
                    nameItem->setData( QVariant::fromValue( QPair<int, int>( minTimeStep, maxTimeStep ) ), ObjectItem::MinMaxTimeStep );                    
                    break;
                default:
                    break;
                }
            }
            calculateTotalMinMaxTimeStep();
            emit updateInSituObjectMinMaxTimeStep( minTimeStep, maxTimeStep );

            // 現在選択中のアイテムがServerPointObjectISかServerGlyphObjectISだったらUIを更新 // AFTER 成分数も更新して
            QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
            if( selectedIndexes.isEmpty() )
            {
                return;  // 選択されているアイテムがない場合は終了
            }

            // 最初の選択されたインデックスを取得
            QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

            if( !index.isValid() )
            {
                return;
            }

            QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
            QStandardItem* formatItem               = m_model->item( index.row(), 1 );
            enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
            if( format == ObjectItem::Format::ServerPointObjectIS || format == ObjectItem::ServerGlyphObjectIS )
            {
                QPair<int,int> minMaxTimeStep = nameItem->data( ObjectItem::nameItemRole::MinMaxTimeStep ).value<QPair<int, int>>();
                ui->timeStepDisplay->setText( "( " + QString::number( minMaxTimeStep.first ) + ", " + QString::number( minMaxTimeStep.second ) + " )" );
            }

        }, Qt::QueuedConnection ); // GUIスレッドで安全に処理
}

void ObjectEditor::requestReplaceServerPointObject()
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
        switch( format )
        {
        case ObjectItem::Format::ServerPointObjectCS:
        case ObjectItem::Format::ServerPointObjectIS:
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestReplace );
            break;
        default:
            break;
        }
    }
}

void ObjectEditor::requestReplaceServerGlyphObject()
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
        switch( format )
        {
        case ObjectItem::Format::ServerGlyphObjectCS:
        case ObjectItem::Format::ServerGlyphObjectIS:
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestReplace );
            break;
        default:
            break;
        }
    }
}

// In-Situで後から粒子が生成された場合の更新処理
// void ObjectEditor::insituObjectActive( kvs::Vec3f minCoords, kvs::Vec3f maxCoords ) // AFTER パラメータの更新しろ、UIの更新も
void ObjectEditor::insituObjectActive( kvs::Vec3f minCoords, kvs::Vec3f maxCoords, int numberOfVector, float particleLimit, float particleDensity ) // AFTER パラメータの更新しろ、UIの更新も
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
        switch( format )
        {
        case ObjectItem::Format::ServerPointObjectIS:
        case ObjectItem::Format::ServerGlyphObjectIS:
            nameItem->setData( QVariant::fromValue( minCoords ),        ObjectItem::nameItemRole::InitialMinObjectCoord );
            nameItem->setData( QVariant::fromValue( maxCoords ),        ObjectItem::nameItemRole::InitialMaxObjectCoord );
            nameItem->setData( QVariant::fromValue( minCoords ),        ObjectItem::nameItemRole::InitialMinExternalCoord );
            nameItem->setData( QVariant::fromValue( maxCoords ),        ObjectItem::nameItemRole::InitialMaxExternalCoord );
            nameItem->setData( QVariant::fromValue( numberOfVector ),   ObjectItem::nameItemRole::NumberOfVectors );
            nameItem->setData( QVariant::fromValue( particleLimit ),    ObjectItem::nameItemRole::TemporaryParticleLimit );
            nameItem->setData( QVariant::fromValue( particleDensity ),  ObjectItem::nameItemRole::TemporaryDensity );
            break;
        default:
            break;
        }
    }
    onApply();
}

void ObjectEditor::onItemSelectionChanged()
{
    // 現在選択されているインデックスを取得
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    QStandardItem* formatItem               = m_model->item( index.row(), 1 );
    QStandardItem* displayCheckBoxItem      = m_model->item( index.row(), 2 );
    QStandardItem* keepInitialCheckBoxItem  = m_model->item( index.row(), 3 );
    QStandardItem* keepFinalCheckBoxItem    = m_model->item( index.row(), 4 );

    if( !nameItem || !formatItem || !displayCheckBoxItem || !keepInitialCheckBoxItem || !keepFinalCheckBoxItem )
    {
        return;
    }

    ui->nameDisplay->setText( nameItem->text() );
    ui->formatDisplay->setText( formatItem->text() );
    ui->directoryDisplay->setText( nameItem->data( ObjectItem::nameItemRole::DirectoryPath ).toString() );
    QPair<int,int> minMaxTimeStep = nameItem->data( ObjectItem::nameItemRole::MinMaxTimeStep ).value<QPair<int, int>>();
    ui->timeStepDisplay->setText( "( " + QString::number( minMaxTimeStep.first ) + ", " + QString::number( minMaxTimeStep.second ) + " )" );
    ui->focusCheckBox->setChecked( nameItem->data( ObjectItem::nameItemRole::TemporaryFocus ).value<bool>() );

    ui->numberOfVectorsDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::NumberOfVectors ).value<int>() ) );
    ui->numberOfElementsDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::NumberOfElements ).value<int>() ) );
    ui->numberOfSubVolumeDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::NumberOfSubVolume ).value<int>() ) );
    ui->numberOfNodesDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::NumberOfNodes ).value<int>() ) );
    ui->elementTypeDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::ElementType ).value<int>() ) );
    ui->fileTypeDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::FileType ).value<int>() ) );
    ui->stepNumberDisplay->setText( QString::number( nameItem->data( ObjectItem::nameItemRole::StepNumber ).value<int>() ) );

    ui->particleLimitSpinBox->setValue( nameItem->data( ObjectItem::nameItemRole::TemporaryParticleLimit ).value<int>() );
    ui->densityDoubleSpinBox->setValue( nameItem->data( ObjectItem::nameItemRole::TemporaryDensity ).value<float>() );
    ui->coordinate1LineEdit->setText( nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate1 ).value<QString>() );
    ui->coordinate2LineEdit->setText( nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate2 ).value<QString>() );
    ui->coordinate3LineEdit->setText( nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate3 ).value<QString>() );
    ui->exportPushButton->setChecked( nameItem->data( ObjectItem::nameItemRole::RequestExport ).value<bool>() );

    QPalette palette = ui->colorDisplay->palette();
    palette.setColor( QPalette::Window, nameItem->data( ObjectItem::nameItemRole::TemporaryPolygonColor ).value<QColor>() );
    ui->colorDisplay->setPalette( palette );
    ui->opacityDoubleSpinBox->setValue( nameItem->data( ObjectItem::nameItemRole::TemporaryPolygonOpacity ).value<float>() );

    kvs::Vec3 initialMinObjectCoords = nameItem->data( ObjectItem::nameItemRole::InitialMinObjectCoord ).value<kvs::Vec3>();
    kvs::Vec3 initialMaxObjectCoords = nameItem->data( ObjectItem::nameItemRole::InitialMaxObjectCoord ).value<kvs::Vec3>();
    kvs::Vec3 initialMinExternalCoords = nameItem->data( ObjectItem::nameItemRole::InitialMinExternalCoord ).value<kvs::Vec3>();
    kvs::Vec3 initialMaxExternalCoords = nameItem->data( ObjectItem::nameItemRole::InitialMaxExternalCoord ).value<kvs::Vec3>();

    ui->objectCoordsXMinDisplay->setText( QString::number( initialMinObjectCoords.x(), 'f', 5 ) );
    ui->objectCoordsYMinDisplay->setText( QString::number( initialMinObjectCoords.y(), 'f', 5 ) );
    ui->objectCoordsZMinDisplay->setText( QString::number( initialMinObjectCoords.z(), 'f', 5 ) );

    ui->objectCoordsXMaxDisplay->setText( QString::number( initialMaxObjectCoords.x(), 'f', 5 ) );
    ui->objectCoordsYMaxDisplay->setText( QString::number( initialMaxObjectCoords.y(), 'f', 5 ) );
    ui->objectCoordsZMaxDisplay->setText( QString::number( initialMaxObjectCoords.z(), 'f', 5 ) );

    ui->externalCoordsXMinDisplay->setText( QString::number( initialMinExternalCoords.x(), 'f', 5 ) );
    ui->externalCoordsYMinDisplay->setText( QString::number( initialMinExternalCoords.y(), 'f', 5 ) );
    ui->externalCoordsZMinDisplay->setText( QString::number( initialMinExternalCoords.z(), 'f', 5 ) );

    ui->externalCoordsXMaxDisplay->setText( QString::number( initialMaxExternalCoords.x(), 'f', 5 ) );
    ui->externalCoordsYMaxDisplay->setText( QString::number( initialMaxExternalCoords.y(), 'f', 5 ) );
    ui->externalCoordsZMaxDisplay->setText( QString::number( initialMaxExternalCoords.z(), 'f', 5 ) );

    hideAllRow();
    switch ( formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>() )
    {
    case ObjectItem::Format::ServerPointObjectCS:
        clientServerPointObjectProperty();
        break;
    case ObjectItem::Format::ServerPointObjectIS:
        inSituPointObjectProperty();
        break;
    case ObjectItem::Format::ServerGlyphObjectCS:
    case ObjectItem::Format::ServerGlyphObjectIS:
        commonProperty();
        break;
    case ObjectItem::Format::PointObjectKVSML:
        commonProperty();
        break;
    case ObjectItem::Format::PointObjectLAS:
    case ObjectItem::Format::PointObjectPTS:
        commonProperty();
        break;
    case ObjectItem::Format::PolygonObjectSTL:
        stlPolygonObjectProperty();
        break;
    case ObjectItem::Format::PolygonObjectKVSML:
        commonProperty();
        break;
    case ObjectItem::Format::PolygonObject3DS:
    case ObjectItem::Format::PolygonObjectFBX:
        commonProperty();
        break;
    case ObjectItem::Format::LineObjectKVSML:
        commonProperty();
        break;
    default:
        break;
    }
}

void ObjectEditor::onBrowser()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,// 親ウィジェット
        tr( "ファイルを選択" ),// ダイアログタイトル
        QString(),// 初期ディレクトリ（空ならカレント）
        tr( "すべてのファイル (*.*)" )// フィルター
        );

    if( !fileName.isEmpty() )
    {
        QFileInfo fileInfo( fileName );

        ObjectItem objectItem;
        QList<QStandardItem*> row = objectItem.createItems( fileInfo );

        if( !row.isEmpty() )
        {
            QStandardItem* nameItem                 = row.at( 0 );
            QStandardItem* formatItem               = row.at( 1 );
            QStandardItem* displayCheckBoxItem      = row.at( 2 );
            QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
            QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

            if( m_model->rowCount() == 0 ) //一番最初に追加されるアイテムはfocusにチェックをいれる。
            {
                nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::TemporaryFocus );
            }

            std::string fileName = Utils::toNativePath( createNumberedFileName( nameItem, nameItem->data( ObjectItem::nameItemRole::MinMaxTimeStep ).value<QPair<int,int>>().first ) );
            std::unique_ptr<kvs::ObjectBase> object = importObject( formatItem, fileName );

            if( object )
            {
                nameItem->setData( QVariant::fromValue( object->minObjectCoord() )  , ObjectItem::nameItemRole::InitialMinObjectCoord );
                nameItem->setData( QVariant::fromValue( object->maxObjectCoord() )  , ObjectItem::nameItemRole::InitialMaxObjectCoord );
                nameItem->setData( QVariant::fromValue( object->minExternalCoord() ), ObjectItem::nameItemRole::InitialMinExternalCoord );
                nameItem->setData( QVariant::fromValue( object->maxExternalCoord() ), ObjectItem::nameItemRole::InitialMaxExternalCoord );
            }
            else
            {
                return;
            }

            m_model->appendRow( row );
            calculateTotalMinMaxTimeStep();
        }
    }
}

void ObjectEditor::onDelete()
{
    // 現在選択されているインデックスを取得
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if( selectedIndexes.isEmpty() ) // アイテムが選択されていない場合は何もしない
    {
        qWarning() << "No Item selected.";
        return;
    }

    QModelIndex index = selectedIndexes.first();
    if( !index.isValid() ) // アイテムが存在しない場合は何もしない。(不正である場合)
    {
        return;
    }

    int deleteRow = index.row(); // 削除対象の行数を保持する。
    bool isDeleteRowFocused = m_model->item( deleteRow, 0 )->data( ObjectItem::nameItemRole::TemporaryFocus ).toBool(); // 削除対象のフォーカス状態を保持する。
    bool otherFocused = false; // 削除対象以外にフォーカス状態が存在するかを記録する。

    for( int row = 0; row < m_model->rowCount(); row++ ) // 全てのアイテム分ループし、アイテムの状態を確認する。
    {
        if( row == deleteRow ) // 削除対象の行である場合は、スキップ
        {
            continue;
        }

        QStandardItem* nameItem = m_model->item( row, 0 );
        if( !nameItem  ) // アイテムが存在しない場合は何もしない。(不正である場合)
        {
            continue;
        }

        if( nameItem->data( ObjectItem::nameItemRole::TemporaryFocus ).toBool() ) // 他のアイテムがフォーカス状態である場合
        {
            otherFocused = true;
            break;
        }
    }

    if( isDeleteRowFocused && !otherFocused ) // 削除予定のアイテムのみフォーカス状態である場合
    {
        for( int row = 0; row < m_model->rowCount(); row++ ) // 全てのアイテム分ループし、アイテムの状態を確認する。
        {
            if( row == deleteRow ) // 削除対象の行である場合は、スキップ
            {
                continue;
            }

            QStandardItem* nameItem = m_model->item( row, 0 );
            if( !nameItem  ) // アイテムが存在しない場合は何もしない。(不正である場合)
            {
                continue;
            }
            nameItem->setData( true, ObjectItem::nameItemRole::TemporaryFocus );
            break;
        }
    }

    m_screen->scene()->removeObject( m_model->item( deleteRow, 0 )->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>().first );
    m_model->removeRow( deleteRow );

    // サーバーオブジェクトの有無
    bool hasServerObject = false;
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* formatItem = m_model->item( row, 1 );
        if( !formatItem ) continue;

        int formatValue = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).toInt();
        if( formatValue == ObjectItem::Format::ServerPointObjectCS ||
            formatValue == ObjectItem::Format::ServerPointObjectIS ||
            formatValue == ObjectItem::Format::ServerGlyphObjectCS ||
            formatValue == ObjectItem::Format::ServerGlyphObjectIS )
        {
            hasServerObject = true;
            break;
        }
    }

    if( !hasServerObject )
    {
        emit noServerObjects();
    }

    calculateTotalMinMaxTimeStep();
    if( m_model->rowCount() == 0 ) // 繧｢繧､繝・Β縺御ｽ輔ｂ縺ｪ縺・ｴ蜷
    {
        hideAllRow();
        ui->objectCoordsXMinDisplay->setText( "" );
        ui->objectCoordsYMinDisplay->setText( "" );
        ui->objectCoordsZMinDisplay->setText( "" );
        ui->objectCoordsXMaxDisplay->setText( "" );
        ui->objectCoordsYMaxDisplay->setText( "" );
        ui->objectCoordsZMaxDisplay->setText( "" );
        ui->externalCoordsXMinDisplay->setText( "" );
        ui->externalCoordsYMinDisplay->setText( "" );
        ui->externalCoordsZMinDisplay->setText( "" );
        ui->externalCoordsXMaxDisplay->setText( "" );
        ui->externalCoordsYMaxDisplay->setText( "" );
        ui->externalCoordsZMaxDisplay->setText( "" );
        emit noItems(); // AFTER
    }

    m_screen->update();
}

void ObjectEditor::onApply()
{
    m_screen->scene()->objectManager()->push_centering_xform();

    // フォーカス対象の最小最大オブジェクト座標の計算結果格納用変数
    kvs::Vec3 resultMinObjectCoords(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
        );

    kvs::Vec3 resultMaxObjectCoords(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
        );

    // 値の確定処理
    // 登録されているアイテム分ループする。
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        QStandardItem* displayCheckBoxItem      = m_model->item( row, 2 );
        QStandardItem* keepInitialCheckBoxItem  = m_model->item( row, 3 );
        QStandardItem* keepFinalCheckBoxItem    = m_model->item( row, 4 );

        if( !nameItem || !formatItem || !displayCheckBoxItem || !keepInitialCheckBoxItem || !keepFinalCheckBoxItem )
        {
            // アイテムが一部でも欠けている場合はスキップ
            continue;
        }

        nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryFocus ) ), ObjectItem::nameItemRole::CurrentFocus ); // フォーカスの確定

        QString temporaryCoordinate1    = nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate1 ).value<QString>();
        QString currentCoordinate1      = nameItem->data( ObjectItem::nameItemRole::CurrentCoordinate1 ).value<QString>();
        QString temporaryCoordinate2    = nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate2 ).value<QString>();
        QString currentCoordinate2      = nameItem->data( ObjectItem::nameItemRole::CurrentCoordinate2 ).value<QString>();
        QString temporaryCoordinate3    = nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate3 ).value<QString>();
        QString currentCoordinate3      = nameItem->data( ObjectItem::nameItemRole::CurrentCoordinate3 ).value<QString>();

        int temporaryParticleLimit      = nameItem->data( ObjectItem::nameItemRole::TemporaryParticleLimit ).value<int>();
        int currentParticleLimit        = nameItem->data( ObjectItem::nameItemRole::CurrentParticleLimit ).value<int>();
        float temporaryDensity          = nameItem->data( ObjectItem::nameItemRole::TemporaryDensity ).value<float>();
        float currentDensity            = nameItem->data( ObjectItem::nameItemRole::CurrentDensity ).value<float>();

        QColor temporaryPolygonColor    = nameItem->data( ObjectItem::nameItemRole::TemporaryPolygonColor ).value<QColor>();
        QColor currentPolygonColor      = nameItem->data( ObjectItem::nameItemRole::CurrentPolygonColor ).value<QColor>();
        float temporaryPolygonOpacity   = nameItem->data( ObjectItem::nameItemRole::TemporaryPolygonOpacity ).value<float>();
        float currentPolygonOpacity     = nameItem->data( ObjectItem::nameItemRole::CurrentPolygonOpacity ).value<float>();

        bool isExport                   = nameItem->data( ObjectItem::nameItemRole::RequestExport ).value<bool>();

        displayCheckBoxItem->setData( QVariant( displayCheckBoxItem->checkState() ), ObjectItem::DisplayCheckBoxItemRole::Isdisplay );                  // デisplayの確定
        keepInitialCheckBoxItem->setData( QVariant( keepInitialCheckBoxItem->checkState() ), ObjectItem::KeepInitialCheckBoxItemRole::IskeepInitial );  // keepInitialの確定
        keepFinalCheckBoxItem->setData( QVariant( keepFinalCheckBoxItem->checkState() ), ObjectItem::KeepFinalCheckBoxItemRole::IskeepFinal );          // keepFinalの確定

        if( temporaryPolygonColor != currentPolygonColor || temporaryPolygonOpacity != currentPolygonOpacity )
        {
            // 未確定と確定の値に違いがある場合 リプレイス(インポート)を行い、値を確定する。
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestReplace );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryPolygonColor ) ), ObjectItem::nameItemRole::CurrentPolygonColor );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryPolygonOpacity ) ), ObjectItem::nameItemRole::CurrentPolygonOpacity );
        }

        if( temporaryParticleLimit != currentParticleLimit || temporaryDensity != currentDensity )
        {
            // 未確定と確定の値に違いがある場合 リプレイス(インポート)を行い、値を確定する。
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestReplace );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryParticleLimit ) ), ObjectItem::nameItemRole::CurrentParticleLimit );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryDensity ) ), ObjectItem::nameItemRole::CurrentDensity );
            emit updateRenderParameterClientMessage( m_model ); // AFTER
        }

        if( temporaryCoordinate1 != currentCoordinate1 || temporaryCoordinate2 != currentCoordinate2 || temporaryCoordinate3 != currentCoordinate3 )
        {
            // 未確定と確定の値に違いがある場合 リプレイス(インポート)を行い、値を確定する。
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestReplace );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate1 ) ), ObjectItem::nameItemRole::CurrentCoordinate1 );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate2 ) ), ObjectItem::nameItemRole::CurrentCoordinate2 );
            nameItem->setData( QVariant( nameItem->data( ObjectItem::nameItemRole::TemporaryCoordinate3 ) ), ObjectItem::nameItemRole::CurrentCoordinate3 );
            emit updateCoordinateParameterClientMessage( m_model ); // AFTER
        }

        if( isExport )
        {
            // エクスポートの要求がある場合、リプレイス(インポート)を行う。
            nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestReplace );
        }

        if( nameItem->data( ObjectItem::nameItemRole::CurrentFocus ).toBool() )
        {
            // フォーカス対象である場合、最小最大オブジェクト座標を取得しする。
            kvs::Vec3 minCoord = nameItem->data( ObjectItem::nameItemRole::InitialMinObjectCoord ).value<kvs::Vec3>();
            kvs::Vec3 maxCoord = nameItem->data( ObjectItem::nameItemRole::InitialMaxObjectCoord ).value<kvs::Vec3>();

            // 最小最大オブジェクト座標の計算結果格納用変数を更新する。
            resultMinObjectCoords.x() = std::min( resultMinObjectCoords.x(), minCoord.x() );
            resultMinObjectCoords.y() = std::min( resultMinObjectCoords.y(), minCoord.y() );
            resultMinObjectCoords.z() = std::min( resultMinObjectCoords.z(), minCoord.z() );

            resultMaxObjectCoords.x() = std::max( resultMaxObjectCoords.x(), maxCoord.x() );
            resultMaxObjectCoords.y() = std::max( resultMaxObjectCoords.y(), maxCoord.y() );
            resultMaxObjectCoords.z() = std::max( resultMaxObjectCoords.z(), maxCoord.z() );
        }
    }

    // 最小最大オブジェクト座標の計算結果格納用変数を使用して各オブジェクトのオブジェクト座標を更新する。
    // 登録されているアイテム分ループする。
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        QStandardItem* displayCheckBoxItem      = m_model->item( row, 2 );
        QStandardItem* keepInitialCheckBoxItem  = m_model->item( row, 3 );
        QStandardItem* keepFinalCheckBoxItem    = m_model->item( row, 4 );

        if( !nameItem || !formatItem || !displayCheckBoxItem || !keepInitialCheckBoxItem || !keepFinalCheckBoxItem )
        {
            // アイテムが一部でも欠けている場合はスキップ
            continue;
        }

        // オブジェクトのIDと最小最大オブジェクト座標、最小最大外部座標を取得する。
        QPair<int,int> ids = nameItem->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>();
        kvs::Vec3 initialMinObjectCoord = nameItem->data( ObjectItem::nameItemRole::InitialMinObjectCoord ).value<kvs::Vec3>();
        kvs::Vec3 initialMaxObjectCoord = nameItem->data( ObjectItem::nameItemRole::InitialMaxObjectCoord ).value<kvs::Vec3>();
        kvs::Vec3 initialMinExternalCoord = nameItem->data( ObjectItem::nameItemRole::InitialMinExternalCoord ).value<kvs::Vec3>();
        kvs::Vec3 initialMaxExternalCoord = nameItem->data( ObjectItem::nameItemRole::InitialMaxExternalCoord ).value<kvs::Vec3>();
        if ( ids.first != -1 || ids.second != -1 ) // 冗長な処理かもしれません。
        {
            if( nameItem->data( ObjectItem::nameItemRole::CurrentFocus ).toBool() )
            {
                Q_UNUSED( initialMinExternalCoord );
                Q_UNUSED( initialMaxExternalCoord );
                m_screen->scene()->object( ids.first )->setMinMaxObjectCoords( initialMinObjectCoord, initialMaxObjectCoord );
                m_screen->scene()->object( ids.first )->setMinMaxExternalCoords( initialMinObjectCoord, initialMaxObjectCoord );
            }
            else
            {
                m_screen->scene()->object( ids.first )->setMinMaxObjectCoords( resultMinObjectCoords, resultMaxObjectCoords );
                m_screen->scene()->object( ids.first )->setMinMaxExternalCoords( resultMinObjectCoords, resultMaxObjectCoords );
            }
        }
        else
        {
            if( nameItem->data( ObjectItem::nameItemRole::CurrentFocus ).toBool() )
            {
                Q_UNUSED( initialMinExternalCoord );
                Q_UNUSED( initialMaxExternalCoord );
                nameItem->setData( QVariant::fromValue( initialMinObjectCoord ), ObjectItem::nameItemRole::CurrentMinObjectCoord );
                nameItem->setData( QVariant::fromValue( initialMaxObjectCoord ), ObjectItem::nameItemRole::CurrentMaxObjectCoord );
            }
            else
            {
                nameItem->setData( QVariant::fromValue( resultMinObjectCoords ), ObjectItem::nameItemRole::CurrentMinObjectCoord );
                nameItem->setData( QVariant::fromValue( resultMaxObjectCoords ), ObjectItem::nameItemRole::CurrentMaxObjectCoord );
            }
        }
    }
    emit updateFocus( resultMinObjectCoords, resultMaxObjectCoords ); // AFTER

    m_screen->scene()->objectManager()->updateMinMaxCoords();
    m_screen->scene()->objectManager()->updateExternalCoords();

    m_screen->scene()->objectManager()->pop_centering_xform();
    emit updatePointsTranslation(); // AFTER
    m_screen->update();
}

void ObjectEditor::onFocus()
{
    // 現在選択されているインデックスを取得
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->focusCheckBox->isChecked() ), ObjectItem::nameItemRole::TemporaryFocus );
}

void ObjectEditor::onCoordinate1()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->coordinate1LineEdit->text() ), ObjectItem::nameItemRole::TemporaryCoordinate1 );
}

void ObjectEditor::onCoordinate2()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->coordinate2LineEdit->text() ), ObjectItem::nameItemRole::TemporaryCoordinate2 );
}

void ObjectEditor::onCoordinate3()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->coordinate3LineEdit->text() ), ObjectItem::nameItemRole::TemporaryCoordinate3 );
}

void ObjectEditor::onExport()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    QString exportFilePath = QFileDialog::getSaveFileName( this, tr( "Save Server-Side Point Object"), QDir::homePath(), tr("すべてのファイル ( * )" ) );
    if ( !exportFilePath.isEmpty() )
    {
        ui->exportPushButton->setDisabled( true );
        nameItem->setData( QVariant::fromValue( true ), ObjectItem::nameItemRole::RequestExport );
        nameItem->setData( QVariant::fromValue( exportFilePath ), ObjectItem::nameItemRole::ExportFilePath );
    }
    else
    {
        nameItem->setData( QVariant::fromValue( false ), ObjectItem::nameItemRole::RequestExport );
    }
}

void ObjectEditor::onParticleLimit()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->particleLimitSpinBox->value() ), ObjectItem::nameItemRole::TemporaryParticleLimit );
}

void ObjectEditor::onDensity()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->densityDoubleSpinBox->value() ), ObjectItem::nameItemRole::TemporaryDensity );
}

void ObjectEditor::onPolygonColor()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    QColorDialog colorDialog;
    colorDialog.adjustSize();

    if( colorDialog.exec() == QDialog::Accepted ) // カラーダイアログがOKされた場合
    {
        QColor color = colorDialog.selectedColor();

        QPalette palette = ui->colorDisplay->palette();
        palette.setColor( QPalette::Window, color );

        ui->colorDisplay->setPalette( palette );

        nameItem->setData( QVariant::fromValue( color ), ObjectItem::nameItemRole::TemporaryPolygonColor );
    }

    raise();
}

void ObjectEditor::onPolygonOpacity()
{
    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();

    if( selectedIndexes.isEmpty() )
    {
        qWarning() << "No item selected.";
        return;  // 選択されているアイテムがない場合は終了
    }

    // 最初の選択されたインデックスを取得
    QModelIndex index = selectedIndexes.first(); // 1つ目の選択アイテムのインデックス

    if( !index.isValid() )
    {
        return;
    }

    QStandardItem* nameItem                 = m_model->item( index.row(), 0 );
    if( !nameItem )
    {
        return;
    }

    nameItem->setData( QVariant::fromValue( ui->opacityDoubleSpinBox->value() ), ObjectItem::nameItemRole::TemporaryPolygonOpacity );
}

void ObjectEditor::onMergeFinished( int requestTimeStep )
{
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        QStandardItem* displayCheckBoxItem      = m_model->item( row, 2 );
        QStandardItem* keepInitialCheckBoxItem  = m_model->item( row, 3 );
        QStandardItem* keepFinalCheckBoxItem    = m_model->item( row, 4 );

        bool isExport = nameItem->data( ObjectItem::nameItemRole::RequestExport ).value<bool>();
        QPair<int,int> ids = nameItem->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>();
        if( !nameItem->data( ObjectItem::nameItemRole::Object ).isNull() )
        {
            if( ids.first == -1 && ids.second == -1 )
            {
                registerObject( nameItem, formatItem );
            }
            else
            {
                replaceObject( nameItem, formatItem );
            }

            if( isExport )
            {
                QPair<int,int> minMaxTimeStep = nameItem->data( ObjectItem::nameItemRole::MinMaxTimeStep ).value<QPair<int,int>>();
                auto* pointObject = nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PointObject*>();
                QString filePath = nameItem->data( ObjectItem::nameItemRole::ExportFilePath ).value<QString>() + "_" + QString( "%1" ).arg( requestTimeStep, 5, 10, QChar( '0' ) ) + ".kvsml";
                if( pointObject )
                {
                    kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>( pointObject );
                    if( kvsml ) // kvsml が正しく作成された場合のみ処理を実行
                    {
                        kvsml->setWritingDataTypeToExternalBinary();
                        kvsml->write( Utils::toNativePath( filePath ) );
                        delete kvsml;
                    }
                    else
                    {
                        nameItem->setData( QVariant::fromValue( false ), ObjectItem::nameItemRole::RequestExport );
                    }
                }
                bool allTimeStepExported = true;
                for( int i = minMaxTimeStep.first; i <= minMaxTimeStep.second; i++ )
                {
                    QFileInfo fileInfo( nameItem->data( ObjectItem::nameItemRole::ExportFilePath ).value<QString>() + "_" + QString( "%1" ).arg( i, 5, 10, QChar('0') ) + ".kvsml" );
                    if( !fileInfo.exists() )
                    {
                        allTimeStepExported = false;
                    }
                }
                if( allTimeStepExported )
                {
                    nameItem->setData( QVariant::fromValue( false ), ObjectItem::nameItemRole::RequestExport );
                    ui->exportPushButton->setEnabled( true );
                }
            }
        }
    }

    emit mergingFinish( requestTimeStep );

    int totalParticles = 0;
    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem                 = m_model->item( row, 0 );
        QStandardItem* formatItem               = m_model->item( row, 1 );
        QPair<int,int> ids = nameItem->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>();
        enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
        switch( format )
        {
        case ObjectItem::Format::ServerPointObjectCS:
        case ObjectItem::Format::ServerPointObjectIS:
        case ObjectItem::Format::PointObjectKVSML:
        case ObjectItem::Format::PointObjectLAS:
        case ObjectItem::Format::PointObjectPTS:
            if( ids.first != -1 && ids.second != -1 )
            {
                if( m_screen->scene()->object( ids.first ) != nullptr )
                {
                    auto* object = m_screen->scene()->object( ids.first );
                    if( object && object->isVisible() )
                    {
                        if( auto* pointObject = dynamic_cast<kvs::PointObject*>( object ) )
                        {
                            totalParticles += pointObject->numberOfVertices();
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    emit updateTotalParticles( totalParticles );
    if( m_model->rowCount() == 0 ) // 繧｢繧､繝・Β縺御ｽ輔ｂ縺ｪ縺・ｴ蜷
    {
        emit noItems();
    }

    m_screen->update();
}

QString ObjectEditor::createNumberedFileName( const QStandardItem* nameItem, const int requestTimeStep )
{
    QString objectName = nameItem->text();
    QString extension = nameItem->data( ObjectItem::nameItemRole::Extension ).toString();
    QString directoryPath = nameItem->data( ObjectItem::nameItemRole::DirectoryPath ).toString();
    QString extractedNumber = QString::number( requestTimeStep ).rightJustified( 5, '0' );

    QString numberFileName = ( directoryPath + "/" + objectName + "_" + extractedNumber + extension );
    return numberFileName;
}

// フォーマットから対応したインポーターを使用しオブジェクトを取得するメソッド
// ローカルオブジェクトのみ、実際の表示に使用されるimportObjectとは違う。
std::unique_ptr<kvs::ObjectBase> ObjectEditor::importObject( const QStandardItem* formatItem, const std::string& fileName )
{
    enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();

    std::unique_ptr<kvs::ObjectBase> object;

    switch( format )
    {
    case ObjectItem::Format::ServerPointObjectCS:
    case ObjectItem::Format::ServerPointObjectIS:
        return nullptr;

    case ObjectItem::Format::PointObjectKVSML:
    case ObjectItem::Format::PointObjectLAS:
    case ObjectItem::Format::PointObjectPTS:
        object = std::make_unique<kvs::PointImporter>( fileName );
        break;

    case ObjectItem::Format::PolygonObjectKVSML:
    case ObjectItem::Format::PolygonObjectSTL:
        object = std::make_unique<kvs::PolygonImporter>( fileName );
        break;

#if defined(ASSIMP)
#ifdef ASSIMP
    case ObjectItem::Format::PolygonObjectFBX:
    case ObjectItem::Format::PolygonObject3DS:
#endif
        object = std::make_unique<kvs::TexturedPolygonImporter>( fileName );
        break;
#endif

    case ObjectItem::Format::LineObjectKVSML:
        object = std::make_unique<kvs::LineImporter>( fileName );
        break;
    default:
        return nullptr; // 何もしない場合
    }
    return object;
}

// 登録されているアイテムリストの総合最小最大タイムステップを計算する。
void ObjectEditor::calculateTotalMinMaxTimeStep()
{
    // 最小・最大タイムステップの初期値設定
    int totalMinTimeStep = std::numeric_limits<int>::max();
    int totalMaxTimeStep = std::numeric_limits<int>::min();

    for( int row = 0; row < m_model->rowCount(); row++ )
    {
        QStandardItem* nameItem = m_model->item( row, 0 );
        QPair<int,int> minMaxTimeStep = nameItem->data( ObjectItem::nameItemRole::MinMaxTimeStep ).value<QPair<int,int>>();

        if( minMaxTimeStep.first != -1 && minMaxTimeStep.second != -1 )
        {
            // 最小値の更新
            totalMinTimeStep = std::min( totalMinTimeStep, minMaxTimeStep.first );

            // 最大値の更新
            totalMaxTimeStep = std::max( totalMaxTimeStep, minMaxTimeStep.second );
        }
    }

    // 計算結果の表示
    if( totalMinTimeStep != std::numeric_limits<int>::max() && totalMaxTimeStep != std::numeric_limits<int>::min() )
    {
        qInfo() << "合計タイムステップ( Min, Max ): " << totalMinTimeStep << ", " << totalMaxTimeStep;
    }
    else
    {
        qWarning() << "合計タイムステップの計算に失敗しました。" << __LINE__;
    }

    bool isSingleObject = false;
    if( m_model->rowCount() == 1 )
    {
        isSingleObject = true;
    }

    if( m_model->rowCount() == 0 )
    {
        totalMinTimeStep = 0;
        totalMaxTimeStep = 0;
    }

    emit updateTotalTimeStepRange( totalMinTimeStep, totalMaxTimeStep, isSingleObject );  // シグナル発行 // AFTER
}

void ObjectEditor::showFormRow( const QString& key )
{
    if( !m_label_field_map.contains( key ) ) return;

    QLabel* label = m_label_field_map[key].first;
    QWidget* field = m_label_field_map[key].second;

    ui->formLayout->addRow( label, field );
    if( label ) label->show();
    if( field ) field->show();
}

void ObjectEditor::hideFormRow( const QString& key )
{
    if( !m_label_field_map.contains( key ) ) return;

    QLabel* label = m_label_field_map[key].first;
    QFormLayout::TakeRowResult row = ui->formLayout->takeRow( label );
    if( row.labelItem && row.labelItem->widget() ) row.labelItem->widget()->hide();
    if( row.fieldItem && row.fieldItem->widget() ) row.fieldItem->widget()->hide();
}

void ObjectEditor::hideAllRow()
{
    QFormLayout* layout = ui->formLayout;
    for( auto it = m_label_field_map.begin(); it != m_label_field_map.end(); it++ )
    {
        QFormLayout::TakeRowResult row = layout->takeRow( it.value().first );
        if( row.labelItem && row.labelItem->widget() ) row.labelItem->widget()->hide();
        if( row.fieldItem && row.fieldItem->widget() ) row.fieldItem->widget()->hide();
    }
}

void ObjectEditor::commonProperty()
{
    showFormRow( "name" );
    showFormRow( "format" );
    showFormRow( "directory" );
    showFormRow( "timeStep" );
    showFormRow( "focus" );
}

void ObjectEditor::clientServerPointObjectProperty()
{
    showFormRow( "name" );
    showFormRow( "format" );
    showFormRow( "directory" );
    showFormRow( "timeStep" );
    showFormRow( "focus" );

    showFormRow( "vectors" );
    showFormRow( "elements" );
    showFormRow( "subvolume" );
    showFormRow( "nodes" );
    showFormRow( "elementType" );
    showFormRow( "fileType" );
    showFormRow( "stepNumber" );
    showFormRow( "coord1" );
    showFormRow( "coord2" );
    showFormRow( "coord3" );
    showFormRow( "export" );
    showFormRow( "particleLimit" );
    showFormRow( "density" );    
}

void ObjectEditor::inSituPointObjectProperty()
{
    showFormRow( "name" );
    showFormRow( "format" );
    showFormRow( "directory" );
    showFormRow( "timeStep" );
    showFormRow( "focus" );

    showFormRow( "vectors" );
    showFormRow( "particleLimit" );
    showFormRow( "density" );
}

void ObjectEditor::stlPolygonObjectProperty()
{
    showFormRow( "name" );
    showFormRow( "format" );
    showFormRow( "directory" );
    showFormRow( "timeStep" );
    showFormRow( "focus" );

    showFormRow( "color" );
    showFormRow( "opacity" );
}

void ObjectEditor::registerObject( QStandardItem* nameItem, const QStandardItem* formatItem )
{
    enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();
    std::unique_ptr<kvs::glsl::ParticleBasedRenderer> particleBasedRenderer;
    std::unique_ptr<kvs::StochasticPolygonRenderer> stochasticPolygonRenderer;
    std::unique_ptr<kvs::StochasticLineRenderer> stochasticLineRenderer;
    std::unique_ptr<kvs::StochasticTexturedPolygonRenderer> stochasticTexturedPolygonRenderer;

    switch( format )
    {
    case ObjectItem::Format::ServerPointObjectCS:
    case ObjectItem::Format::ServerPointObjectIS:
        particleBasedRenderer = std::make_unique<kvs::glsl::ParticleBasedRenderer>();
        particleBasedRenderer.get()->enableShuffle();
        emit shading( particleBasedRenderer.get() );
        nameItem->setData( QVariant::fromValue( m_screen->registerObject( nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PointObject*>(), particleBasedRenderer.release() ) ), ObjectItem::nameItemRole::Ids );
        break;

    case ObjectItem::Format::ServerGlyphObjectCS:
    case ObjectItem::Format::ServerGlyphObjectIS:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        nameItem->setData( QVariant::fromValue( m_screen->registerObject( nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PolygonObject*>(), stochasticPolygonRenderer.release() ) ), ObjectItem::nameItemRole::Ids );
        break;

    case ObjectItem::Format::PointObjectKVSML:
    case ObjectItem::Format::PointObjectLAS:
    case ObjectItem::Format::PointObjectPTS:
        particleBasedRenderer = std::make_unique<kvs::glsl::ParticleBasedRenderer>();
        particleBasedRenderer.get()->enableShuffle();
        emit shading( particleBasedRenderer.get() );
        // particleBasedRenderer.get()->disableZooming(); //必要かどうか協議
        nameItem->setData( QVariant::fromValue( m_screen->registerObject( nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PointObject*>(), particleBasedRenderer.release() ) ), ObjectItem::nameItemRole::Ids );
        break;

    case ObjectItem::Format::PolygonObjectKVSML:
    case ObjectItem::Format::PolygonObjectSTL:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        nameItem->setData( QVariant::fromValue( m_screen->registerObject( nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PolygonObject*>(), stochasticPolygonRenderer.release() ) ), ObjectItem::nameItemRole::Ids );
        break;

#if defined(ASSIMP)
#ifdef ASSIMP
    case ObjectItem::Format::PolygonObjectFBX:
    case ObjectItem::Format::PolygonObject3DS:
#endif
        stochasticTexturedPolygonRenderer = std::make_unique<kvs::StochasticTexturedPolygonRenderer>();
        emit shading( stochasticTexturedPolygonRenderer.get() );
        nameItem->setData( QVariant::fromValue( m_screen->registerObject( nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::TexturedPolygonObject*>(), stochasticTexturedPolygonRenderer.release() ) ), ObjectItem::nameItemRole::Ids );
        break;
#endif

    case ObjectItem::Format::LineObjectKVSML:
        stochasticLineRenderer = std::make_unique<kvs::StochasticLineRenderer>();
        emit shading( stochasticLineRenderer.get() );
        nameItem->setData( QVariant::fromValue( m_screen->registerObject( nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::LineObject*>(), stochasticLineRenderer.release() ) ), ObjectItem::nameItemRole::Ids );
        break;
    default:
        return;
    }
}

void ObjectEditor::replaceObject( QStandardItem* nameItem, const QStandardItem* formatItem )
{
    QPair<int,int> ids = nameItem->data( ObjectItem::nameItemRole::Ids ).value<QPair<int, int>>();
    enum ObjectItem::Format format = formatItem->data( ObjectItem::FormatItemRole::FormatValue ).value<enum ObjectItem::Format>();

    std::unique_ptr<kvs::StochasticPolygonRenderer> stochasticPolygonRenderer;

    switch( format )
    {
    case ObjectItem::Format::ServerPointObjectCS:
    case ObjectItem::Format::ServerPointObjectIS:
        m_screen->scene()->replaceObject( ids.first, nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PointObject*>() );
        break;

    case ObjectItem::Format::ServerGlyphObjectCS:
    case ObjectItem::Format::ServerGlyphObjectIS:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        m_screen->scene()->replaceObject( ids.first, nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PolygonObject*>() );
        m_screen->scene()->replaceRenderer( ids.second, stochasticPolygonRenderer.release() );
        break;

    case ObjectItem::Format::PointObjectKVSML:
    case ObjectItem::Format::PointObjectLAS:
    case ObjectItem::Format::PointObjectPTS:
        m_screen->scene()->replaceObject( ids.first, nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PointObject*>() );
        break;

    case ObjectItem::Format::PolygonObjectKVSML:
    case ObjectItem::Format::PolygonObjectSTL:
        stochasticPolygonRenderer = std::make_unique<kvs::StochasticPolygonRenderer>();
        emit shading( stochasticPolygonRenderer.get() );
        m_screen->scene()->replaceObject( ids.first, nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::PolygonObject*>() );
        m_screen->scene()->replaceRenderer( ids.second, stochasticPolygonRenderer.release() );
        break;

#if defined(ASSIMP)
#ifdef ASSIMP
    case ObjectItem::Format::PolygonObjectFBX:
    case ObjectItem::Format::PolygonObject3DS:
#endif
        m_screen->scene()->replaceObject( ids.first, nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::TexturedPolygonObject*>() );
        break;
#endif

    case ObjectItem::Format::LineObjectKVSML:
        m_screen->scene()->replaceObject( ids.first, nameItem->data( ObjectItem::nameItemRole::Object ).value<kvs::LineObject*>() );
        break;
    default:
        return;
    }
}

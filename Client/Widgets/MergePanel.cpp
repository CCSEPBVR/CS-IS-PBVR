#include "MergePanel.h"
#include "ui_MergePanel.h"
#include "App/pbvrgui.h"
/**
 * @brief MergePanel::MergePanel
 * @author TO0603
 * @date 2024/08/16
 * @param parent 親ウィジェット
 * @details
 * このコンストラクタはMergePanelオブジェクトを初期化し、UIをセットアップする。
 * またボタンに対するシグナルとスロットの接続を行う。
 * - `ui->browserPushButton` がクリックされたときに `onBrowser` 関数を呼び出します。
 * - `ui->applyPushButton` がクリックされたときに `onApply` 関数を呼び出します。
 *
 * メンバー変数 `m_files_manager` は、`FilesManager` オブジェクトのリストを管理するために初期化されます。
 */
MergePanel::MergePanel(QWidget *parent, PBVRGUI *pbvr_gui, Preference* preference, TimeControllerB* time_controller_b, TotalParticles* total_particles, Connect* connectUI, ShadingController* shading_controller ) :
    QDockWidget(parent),
    ui(new Ui::MergePanel),
    m_pbvr_gui( pbvr_gui ),
    m_preference( preference ),
    m_time_controller_b( time_controller_b ),
    m_total_particles( total_particles ),
    m_connect( connectUI ),
    m_shading_controller( shading_controller ),
    m_files_manager(),
    m_is_worker_thread_running( false ),
    m_is_particle_generation_needed( false ),
    m_is_export( false )
{
    ui->setupUi(this);

    connect(ui->filesTableWidget, &QTableWidget::cellDoubleClicked, this, &MergePanel::onFilesTWidgetCellDoubleClicked);
    connect( ui->browserPushButton, &QPushButton::clicked, this, &MergePanel::onBrowser );
    connect( ui->exportPushButton, &QPushButton::clicked, this, &MergePanel::onExport );
    connect( ui->centeringPushButton, &QPushButton::clicked, this, &MergePanel::onCentering );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &MergePanel::onApply );
}

/**
 * @brief MergePanel::~MergePanel
 * @author TO0603
 * @date 2024/08/16
 * @details
 * このデストラクタはm_files_managerリスト内の全てのFilesManagerオブジェクトとUIリソースを解放する。
 * qDeleteAllを使用して、m_files_manager内の全てのポインタを安全に削除し、uiオブジェクトを解放する。
 */
MergePanel::~MergePanel()
{
    qDeleteAll(m_files_manager);
    delete ui;
}

#include <QColorDialog>
void MergePanel::onFilesTWidgetCellDoubleClicked(int row, int column)
{
    if (column == 4) // Colorのセルをダブルクリックしているか。
    {
        if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML || m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL ) //フォーマットがテクスチャ無しポリゴンである場合
        {
            QTableWidgetItem* formatItem = ui->filesTableWidget->item( row, 4 );
            formatItem->setBackground(QColorDialog::getColor(Qt::gray));
        }
    }
}

/**
 * @brief MergePanel::registerFiles
 * @author TO0603
 * @date 2024/08/16
 * @param fileName ファイルダイアログで取得したファイルパス
 * @details
 * この関数は、指定されたファイルパスに基づいてFilesManagerオブジェクトを作成し、
 * そのフォーマットとタイムステップが適切であるかを確認した上で、リストに追加します。
 *
 * まず、FilesManagerの新しいインスタンスを動的に確保し、指定されたファイルパスの情報を設定します。
 * その後、ファイルフォーマットとタイムステップのチェックを行い、条件を満たす場合は以下の初期設定を行います:
 *
 * - テクスチャ無しポリゴン用に色をグレー（RGB: 128, 128, 128）に設定。
 * - テクスチャ無しポリゴン用に不透明度を0.5に設定。
 * - 初期状態ではオブジェクトは設定しないためnullptrに設定、それに伴いIDと表示されるタイムステップを-1に設定します。
 *
 * 最後に、この新しいFilesManagerオブジェクトをm_files_managerリストに追加し、テーブルウィジェットに表示されるようにします。
 * もし、フォーマットやタイムステップが条件を満たさない場合は、動的に確保したFilesManager2オブジェクトを削除して終了します。
 */
void MergePanel::registerFiles( const QString& filePath )
{
    //登録する予定のFilesManagerを確保
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( filePath ) );
    //    qDebug() << filePath;
    if( checkFormat( newFile ) && checkMinMaxTimeStep( newFile ) ) //m_format, m_min_time_step, m_max_time_stepの設定
    {
        newFile->setDisplay( true );
        newFile->setKeepInital( false );
        newFile->setKeepFinal( false );
        newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
        newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
        newFile->setChangePolygonTransferFunction( false );
        newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
        newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
        newFile->setObject( nullptr );

        m_files_manager.append( newFile );

        addFilesTable( m_files_manager.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    }
    else
    {
        delete newFile;
    }
}

void MergePanel::serverPointObjectCS( QString volumeDataFilePath, int min, int max )
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setFormat( FilesManager::ServerPointObjectCS );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setDisplay( true );
    newFile->setKeepInital( false );
    newFile->setKeepFinal( false );
    newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
    newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
    newFile->setChangePolygonTransferFunction( false );
    newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
    newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
    newFile->setObject( nullptr );
    m_files_manager.append( newFile );
    addFilesTable( m_files_manager.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    calculateTotalMinMaxTimeStep();
}

void MergePanel::serverPointObjectIS( QString volumeDataFilePath, int min, int max )
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setFormat( FilesManager::ServerPointObjectIS );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setDisplay( true );
    newFile->setKeepInital( false );
    newFile->setKeepFinal( false );
    newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
    newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
    newFile->setChangePolygonTransferFunction( false );
    newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
    newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
    newFile->setObject( nullptr );
    m_files_manager.append( newFile );
    addFilesTable( m_files_manager.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    calculateTotalMinMaxTimeStep();
    IS_OBJ = true;
    IS_OBJ_DONE_INIT = false;
}

void MergePanel::serverGlyphObjectCS( QString volumeDataFilePath, int min, int max )
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setFormat( FilesManager::ServerGlyphObjectCS );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setDisplay( true );
    newFile->setKeepInital( false );
    newFile->setKeepFinal( false );
    newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
    newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
    newFile->setChangePolygonTransferFunction( false );
    newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
    newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
    newFile->setObject( nullptr );
    m_files_manager.append( newFile );
    addFilesTable( m_files_manager.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    calculateTotalMinMaxTimeStep();
}

void MergePanel::serverGlyphObjectIS( QString volumeDataFilePath, int min, int max )
{
    FilesManager *newFile = new FilesManager;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setFormat( FilesManager::ServerGlyphObjectIS );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setDisplay( true );
    newFile->setKeepInital( false );
    newFile->setKeepFinal( false );
    newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
    newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
    newFile->setChangePolygonTransferFunction( false );
    newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
    newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
    newFile->setObject( nullptr );
    m_files_manager.append( newFile );
    addFilesTable( m_files_manager.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    calculateTotalMinMaxTimeStep();
    IS_OBJ = true;
    IS_OBJ_DONE_INIT = false;
}

void MergePanel::updateObjectTimeStepIS( int min, int max )
{
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS ||
            m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectIS )
        {
            m_files_manager[row]->setMinTimeStep( min );
            m_files_manager[row]->setMaxTimeStep( max );
            calculateTotalMinMaxTimeStep();
        }
    }
}

#include <QXmlStreamReader>
/**
 * @brief MergePanel::checkFormat
 * @author TO0603
 * @date 2024/08/16
 * @param newFile 追加を予定するFileManagerのオブジェクト
 * @details
 * この関数は、指定されたファイルの拡張子に基づいて、そのフォーマットを設定し
 * FilesManagerオブジェクトに適切な形式を設定する。
 *
 * ファイルが開けない場合はfalseを返し、何も処理を行わない。
 *
 * 対応している拡張子とそれに対応する処理は以下の通りである。
 * - `.kvsml`: XML解析を行い、`PointObject` または `PolygonObject` タグに基づいてフォーマットを設定する。
 * - `.stl`: 非テクスチャ付きポリゴンオブジェクトとしてフォーマットを設定する。
 * - `.fbx`: (PBVR_SUPPORT_FBX が有効な場合) テクスチャ付きポリゴンオブジェクトとしてフォーマットを設定する。
 * - `.3ds`: (PBVR_SUPPORT_3DS が有効な場合) テクスチャ付きポリゴンオブジェクトとしてフォーマットを設定する。
 * - `.las`: ポイントオブジェクトとしてフォーマットを設定する。
 * - `.pts`: ポイントオブジェクトとしてフォーマットを設定する。
 * XML解析中にエラーが発生した場合やサポートされていないファイル形式の場合はfalseを返す。
 */
bool MergePanel::checkFormat( FilesManager* newFile )
{
    QFile file( newFile->getFileInfo().filePath() );
    QString fileSuffix = newFile->getFileInfo().suffix().toLower();
    bool success = true;

    //ファイルが開けない場合、何もしない
    if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        success = false;
        return success;
    }

    //拡張子が.kvsmlだった場合の処理
    if( fileSuffix == "kvsml" )
    {
        QXmlStreamReader xml( &file );
        //終端でなく、エラーがない場合ループ
        while( !xml.atEnd() && !xml.hasError() )
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

        //XMLパースエラー
        if( xml.hasError() )
        {
            success = false;
            return success;
        }
    }
    else if( fileSuffix == "stl" ) //それ以外の拡張子であった場合( .stl .fbx .3ds .las .pts )
    {
        newFile->setFormat( FilesManager::NonTexturedPolygonObjectSTL );
    }
#ifdef PBVR_SUPPORT_FBX
    else if( fileSuffix == "fbx" )
    {
        newFile->setFormat( FilesManager::TexturedPolygonObjectFBX );
    }
#endif
#ifdef PBVR_SUPPORT_3DS
    else if( fileSuffix == "3ds" )
    {
        newFile->setFormat( FilesManager::TexturedPolygonObject3DS );
    }
#endif
    else if( fileSuffix == "las" )
    {
        newFile->setFormat( FilesManager::PointObjectLAS );
    }
    else if( fileSuffix == "pts" )
    {
        newFile->setFormat( FilesManager::PointObjectPTS );
    }
    else //NOT SUPPORTED FORMAT
    {
        success = false;
        return success;
    }
    return success;
}

#include <QDir>
/**
 * @brief MergePanel::checkMinMaxTimeStep
 * @author TO0603
 * @date 2024/08/16
 * @param newFile 追加を予定するFileManagerのオブジェクト
 * @details
 * この関数は、指定されたファイルのタイムステップ情報を取得し、その最小値と最大値をFilesManagerオブジェクトに設定する。
 * ファイル名のパターンに基づいてタイムステップを抽出するために正規表現を使用する。ファイルフォーマットによって、
 * 使用する正規表現のパターンが異なる。
 * - `.kvsml` ファイルの場合: ファイル名の最初の`_`の前の部分に基づいてパターンを生成する。
 * - それ以外のファイルの場合: ファイル名の最後の`_`の前の部分に基づいてパターンを生成する。
 *
 * ディレクトリ内の全てのファイルを対象に、正規表現パターンと一致するファイル名を探索し、その中で最も小さいタイムステップと
 * 最も大きいタイムステップを取得する。
 *
 * タイムステップの最小値と最大値が見つかった場合、それらをnewFileに設定し、trueを返す。
 * タイムステップが見つからなかった場合は、newFileに何も設定せず、falseを返す。
 */
bool MergePanel::checkMinMaxTimeStep( FilesManager* newFile )
{
    QRegularExpression regularExpression;
    bool success = true;
    switch ( newFile->getFormat() )
    {
    case FilesManager::PointObjectKVSML:
    case FilesManager::NonTexturedPolygonObjectKVSML:
        regularExpression.setPattern( newFile->getFileInfo().baseName().left( newFile->getFileInfo().baseName().indexOf( '_' )) + "_([0-9]+)\\.*" );
        break;
    default:
        regularExpression.setPattern( newFile->getFileInfo().baseName().left( newFile->getFileInfo().baseName().lastIndexOf( '_' )) + "_([0-9]+)\\.*" );
        break;
    }

    int minTimeStep = std::numeric_limits<int>::max();
    int maxTimeStep = std::numeric_limits<int>::min();

    foreach( const QFileInfo &fileInfo, newFile->getFileInfo().dir().entryInfoList( QDir::Files ) )
    {
        QRegularExpressionMatch match = regularExpression.match( fileInfo.fileName() );

        if( match.hasMatch() )
        {
            int nummericalValue = match.captured(1).toInt();
            if( nummericalValue < minTimeStep )
            {
                minTimeStep = nummericalValue;
            }
            if( nummericalValue > maxTimeStep )
            {
                maxTimeStep = nummericalValue;
            }
        }
    }

    if( minTimeStep != std::numeric_limits<int>::max() && maxTimeStep != std::numeric_limits<int>::min() )
    {
        //        qDebug() << minTimeStep << "," << maxTimeStep;
        newFile->setMinTimeStep( minTimeStep );
        newFile->setMaxTimeStep( maxTimeStep );
    }
    else//CAN NOT FIND MIN MAX TIME STEP
    {
        success = false;
        return success;
    }
    return success;
}

#include <QCheckBox>
#include <QDoubleSpinBox>
/**
 * @brief MergePanel::addFilesTable
 * @author TO0603
 * @date 2024/08/16
 * @param newFile 追加を予定するFileManagerのオブジェクト
 * @details
 * この関数はFilesManagerのオブジェクトを受け取り、その情報を基にテーブルに新しい行を追加する。
 * 追加される情報はファイルの名前、表示オプション、KeepInitial/Finalオプション,フォーマット、および必要に応じて色と不透明度が含まれる。
 *
 * 以下の項目がテーブルに追加される。
 * - **行ヘッダー**: ファイル名の一部 (`_` の前の部分)
 * - **チェックボックス**:
 *   - 表示 (初期状態: チェック)
 *   - 初期状態保持 (初期状態: チェックなし)
 *   - 最終状態保持 (初期状態: チェックなし)
 *   - 削除 (初期状態: チェックなし)
 * - **フォーマット**: ファイルのフォーマットに応じて設定されるテキスト (編集不可)
 * - **色と不透明度** (非テクスチャ付きポリゴンオブジェクトの場合):
 *   - 色: RGBで設定可能
 *   - 不透明度: 0.0 から 1.0 の範囲で設定可能
 *
 *   他のフォーマットの場合、色と不透明度のセルは空になる。
 */
void MergePanel::addFilesTable( FilesManager* newFile )
{
    int row = ui->filesTableWidget->rowCount(); //登録されている行数
    ui->filesTableWidget->setRowCount( row + 1 ); //行を追加

    QTableWidgetItem *headerItem = new QTableWidgetItem( newFile->getFileInfo().baseName().left( newFile->getFileInfo().baseName().indexOf('_') ) );
    ui->filesTableWidget->setVerticalHeaderItem(row, headerItem);

    QCheckBox *displayCheckBox = new QCheckBox();
    displayCheckBox->setCheckState( Qt::Checked );

    QCheckBox *keepInitialCheckBox = new QCheckBox();
    keepInitialCheckBox->setCheckState( Qt::Unchecked );

    QCheckBox *keepFinalCheckBox = new QCheckBox();
    keepFinalCheckBox->setCheckState( Qt::Unchecked );

    QTableWidgetItem* format = new QTableWidgetItem;
    format->setFlags( format->flags() & ~Qt::ItemIsEditable );
    format->setText( newFile->formatToString( newFile->getFormat() ) );

    if( newFile->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML || newFile->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
    {
        QTableWidgetItem *colorValue = new QTableWidgetItem;
        colorValue->setFlags( format->flags() & ~Qt::ItemIsEditable );
        colorValue->setBackground( newFile->getColor() );

        QDoubleSpinBox *opacityValue = new QDoubleSpinBox();
        opacityValue->setRange( 0.0, 1.0 );
        opacityValue->setSingleStep( 0.01 );
        opacityValue->setValue( newFile->getOpacity() );
        ui->filesTableWidget->setItem( ui->filesTableWidget->rowCount() - 1, 4, colorValue );
        ui->filesTableWidget->setCellWidget( ui->filesTableWidget->rowCount() - 1, 5, opacityValue);
    }
    else
    {
        QTableWidgetItem* empCell1 = new QTableWidgetItem;
        QTableWidgetItem* empCell2 = new QTableWidgetItem;
        empCell1->setFlags( empCell1->flags() & ~Qt::ItemIsEditable );
        empCell2->setFlags( empCell2->flags() & ~Qt::ItemIsEditable );
        ui->filesTableWidget->setItem( ui->filesTableWidget->rowCount() - 1, 4, empCell1 );
        ui->filesTableWidget->setItem( ui->filesTableWidget->rowCount() - 1, 5, empCell2 );
    }

    QCheckBox *deleteCheckBox = new QCheckBox;
    deleteCheckBox->setCheckState( Qt::Unchecked );

    ui->filesTableWidget->setCellWidget( ui->filesTableWidget->rowCount() - 1, 0, displayCheckBox );
    ui->filesTableWidget->setCellWidget( ui->filesTableWidget->rowCount() - 1, 1, keepInitialCheckBox );
    ui->filesTableWidget->setCellWidget( ui->filesTableWidget->rowCount() - 1, 2, keepFinalCheckBox );
    ui->filesTableWidget->setItem( ui->filesTableWidget->rowCount() - 1, 3, format );

    ui->filesTableWidget->setCellWidget( ui->filesTableWidget->rowCount() - 1, 6, deleteCheckBox );
}

/**
 * @brief MergePanel::calculateTotalMinMaxTimeStep
 * @author TO0603
 * @date 2024/08/19
 * @details
 * 登録されているデータの最小最大タイムステップを計算する。
 */
void MergePanel::calculateTotalMinMaxTimeStep()
{
    int totalMinTimeStep = std::numeric_limits<int>::max();
    int totalMaxTimeStep = std::numeric_limits<int>::min();
    int totalFiles;

    for( totalFiles = 0; totalFiles < m_files_manager.size(); totalFiles++ )
    {
        FilesManager* filesManager = m_files_manager[totalFiles];
        totalMinTimeStep = std::min( totalMinTimeStep, filesManager->getMinTimeStep() );
        totalMaxTimeStep = std::max( totalMaxTimeStep, filesManager->getMaxTimeStep() );
    }

    qDebug() << totalMinTimeStep << "," << totalMaxTimeStep;
    m_time_controller_b->updateMinMax( totalMinTimeStep, totalMaxTimeStep, totalFiles );
}

void MergePanel::updateCheckState()
{
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        QCheckBox *deleteCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 6 ) );

        if (deleteCheckBox && deleteCheckBox->checkState() == Qt::Checked)
        {
            m_pbvr_gui->screen()->scene()->removeObject( m_files_manager[row]->getIDs().first );

            m_files_manager[row]->setIDs( std::pair<int,int>(-1,-1) );
            ui->filesTableWidget->removeRow(row);
            delete deleteCheckBox;

            // m_files_managerからも対応する要素を削除する
            if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS || m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS )
            {
                m_connect->deletedServerObject();
                IS_OBJ = false;
                IS_OBJ_DONE_INIT = false;
            }

            if( m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectCS || m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectIS )
            {
                m_connect->sendGlyphFlagFalse();
            }
            delete m_files_manager[row];
            m_files_manager.removeAt(row);

            row--; // 行が削除されたので、ループのインデックスを調整する
        }

    }

    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        QCheckBox *displayCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 0 ) );
        QCheckBox *keepInitialCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 1 ) );
        QCheckBox *keepFinalCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 2 ) );

        m_files_manager[row]->setDisplay( displayCheckBox->isChecked() );
        m_files_manager[row]->setKeepInital( keepInitialCheckBox->isChecked() );
        m_files_manager[row]->setKeepFinal( keepFinalCheckBox->isChecked() );

        qDebug() << m_files_manager[row]->getDisplay();
        qDebug() << m_files_manager[row]->getKeepInitial();
        qDebug() << m_files_manager[row]->getKeepFinal();
    }

    calculateTotalMinMaxTimeStep();
}

void MergePanel::updatePolygonColorOpacity()
{
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML || m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
        {
            QTableWidgetItem* colorItem = ui->filesTableWidget->item(row, 4);
            QDoubleSpinBox* opacitySpinBox = dynamic_cast<QDoubleSpinBox*>(ui->filesTableWidget->cellWidget(row, 5));
            if( colorItem != nullptr && opacitySpinBox != nullptr )
            {
                QColor rgbColor = colorItem->background().color();
                double opacity = opacitySpinBox->value();
                if( m_files_manager[row]->getColor() != rgbColor )
                {
                    qDebug() << "COLOR CHANGE!";
                    m_files_manager[row]->setColor( rgbColor );
                    m_files_manager[row]->setChangePolygonTransferFunction( true );
                }
                if( m_files_manager[row]->getOpacity() != opacity )
                {
                    qDebug() << "OPACITY CHANGE!";
                    m_files_manager[row]->setOpacity( opacity );
                    m_files_manager[row]->setChangePolygonTransferFunction( true );
                }
            }
        }
    }
}

#include <QFileDialog>
/**
 * @brief MergePanel::onBrowser
 * @author TO0603
 * @date 2024/08/16
 * @details
 * この関数は、...ボタンが押された際にファイルブラウザダイアログを表示し、ユーザが選択したファイルを処理する。
 * ファイルダイアログは複数の既存ファイルを選択可能に設定されており、サポートしているファイル形式に基づいて
 * フィルタリングを行う。サポートされる形式はコンパイル時のオプションによって異なる。
 * - `PBVR_SUPPORT_FBX` および `PBVR_SUPPORT_3DS` が有効: `*.kvsml`, `*.stl`, `*.fbx`, `*.3ds`, `*.las`, `*.pts`
 * - `PBVR_SUPPORT_FBX` のみが有効: `*.kvsml`, `*.stl`, `*.fbx`, `*.las`, `*.pts`
 * - `PBVR_SUPPORT_3DS` のみが有効: `*.kvsml`, `*.stl`, `*.3ds`, `*.las`, `*.pts`
 * - どちらも無効: `*.kvsml`, `*.stl`, `*.las`, `*.pts`
 *
 * ユーザがファイルを選択し、ダイアログが'OK'で閉じられた場合、選択されたファイルのリストを取得し、
 * 各ファイルをregisterFilesを使って登録を行う。
 */
void MergePanel::onBrowser()
{
    QFileDialog fileDialog( this );
    fileDialog.setFileMode( QFileDialog::ExistingFiles );

#if defined( PBVR_SUPPORT_FBX ) && defined( PBVR_SUPPORT_3DS )
    fileDialog.setNameFilter("*.kvsml *.stl *.fbx *.3ds *.las *.pts");
#elif PBVR_SUPPORT_FBX
    fileDialog.setNameFilter("*.kvsml *.stl *.fbx *.las *.pts");
#elif PBVR_SUPPORT_3DS
    fileDialog.setNameFilter("*.kvsml *.stl *.3ds *.las *.pts");
#else
    fileDialog.setNameFilter("*.kvsml *.stl *.las *.pts");
#endif

    if( fileDialog.exec() )
    {
        QStringList selectedFiles = fileDialog.selectedFiles();
        // 選択されたファイルを処理
        for( const QString& fileName : selectedFiles )
        {
            registerFiles( fileName );
        }
    }
    calculateTotalMinMaxTimeStep();
}

void MergePanel::onExport()
{
    m_export_file_path = QFileDialog::getSaveFileName(this, tr("Save Server-Side Point Object"), QDir::homePath(), tr("すべてのファイル ( * )"));
    if ( !m_export_file_path.isEmpty() )
    {
        ui->exportPushButton->setEnabled( false );
        m_is_export = true;
    }
    else
    {
        m_is_export = false;
    }
}

#include "ExtendedKVS/CustomObjectManager.h"

void MergePanel::onCentering()
{
    CustomObjectManager* object_manager = static_cast<CustomObjectManager*>( m_pbvr_gui->screen()->scene()->objectManager() );
    kvs::Vec3 min_obj;
    kvs::Vec3 max_obj;
    int counter = 0;

    for (int row = 0; row < m_files_manager.size(); ++row)
    {
        if (m_pbvr_gui->screen()->scene()->object(m_files_manager[row]->getIDs().first)->isVisible())
        {
            ++counter;
            if (counter >= 2)
            {
                break;
            }
        }
    }

    if( counter == 1 )
    {
        kvs::Vec3 init_object_manager_min_object( 1e+06, 1e+06, 1e+06 );
        kvs::Vec3 init_object_manager_max_object( -1e+06, -1e+06, -1e+06 );
        for( int row = 0; row < m_files_manager.size(); row++ )
        {
            if( m_files_manager[row]->getIDs().first != -1 && m_files_manager[row]->getIDs().second != -1 )
            {
                auto* object = m_pbvr_gui->screen()->scene()->object( m_files_manager[row]->getIDs().first );
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
        kvs::Vec3 init_object_manager_min_object = m_pbvr_gui->screen()->scene()->objectManager()->minObjectCoord();
        kvs::Vec3 init_object_manager_max_object = m_pbvr_gui->screen()->scene()->objectManager()->maxObjectCoord();
        for( int row = 0; row < m_files_manager.size(); row++ )
        {
            if( m_files_manager[row]->getIDs().first != -1 && m_files_manager[row]->getIDs().second != -1 )
            {
                auto* object = m_pbvr_gui->screen()->scene()->object( m_files_manager[row]->getIDs().first );
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
    m_pbvr_gui->screen()->scene()->reset();
    m_pbvr_gui->screen()->update();
}

/**
 * @brief MergePanel::onApply
 * @author TO0603
 * @date 2024/08/16
 * @details
 * この関数は適用ボタンが押された際の処理を実行する。
 * この関数はユーザが適用ボタンを押した際に呼び出され、isEraseChecked関数を実行する。
 */
void MergePanel::onApply()
{
    updateCheckState();
    updatePolygonColorOpacity();
    totalParticles();
    m_pbvr_gui->screen()->update();
}

void MergePanel::mergeObjects( int currentTimeStep, int requestTimeStep )
{
    if( m_is_worker_thread_running )//スレッド処理実行中は何もしない。
    {
        return;
    }

    WorkerThread* workerThread = new WorkerThread( this );
    workerThread->setCurrentTimeStep( currentTimeStep );
    workerThread->setRequestTimeStep( requestTimeStep );
    connect( workerThread, &QThread::finished, workerThread, &QObject::deleteLater );
    connect( workerThread, &WorkerThread::workFinished, this, &MergePanel::onWorkerThreadFinished );
    workerThread->start();
    m_is_worker_thread_running = true;
}

MergePanel::WorkerThread::WorkerThread( MergePanel* gui ) : m_merge( gui )
{
}

//#include <kvs/ObjectManager>
#include <kvs/PointObject>
#include <kvs/ParticleBasedRenderer>
#include <kvs/PointImporter>
#include <kvs/PolygonObject>
#include <kvs/StochasticPolygonRenderer>
#include <kvs/PolygonImporter>
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
#include <kvs/TexturedPolygonObject>
#include <kvs/StochasticTexturedPolygonRenderer>
#include "ExtendedKVS/TexturedPolygonImporter.h"
#endif

/**
 * @brief MergePanel::WorkerThread::run
 * @author TO0603
 * @date 2024/08/23
 * @details
 * この関数は、m_files_managerの各ファイルに対して、そのデータ形式に応じたインポートの処理を行う。
 * 全てのデータの処理が完了後、作業が完了したことを知らせるworkFinished()を発行する。
 *
 * データ形式に応じた処理:
 * - ServerPointObjectCSまたはServerPointObjectISの場合:
 * データはサーバ/デーモンから取得するためインポーターの設定はない。kvs::PointObjectとkvs::glsl::ParticleBasedRendererを使用する。
 *
 * - PointObjectKVSML, PointObjectLAS, PointObjectPTS の場合:
 * kvs::PointImporter を使用して kvs::PointObject と kvs::glsl::ParticleBasedRenderer をインポートします。
 *
 * - NonTexturedPolygonObjectKVSML, NonTexturedPolygonObjectSTL の場合:
 * kvs::PolygonImporter を使用して kvs::PolygonObject と kvs::StochasticPolygonRenderer をインポートします
 * 。
 * - TexturedPolygonObjectFBX または TexturedPolygonObject3DS (FBXまたは3DSのサポートが有効な場合):
 * kvs::TexturedPolygonImporter を使用して kvs::TexturedPolygonObject と kvs::StochasticTexturedPolygonRenderer をインポートします。
 *
 * @note PBVR_SUPPORT_FBX または PBVR_SUPPORT_3DS のサポートがコンパイル時に有効化されている場合にのみ、
 *       TexturedPolygonObjectFBX や `TexturedPolygonObject3DS の処理を行う
 *
 * @see MergePanel::WorkerThread2::timeStepCheckAndImport
 */
void MergePanel::WorkerThread::run()
{
    for( int row = 0; row < m_merge->m_files_manager.size(); row++ )
    {
        switch ( m_merge->m_files_manager[row]->getFormat() )
        {
        case FilesManager::ServerPointObjectCS:
        case FilesManager::ServerPointObjectIS:
            timeStepCheckAndImport<void, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager::ServerGlyphObjectCS:
        case FilesManager::ServerGlyphObjectIS:
            timeStepCheckAndImport<void, kvs::PolygonObject, kvs::StochasticPolygonRenderer>( row );
            break;
        case FilesManager::PointObjectKVSML:
        case FilesManager::PointObjectLAS:
        case FilesManager::PointObjectPTS:
            timeStepCheckAndImport<kvs::PointImporter, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager::NonTexturedPolygonObjectKVSML:
        case FilesManager::NonTexturedPolygonObjectSTL:
            timeStepCheckAndImport<kvs::PolygonImporter, kvs::PolygonObject, kvs::StochasticPolygonRenderer>( row );
            break;
#ifdef PBVR_SUPPORT_FBX
        case FilesManager::TexturedPolygonObjectFBX:
#endif
#ifdef PBVR_SUPPORT_3DS
        case FilesManager::TexturedPolygonObject3DS:
#endif
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
            timeStepCheckAndImport<kvs::TexturedPolygonImporter, kvs::TexturedPolygonObject, kvs::StochasticTexturedPolygonRenderer>( row );
            break;
#endif
        default:
            break;
        }
    }
    emit workFinished();
}

/**
 * @brief MergePanel::WorkerThread::timeStepCheckAndImport
 * @author TO0603
 * @date 2024/08/23
 * @tparam Importer データをインポートするためのクラス。voidを指定した場合はサーバで粒子生成を行います。
 * @tparam ObjectType オブジェクトの型
 * @tparam RendererType レンダラーの型
 * @param row インポート処理を行うデータ行のインデックス
 * @details
 * データがシーンに登録されていない場合 :
 * - データがDisplayにチェックがされている場合、要求されたタイムステップがデータの範囲内かどうかを確認し、
 * 範囲内であればインポートを行います。 要求されたタイムステップが範囲外である場合、keepInitialまたはkeepFinalの設定に応じて
 * 最小、または最大タイムステップのデータをインポートします。
 *
 * データがシーンに登録されている場合 :
 * - Displayにチェックされているかを確認し、表示を制御します。既に要求されたタイムステップのデータがインポートされている場合、そのオブジェクトを表示する。
 * そうでない場合は、要求されたタイムステップに対応するデータをインポートしオブジェクトを表示します。要求されたタイムステップが範囲外である場合、keepInitialまたはkeepFinalの設定に応じて
 * 最小、または最大タイムステップのデータをインポートします。
 */

template <typename Importer, typename ObjectType, typename RendererType>
void MergePanel::WorkerThread::timeStepCheckAndImport( int row )
{
    // const QString            filePath    = m_merge->m_files_manager[row]->getFileInfo().filePath();
    const bool               display     = m_merge->m_files_manager[row]->getDisplay();
    const bool               keepInitial = m_merge->m_files_manager[row]->getKeepInitial();
    const bool               keepFinal   = m_merge->m_files_manager[row]->getKeepFinal();
    const int                minTimeStep = m_merge->m_files_manager[row]->getMinTimeStep();
    const int                maxTimeStep = m_merge->m_files_manager[row]->getMaxTimeStep();
    const std::pair<int,int> ids         = m_merge->m_files_manager[row]->getIDs();
    m_merge->m_files_manager[row]->setObject( nullptr );

    if( ids.first == -1 && ids.second == -1 ) // 一度もregisterObjectされたことない場合
    {
        if( display == true ) // 表示の要求がある場合
        {
            if( m_request_time_step >= minTimeStep && m_request_time_step <= maxTimeStep ) // 要求タイムステップが範囲内の場合
            {
                process<Importer, ObjectType>( row, m_request_time_step );
            }
            if( m_request_time_step < minTimeStep ) // 要求タイムステップが最小タイムステップよりも小さい場合
            {
                if( keepInitial == true ) // KeepInitialにチェックがついている。
                {
                    process<Importer, ObjectType>( row, minTimeStep );
                }
            }
            if( m_request_time_step > maxTimeStep ) // 要求タイムステップが最大タイムステップよりも大きい場合
            {
                if( keepFinal == true ) // KeepInitialにチェックがついている。
                {
                    process<Importer, ObjectType>( row, maxTimeStep );
                }
            }
        }
    }
    else // 既にregisterObjectされたことがある場合
    {
        auto* object = m_merge->m_pbvr_gui->screen()->scene()->object( m_merge->m_files_manager[row]->getIDs().first );
        if( display == true ) // 表示の要求がある場合
        {
            if( m_request_time_step >= minTimeStep && m_request_time_step <= maxTimeStep ) // 要求タイムステップが範囲内の場合
            {
                if( m_request_time_step == m_merge->m_files_manager[row]->getAlreadyImportedTimeStep() ) // 既に要求タイムステップをインポートしている場合
                {
                    if( ( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS && m_merge->getIsParticleGenerationNeeded() ) ||
                        ( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS && m_merge->getIsParticleGenerationNeeded() ) ) // 粒子生成の要求がある場合
                    {
                        process<Importer, ObjectType>( row, m_request_time_step );
                    }
                    if( object->isVisible() == false ) object->show(); // 不可視状態である場合、可視状態にする。
                }
                else // 要求タイムステップをインポートしていない場合
                {
                    process<Importer, ObjectType>( row, m_request_time_step );
                    if( object->isVisible() == false ) object->show(); // 不可視状態である場合、可視状態にする。
                }
            }
            if( m_request_time_step < minTimeStep ) // 要求タイムステップが最小タイムステップよりも小さい場合
            {
                if( keepInitial == true ) // KeepInitialにチェックがついている。
                {
                    if( minTimeStep >= m_merge->m_files_manager[row]->getAlreadyImportedTimeStep() ) // 既に最小タイムステップをインポートしている場合
                    {
                        if( ( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS && m_merge->getIsParticleGenerationNeeded() ) ||
                            ( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS && m_merge->getIsParticleGenerationNeeded() ) ) // 粒子生成の要求がある場合
                        {
                            process<Importer, ObjectType>( row, minTimeStep );
                            m_merge->setIsParticleGenerationNeeded( false );
                        }
                        if( object->isVisible() == false ) object->show(); // 不可視状態である場合、可視状態にする。
                    }
                    else // 要求タイムステップをインポートしていない場合
                    {
                        process<Importer, ObjectType>( row, minTimeStep );
                        if( object->isVisible() == false ) object->show();
                    }
                }
                else // KeepInitialにチェックがついていない場合
                {
                    object->hide();
                }
            }
            if( m_request_time_step > maxTimeStep ) // 要求タイムステップが最大タイムステップよりも大きい場合
            {
                if( keepFinal == true ) // KeepFinalにチェックがついている。
                {
                    if( maxTimeStep <= m_merge->m_files_manager[row]->getAlreadyImportedTimeStep() ) // 既に最大タイムステップをインポートしている場合
                    {
                        if( ( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS && m_merge->getIsParticleGenerationNeeded() ) ||
                            ( m_merge->m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS && m_merge->getIsParticleGenerationNeeded() ) ) // 粒子生成の要求がある場合
                        {
                            process<Importer, ObjectType>( row, maxTimeStep );
                            m_merge->setIsParticleGenerationNeeded( false );
                        }
                        if( object->isVisible() == false ) object->show(); // 不可視状態である場合、可視状態にする。
                    }
                    else // 要求タイムステップをインポートしていない場合
                    {
                        process<Importer, ObjectType>( row, maxTimeStep );
                        if( object->isVisible() == false ) object->show();
                    }
                }
                else // KeepInitialにチェックがついていない場合
                {
                    object->hide();
                }
            }
        }
        else
        {
            object->hide();
        }
    }
}

template <typename Importer, typename ObjectType>
void MergePanel::WorkerThread::process( const int row ,const int timeStep )
{
    const QString filePath = m_merge->m_files_manager[row]->getFileInfo().filePath();

    if constexpr ( !std::is_same_v<Importer, void > )
    {
        m_merge->m_files_manager[row]->setObject( new Importer( updateTimeStepInFileName( filePath, timeStep ) ) );
    }
    else if ( std::is_same_v<ObjectType, kvs::PointObject> )
    {
        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->generateParticles( timeStep ) );
        //m_merge->m_connect->generateGlyphPolygons( timeStep );
        //m_merge->m_connect->sendRecvPlotOverLine( timeStep ); //4 debug
        m_merge->setIsParticleGenerationNeeded( false );
    }
    else if (std::is_same_v<ObjectType, kvs::PolygonObject>)
    {
        m_merge->m_files_manager[row]->setObject( m_merge->m_connect->generateGlyphPolygons( timeStep ) );
    }
    m_merge->m_files_manager[row]->setAlreadyImportedTimeStep( timeStep );
}

std::string MergePanel::WorkerThread::updateTimeStepInFileName(QString fileName, int nextTimeStep)
{
    // 正規表現パターン: 5桁の数字
    QRegularExpression regex(R"(\d{5})");
    QRegularExpressionMatch match = regex.match(fileName);

    if (match.hasMatch())
    {
        // futureTimeの値を考慮して新しい5桁の数字を生成
        int newNumber = nextTimeStep;

        // 新しい5桁の数字をQStringに変換し、0埋めして格納
        QString extractedNumber = QString::number(newNumber).rightJustified(5, '0');

        // 5桁の数字を含む前後の文字列を抜き取り
        int startPos = match.capturedStart();
        int endPos = match.capturedEnd();
#ifdef Q_OS_WIN
        return ( fileName.left(startPos).replace( "/", "\\" ) + extractedNumber + fileName.mid(endPos) ).toLocal8Bit().constData();
#else
        return ( fileName.left(startPos) + extractedNumber + fileName.mid(endPos) ).toStdString();
#endif
    }
    else
    {
#ifdef Q_OS_WIN
        return ( fileName.replace( "/", "\\" ) ).toLocal8Bit().constData();
#else
        return ( fileName ).toStdString();
#endif
    }
}

#include <kvs/PointExporter>

void MergePanel::onWorkerThreadFinished()
{
    kvs::Xform beforeObjectManagerXform = m_pbvr_gui->screen()->scene()->objectManager()->xform();
    for( int row = 0; row < m_files_manager.size(); row++ ) //tableWidgetに登録されているアイテム分ループする。
    {
        if( m_files_manager[row]->getIDs().first == -1 && m_files_manager[row]->getIDs().second == -1 ) // 一度もregisterObjectされたことない場合
        {
            if( m_files_manager[row]->getObject() != nullptr ) // オブジェクトがインポートされている場合は登録を行う。
            {
                m_files_manager[row]->getObject()->setXform( beforeObjectManagerXform );
                if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS ||
                    m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS ||
                    m_files_manager[row]->getFormat() == FilesManager::PointObjectKVSML    ||
                    m_files_manager[row]->getFormat() == FilesManager::PointObjectLAS      ||
                    m_files_manager[row]->getFormat() == FilesManager::PointObjectPTS ) // ポイントオブジェクト
                {
                    kvs::PointObject* point_object = dynamic_cast<kvs::PointObject*>( m_files_manager[row]->getObject() );
                    kvs::RendererBase* particle_based_renderer = new kvs::glsl::ParticleBasedRenderer;
                    kvs::Vec3 translationOffset = m_pbvr_gui->screen()->scene()->camera()->xform().translation() - m_pbvr_gui->getInitializedCameraXform().translation();
                    static_cast<kvs::glsl::ParticleBasedRenderer*>(particle_based_renderer)->setTranslationOffset( translationOffset );
                    static_cast<kvs::glsl::ParticleBasedRenderer*>(particle_based_renderer)->setObjectDepth( m_pbvr_gui->screen()->scene()->objectManager()->xform().scaling().z() / m_pbvr_gui->screen()->scene()->camera()->xform().scaling().z() );
                    m_shading_controller->applyShading( particle_based_renderer );
                    m_files_manager[row]->setIDs( m_pbvr_gui->screen()->scene()->registerObject( point_object, particle_based_renderer ) );
                }
                if( m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectCS ||
                    m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectIS ) // テクスチャなしポリゴンオブジェクト
                {
                    kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>( m_files_manager[row]->getObject() );
                    // polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getColor().red(), m_files_manager[row]->getColor().green(), m_files_manager[row]->getColor().blue() ) );
                    // polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_polygon_renderer = new kvs::StochasticPolygonRenderer;
                    m_shading_controller->applyShading( stochastic_polygon_renderer );
                    m_files_manager[row]->setIDs( m_pbvr_gui->screen()->scene()->registerObject( polygon_object, stochastic_polygon_renderer ) );
                    m_files_manager[row]->setChangePolygonTransferFunction( false );
                }
                if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML ||
                    m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL ) // テクスチャなしポリゴンオブジェクト
                {
                    kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>( m_files_manager[row]->getObject() );
                    // polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getColor().red(), m_files_manager[row]->getColor().green(), m_files_manager[row]->getColor().blue() ) );
                    polygon_object->setMinMaxExternalCoords(kvs::Vec3(polygon_object->minExternalCoord().x(),polygon_object->minExternalCoord().y(),polygon_object->minExternalCoord().z() * 25),
                                                            kvs::Vec3(polygon_object->maxExternalCoord().x(),polygon_object->maxExternalCoord().y(),polygon_object->maxExternalCoord().z() * 25));
                    polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_polygon_renderer = new kvs::StochasticPolygonRenderer;
                    m_shading_controller->applyShading( stochastic_polygon_renderer );
                    m_files_manager[row]->setIDs( m_pbvr_gui->screen()->scene()->registerObject( polygon_object, stochastic_polygon_renderer ) );
                    m_files_manager[row]->setChangePolygonTransferFunction( false );
                }
#if defined( PBVR_SUPPORT_FBX )
                if( m_files_manager[row]->getFormat() == FilesManager::TexturedPolygonObjectFBX )
                {
                    kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>( m_files_manager[row]->getObject() );
                    textured_polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getColor().red(), m_files_manager[row]->getColor().green(), m_files_manager[row]->getColor().blue() ) );
                    textured_polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_textured_polygon_renderer = new kvs::StochasticTexturedPolygonRenderer;
                    m_shading_controller->applyShading( stochastic_textured_polygon_renderer );
                    m_files_manager[row]->setIDs( m_pbvr_gui->screen()->scene()->registerObject( textured_polygon_object, stochastic_textured_polygon_renderer ) );
                }
#endif
#if defined( PBVR_SUPPORT_3DS )
                if( m_files_manager[row]->getFormat() == FilesManager::TexturedPolygonObject3DS )
                {
                    kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>( m_files_manager[row]->getObject() );
                    textured_polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getColor().red(), m_files_manager[row]->getColor().green(), m_files_manager[row]->getColor().blue() ) );
                    textured_polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_textured_polygon_renderer = new kvs::StochasticTexturedPolygonRenderer;
                    m_shading_controller->applyShading( stochastic_textured_polygon_renderer );
                    m_files_manager[row]->setIDs( m_pbvr_gui->screen()->scene()->registerObject( textured_polygon_object, stochastic_textured_polygon_renderer ) );
                }
#endif
            }
        }
        else // 既にregisterObjectされたことがある場合
        {
            if( m_files_manager[row]->getObject() != nullptr ) //オブジェクトがインポートされていれば交換を行う。
            {
                if( kvs::PointObject* point_object = dynamic_cast<kvs::PointObject*>( m_files_manager[row]->getObject() ) ) //ポイントオブジェクトの場合
                {
                    if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS ||
                        m_files_manager[row]->getFormat() == FilesManager::PointObjectKVSML    ||
                        m_files_manager[row]->getFormat() == FilesManager::PointObjectLAS      ||
                        m_files_manager[row]->getFormat() == FilesManager::PointObjectPTS )
                    {
                        m_pbvr_gui->screen()->scene()->replaceObject(m_files_manager[row]->getIDs().first, point_object );
                    }
                    if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS )
                    {
                        if( m_time_controller_b->getTimeControllerA()->getCurrentTimeStepLineEdit()->value() == m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->value() )
                        {
                        }
                        else
                        {
                            m_pbvr_gui->screen()->scene()->replaceObject(m_files_manager[row]->getIDs().first, point_object );
                        }
                    }
                }
                if( kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>(m_files_manager[row]->getObject()) )
                {
                    if( m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectKVSML ||
                        m_files_manager[row]->getFormat() == FilesManager::NonTexturedPolygonObjectSTL )
                    {
                        // polygon_object->setColor( kvs::RGBColor( m_files_manager[row]->getColor().red(), m_files_manager[row]->getColor().green(), m_files_manager[row]->getColor().blue() ) );
                        polygon_object->setMinMaxExternalCoords(kvs::Vec3(polygon_object->minExternalCoord().x(),polygon_object->minExternalCoord().y(),polygon_object->minExternalCoord().z() * 25),
                                                                kvs::Vec3(polygon_object->maxExternalCoord().x(),polygon_object->maxExternalCoord().y(),polygon_object->maxExternalCoord().z() * 25));
                        polygon_object->setOpacity( m_files_manager[row]->getOpacity() * 255 );
                        m_pbvr_gui->screen()->scene()->replaceObject(m_files_manager[row]->getIDs().first, polygon_object );
                    }
                    if( m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectCS ||
                        m_files_manager[row]->getFormat() == FilesManager::ServerGlyphObjectIS )
                    {
                        kvs::RendererBase* renderer = new kvs::StochasticPolygonRenderer;
                        m_shading_controller->applyShading( renderer );
                        m_pbvr_gui->screen()->scene()->replaceObject(m_files_manager[row]->getIDs().first, polygon_object );
                        m_pbvr_gui->screen()->scene()->replaceRenderer(m_files_manager[row]->getIDs().second, renderer );
                    }
                }
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
                if( kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>( m_files_manager[row]->getObject() ) )
                {
                    m_pbvr_gui->screen()->scene()->replaceObject(m_files_manager[row]->getIDs().first, textured_polygon_object );
                }
#endif
            }
            if( m_files_manager[row]->getChangePolygonTransferFunction() == true ) //色不透明度の変更がある場合はポリゴンを作り替える。
            {
                auto* polygonObject = dynamic_cast<kvs::PolygonObject*>( m_pbvr_gui->screen()->scene()->object( m_files_manager[row]->getIDs().first ) );

                kvs::RGBColor color( m_files_manager[row]->getColor().red(), m_files_manager[row]->getColor().green(), m_files_manager[row]->getColor().blue() );
                int opacity = m_files_manager[row]->getOpacity() * 255;

                kvs::PolygonObject* copiedObject = new kvs::PolygonObject( *polygonObject );
                copiedObject->setColor( color );
                copiedObject->setOpacity( opacity );

                m_pbvr_gui->screen()->scene()->replaceObject( m_files_manager[row]->getIDs().first, copiedObject );

                m_files_manager[row]->setChangePolygonTransferFunction( false );
            }
        }

        if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS && m_is_export == true )
        {
            for( int i = m_files_manager[row]->getMinTimeStep(); i <= m_files_manager[row]->getMaxTimeStep(); i++ )
            {
                // 現在のタイムステップに対応するファイルのパスを生成
                QFileInfo fileInfo( m_export_file_path + "_" + QString( "%1" ).arg( i, 5, 10, QChar('0') ) + ".kvsml" );

                // ファイルが存在するかどうかをチェック
                if( fileInfo.exists() )
                {
                    continue;  // ファイルが存在する場合は次のタイムステップへ
                }
                else
                {
                    // getAlreadyImportedTimeStep の値を使用して新しいファイルを生成
                    QFileInfo newFileInfo( m_export_file_path + "_" + QString( "%1" ).arg( m_files_manager[row]->getAlreadyImportedTimeStep(), 5, 10, QChar('0') ) + ".kvsml" );
                    auto* pointObject = dynamic_cast<kvs::PointObject*>( m_pbvr_gui->screen()->scene()->objectManager()->object( m_files_manager[row]->getIDs().first ) );
                    if( pointObject )
                    {
                        kvs::KVSMLPointObject* kvsml = new kvs::PointExporter<kvs::KVSMLPointObject>( pointObject );

                        if (kvsml) // kvsml が正しく作成された場合のみ処理を実行
                        {
                            kvsml->setWritingDataTypeToExternalBinary();
#ifdef Q_OS_WIN
                            kvsml->write( newFileInfo.filePath().replace( "/","\\" ).toLocal8Bit().constData() );
#else
                            kvsml->write( newFileInfo.filePath().toStdString() );
#endif
                            delete kvsml;
                        }
                        else
                        {
                            // kvsml の作成が失敗した場合のエラーハンドリング（必要であれば追加）
                            m_is_export = false;
                            ui->exportPushButton->setEnabled(true);
                            return;
                        }
                    }
                }
            }

            bool allTimeStepExported = true;
            for( int i = m_files_manager[row]->getMinTimeStep(); i <= m_files_manager[row]->getMaxTimeStep(); i++ )
            {
                QFileInfo fileInfo( m_export_file_path + "_" + QString( "%1" ).arg( i, 5, 10, QChar('0') ) + ".kvsml" );
                if( !fileInfo.exists() )
                {
                    allTimeStepExported = false;
                }
            }

            if( allTimeStepExported == true )
            {
                m_is_export = false;
                ui->exportPushButton->setEnabled(true);
            }
        }
    }
    if( m_files_manager.size() == 0 )
    {
        m_time_controller_b->getTimeControllerA()->getCurrentTimeStepLineEdit()->setValue( -1 );
    }
    else
    {
        if( IS_OBJ == false )
        {
            m_time_controller_b->getTimeControllerA()->getCurrentTimeStepLineEdit()->setValue( m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->value() );
        }
        else
        {
            if( IS_OBJ_DONE_INIT == false )
            {
                m_time_controller_b->getTimeControllerA()->getCurrentTimeStepLineEdit()->setValue( m_connect->getServerMessage()->m_last_step );
                m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->setValue( m_connect->getServerMessage()->m_last_step );
                IS_OBJ_DONE_INIT = true;
            }
            else
            {
                m_time_controller_b->getTimeControllerA()->getCurrentTimeStepLineEdit()->setValue( m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->value() );
            }
        }
    }
    m_preference->setCurrentTimeStep( m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->value() );
    totalParticles();
    m_is_worker_thread_running = false;
    m_pbvr_gui->screen()->update();
    m_time_controller_b->updateTimeStep();
}

void MergePanel::totalParticles()
{
    int totalParticles = 0;
    for( int row = 0; row < m_files_manager.size(); row++ )
    {
        if( m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectCS ||
            m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS ||
            m_files_manager[row]->getFormat() == FilesManager::PointObjectKVSML ||
            m_files_manager[row]->getFormat() == FilesManager::PointObjectLAS ||
            m_files_manager[row]->getFormat() == FilesManager::PointObjectPTS )
        {
            auto* object = m_pbvr_gui->screen()->scene()->object( m_files_manager[row]->getIDs().first );
            if( object && object->isVisible() )
            {
                if (auto* pointObject = dynamic_cast<kvs::PointObject*>(object))
                {
                    totalParticles += pointObject->numberOfVertices();
                }
            }
        }
    }
    m_total_particles->setTotalParticles( totalParticles );
}

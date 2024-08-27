#include "MergePanel2.h"
#include "ui_MergePanel2.h"

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
MergePanel2::MergePanel2(QWidget *parent, Preference* preference, TimeControllerB* time_controller_b, TotalParticles* total_particles, Connect* connectUI, ShadingController* shading_controller ) :
    QDockWidget(parent),
    ui(new Ui::MergePanel2),
    m_preference( preference ),
    m_time_controller_b( time_controller_b ),
    m_total_particles( total_particles ),
    m_connect( connectUI ),
    m_shading_controller( shading_controller ),
    m_files_manager2(),
    m_is_worker_thread_running( false )
{
    ui->setupUi(this);

    connect(ui->filesTableWidget, &QTableWidget::cellDoubleClicked, this, &MergePanel2::onFilesTWidgetCellDoubleClicked);
    connect( ui->browserPushButton, &QPushButton::clicked, this, &MergePanel2::onBrowser );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &MergePanel2::onApply );
}

/**
 * @brief MergePanel::~MergePanel
 * @author TO0603
 * @date 2024/08/16
 * @details
 * このデストラクタはm_files_managerリスト内の全てのFilesManagerオブジェクトとUIリソースを解放する。
 * qDeleteAllを使用して、m_files_manager内の全てのポインタを安全に削除し、uiオブジェクトを解放する。
 */
MergePanel2::~MergePanel2()
{
    qDeleteAll(m_files_manager2);
    delete ui;
}

#include <QColorDialog>
void MergePanel2::onFilesTWidgetCellDoubleClicked(int row, int column)
{
    if (column == 4) // Colorのセルをダブルクリックしているか。
    {
        if( m_files_manager2[row]->getFormat() == FilesManager2::NonTexturedPolygonObjectKVSML || m_files_manager2[row]->getFormat() == FilesManager2::NonTexturedPolygonObjectSTL ) //フォーマットがテクスチャ無しポリゴンである場合
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
void MergePanel2::registerFiles( const QString& filePath )
{
    //登録する予定のFilesManagerを確保
    FilesManager2 *newFile = new FilesManager2;
    newFile->setFileInfo( QFileInfo( filePath ) );
    //    qDebug() << filePath;
    if( checkFormat( newFile ) && checkMinMaxTimeStep( newFile ) ) //m_format, m_min_time_step, m_max_time_stepの設定
    {
        newFile->setDisplay( true );
        newFile->setKeepInital( false );
        newFile->setKeepFinal( false );
        newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
        newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
        newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
        newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
        newFile->setObject( nullptr );

        m_files_manager2.append( newFile );

        addFilesTable( m_files_manager2.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    }
    else
    {
        delete newFile;
    }
}

void MergePanel2::serverObjectCS( QString volumeDataFilePath, int min, int max )
{
    FilesManager2 *newFile = new FilesManager2;
    newFile->setFileInfo( QFileInfo( volumeDataFilePath ) );
    newFile->setFormat( FilesManager2::ServerPointObjectCS );
    newFile->setMinTimeStep( min );
    newFile->setMaxTimeStep( max );
    newFile->setDisplay( true );
    newFile->setKeepInital( false );
    newFile->setKeepFinal( false );
    newFile->setColor( QColor( 128, 128, 128 ) ); //テクスチャ無しポリゴン用の初期値
    newFile->setOpacity( 0.5 ); //テクスチャ無しポリゴン用の初期値
    newFile->setIDs( std::pair<int,int>( -1, -1 ) ); //登録時に表示されることはないので-1となる。registerObjectされた際に値が決まる。
    newFile->setAlreadyImportedTimeStep( -1 ); //登録時に表示されることはないので-1となる。
    newFile->setObject( nullptr );
    m_files_manager2.append( newFile );
    addFilesTable( m_files_manager2.last() ); //アペンド直後のFilesManagerをテーブルウィジェットに追加する。
    calculateTotalMinMaxTimeStep();
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
bool MergePanel2::checkFormat( FilesManager2* newFile )
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
                    newFile->setFormat( FilesManager2::PointObjectKVSML );
                    break;
                }
                else if( tagName == "PolygonObject" )
                {
                    newFile->setFormat( FilesManager2::NonTexturedPolygonObjectKVSML );
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
        newFile->setFormat( FilesManager2::NonTexturedPolygonObjectSTL );
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
        newFile->setFormat( FilesManager2::PointObjectLAS );
    }
    else if( fileSuffix == "pts" )
    {
        newFile->setFormat( FilesManager2::PointObjectPTS );
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
bool MergePanel2::checkMinMaxTimeStep( FilesManager2* newFile )
{
    QRegularExpression regularExpression;
    bool success = true;
    switch ( newFile->getFormat() )
    {
    case FilesManager2::PointObjectKVSML:
    case FilesManager2::NonTexturedPolygonObjectKVSML:
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
void MergePanel2::addFilesTable( FilesManager2* newFile )
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

    if( newFile->getFormat() == FilesManager2::NonTexturedPolygonObjectKVSML || newFile->getFormat() == FilesManager2::NonTexturedPolygonObjectSTL )
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
void MergePanel2::calculateTotalMinMaxTimeStep()
{
    int totalMinTimeStep = std::numeric_limits<int>::max();
    int totalMaxTimeStep = std::numeric_limits<int>::min();
    int count;
    bool isSingleObject = true;
    bool isNoObject = true;

    for( count = 0; count < m_files_manager2.size(); count++ )
    {
        FilesManager2* filesManager = m_files_manager2[count];
        totalMinTimeStep = std::min( totalMinTimeStep, filesManager->getMinTimeStep() );
        totalMaxTimeStep = std::max( totalMaxTimeStep, filesManager->getMaxTimeStep() );

        if( count >= 1 )
        {
            isSingleObject = false;
        }
        isNoObject = false;
    }

    if( isNoObject )
    {
        totalMinTimeStep = 0;
        totalMaxTimeStep = 0;
    }
    qDebug() << totalMinTimeStep << "," << totalMaxTimeStep;
    m_time_controller_b->updateMinMax( totalMinTimeStep, totalMaxTimeStep, isSingleObject );
}

/**
 * @brief MergePanel::isEraseChecked
 * @author TO0603
 * @date 2024/08/16
 * @details
 * この関数はファイルテーブルを走査し、削除チェックボックスがチェックされている行のファイルを削除する。
 * 削除が確認されたファイルは以下の処理が行われる。
 * - シーンからオブジェクトを削除 (コメントアウトされているコードにて)
 * - `FilesManager` オブジェクトのIDをリセット
 * - テーブルから該当行を削除
 * - `m_files_manager` リストから該当オブジェクトを削除し、そのメモリを解放
 *
 * 行を削除するとテーブルのインデックスが変わるため、ループ内でインデックスの調整を行う。
 */
void MergePanel2::isEraseChecked()
{
    for (int row = 0; row < m_files_manager2.size(); row++)
    {
        QWidget *widget = ui->filesTableWidget->cellWidget(row, 6); // deleteCheckBox の列を指定
        QCheckBox *deleteCheckBox = qobject_cast<QCheckBox*>(widget);
        if (deleteCheckBox && deleteCheckBox->checkState() == Qt::Checked)
        {
//            m_screen->scene()->removeObject( m_files_manager2[row]->getIDs().first );

            m_files_manager2[row]->setIDs( std::pair<int,int>(-1,-1) );
            ui->filesTableWidget->removeRow(row);
            delete deleteCheckBox;

            // m_files_managerからも対応する要素を削除する
            //            if( m_files_manager2[row]->getFormat() == FilesManager::ServerPointObjectCS || m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS )
            //            {
            //                m_connect->deletedServerObject();
            //            }
            delete m_files_manager2[row];
            m_files_manager2.removeAt(row);

            row--; // 行が削除されたので、ループのインデックスを調整する
        }
    }
    calculateTotalMinMaxTimeStep();
}

void MergePanel2::updateCheckState()
{
    for( int row = 0; row < m_files_manager2.size(); row++ )
    {
        QCheckBox *deleteCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 6 ) );

        if (deleteCheckBox && deleteCheckBox->checkState() == Qt::Checked)
        {
            m_screen->scene()->removeObject( m_files_manager2[row]->getIDs().first );

            m_files_manager2[row]->setIDs( std::pair<int,int>(-1,-1) );
            ui->filesTableWidget->removeRow(row);
            delete deleteCheckBox;

            // m_files_managerからも対応する要素を削除する
            //            if( m_files_manager2[row]->getFormat() == FilesManager::ServerPointObjectCS || m_files_manager[row]->getFormat() == FilesManager::ServerPointObjectIS )
            //            {
            //                m_connect->deletedServerObject();
            //            }
            delete m_files_manager2[row];
            m_files_manager2.removeAt(row);

            row--; // 行が削除されたので、ループのインデックスを調整する
        }

    }

    for( int row = 0; row < m_files_manager2.size(); row++ )
    {
        QCheckBox *displayCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 0 ) );
        QCheckBox *keepInitialCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 1 ) );
        QCheckBox *keepFinalCheckBox = qobject_cast<QCheckBox*>( ui->filesTableWidget->cellWidget( row, 2 ) );

        m_files_manager2[row]->setDisplay( displayCheckBox->isChecked() );
        m_files_manager2[row]->setKeepInital( keepInitialCheckBox->isChecked() );
        m_files_manager2[row]->setKeepFinal( keepFinalCheckBox->isChecked() );

        qDebug() << m_files_manager2[row]->getDisplay();
        qDebug() << m_files_manager2[row]->getKeepInitial();
        qDebug() << m_files_manager2[row]->getKeepFinal();
    }

    calculateTotalMinMaxTimeStep();
}

void MergePanel2::updatePolygonColorOpacity()
{
    for( int row = 0; row < m_files_manager2.size(); row++ )
    {
        if( m_files_manager2[row]->getFormat() == FilesManager2::NonTexturedPolygonObjectKVSML || m_files_manager2[row]->getFormat() == FilesManager2::NonTexturedPolygonObjectSTL )
        {
            QTableWidgetItem* colorItem = ui->filesTableWidget->item(row, 4);
            QDoubleSpinBox* opacitySpinBox = dynamic_cast<QDoubleSpinBox*>(ui->filesTableWidget->cellWidget(row, 5));
            if( colorItem != nullptr && opacitySpinBox != nullptr )
            {
                QColor rgbColor = colorItem->background().color();
                double opacity = opacitySpinBox->value();
                if( m_files_manager2[row]->getColor() != rgbColor )
                {
                    qDebug() << "COLOR CHANGE!";
                    m_files_manager2[row]->setColor( rgbColor );
                    m_files_manager2[row]->setChangePolygonTransferFunction( true );
                }
                if( m_files_manager2[row]->getOpacity() != opacity )
                {
                    qDebug() << "OPACITY CHANGE!";
                    m_files_manager2[row]->setOpacity( opacity );
                    m_files_manager2[row]->setChangePolygonTransferFunction( true );
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
void MergePanel2::onBrowser()
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

/**
 * @brief MergePanel::onApply
 * @author TO0603
 * @date 2024/08/16
 * @details
 * この関数は適用ボタンが押された際の処理を実行する。
 * この関数はユーザが適用ボタンを押した際に呼び出され、isEraseChecked関数を実行する。
 */
void MergePanel2::onApply()
{
    //    isEraseChecked();
    updateCheckState();
    updatePolygonColorOpacity();
}

void MergePanel2::mergeObjects( int currentTimeStep, int requestTimeStep )
{
    if( m_is_worker_thread_running )//スレッド処理実行中は何もしない。
    {
        return;
    }

    WorkerThread2* workerThread = new WorkerThread2( this );
    workerThread->setCurrentTimeStep( currentTimeStep );
    workerThread->setRequestTimeStep( requestTimeStep );
    connect( workerThread, &QThread::finished, workerThread, &QObject::deleteLater );
    connect( workerThread, &WorkerThread2::workFinished, this, &MergePanel2::onWorkerThreadFinished );
    workerThread->start();
    m_is_worker_thread_running = true;
}

MergePanel2::WorkerThread2::WorkerThread2( MergePanel2* gui ) : m_merge( gui )
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
 * @see MergePanel2::WorkerThread2::timeStepCheckAndImport
 */
void MergePanel2::WorkerThread2::run()
{
    for( int row = 0; row < m_merge->m_files_manager2.size(); row++ )
    {
        switch ( m_merge->m_files_manager2[row]->getFormat() )
        {
        case FilesManager2::ServerPointObjectCS:
        case FilesManager2::ServerPointObjectIS:
            timeStepCheckAndImport<void, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager2::PointObjectKVSML:
        case FilesManager2::PointObjectLAS:
        case FilesManager2::PointObjectPTS:
            timeStepCheckAndImport<kvs::PointImporter, kvs::PointObject, kvs::glsl::ParticleBasedRenderer>( row );
            break;
        case FilesManager2::NonTexturedPolygonObjectKVSML:
        case FilesManager2::NonTexturedPolygonObjectSTL:
            timeStepCheckAndImport<kvs::PolygonImporter, kvs::PolygonObject, kvs::StochasticPolygonRenderer>( row );
            break;
#ifdef PBVR_SUPPORT_FBX
        case FilesManager2::TexturedPolygonObjectFBX:
#endif
#ifdef PBVR_SUPPORT_3DS
        case FilesManager2::TexturedPolygonObject3DS:
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
void MergePanel2::WorkerThread2::timeStepCheckAndImport( int row )
{
    const QString filePath = m_merge->m_files_manager2[row]->getFileInfo().filePath();
    const bool display = m_merge->m_files_manager2[row]->getDisplay();
    const bool keepInitial = m_merge->m_files_manager2[row]->getKeepInitial();
    const bool keepFinal = m_merge->m_files_manager2[row]->getKeepFinal();
    const int minTimeStep = m_merge->m_files_manager2[row]->getMinTimeStep();
    const int maxTimeStep = m_merge->m_files_manager2[row]->getMaxTimeStep();
    const std::pair<int,int> ids = m_merge->m_files_manager2[row]->getIDs();
    m_merge->m_files_manager2[row]->setObject( nullptr );

    if( ids.first == -1 && ids.second == -1 )
    {
        qDebug() << "対象のデータはシーンに登録されていません。" << "[" << row << "]" << __LINE__;
        if( display == true )
        {
            qDebug() << "対象のデータはDisplayにチェックがついています。" << "[" << row << "]" << __LINE__;
            if( m_request_time_step >= minTimeStep && m_request_time_step <= maxTimeStep )
            {
                qDebug() << "対象のデータは要求されたタイムステップの範囲内です。" << "[" << row << "]" << __LINE__;
                qDebug() << "要求されたタイムステップのデータをインポートします。" << "[" << row << "]" << __LINE__;
                if constexpr (!std::is_same_v<Importer, void>)
                {
                    m_merge->m_files_manager2[row]->setObject( new Importer( updateTimeStepInFileName( filePath, m_request_time_step ) ) );
                }
                else
                {
                    m_merge->m_files_manager2[row]->setObject( m_merge->m_connect->generateParticles( m_request_time_step ) );
                }
                m_merge->m_files_manager2[row]->setAlreadyImportedTimeStep( m_request_time_step );
            }

            if( m_request_time_step < minTimeStep )
            {
                qDebug() << "要求されたタイムステップが対象のデータの最小タイムステップよりも小さいです。" << "[" << row << "]" << __LINE__;
                if( keepInitial == true )
                {
                    qDebug() << "対象のデータはKeepInitialにチェックがついています。" << "[" << row << "]" << __LINE__;
                    qDebug() << "対象のデータの最小タイムステップのデータをインポートします。" << "[" << row << "]" << __LINE__;
                    if constexpr (!std::is_same_v<Importer, void>)
                    {
                        m_merge->m_files_manager2[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ) ) );
                    }
                    else
                    {
                        m_merge->m_files_manager2[row]->setObject( m_merge->m_connect->generateParticles( minTimeStep ) );
                    }
                    m_merge->m_files_manager2[row]->setAlreadyImportedTimeStep( minTimeStep );
                }
            }

            if( m_request_time_step > maxTimeStep )
            {
                qDebug() << "要求されたタイムステップが対象のデータの最大タイムステップよりも大きいです。" << "[" << row << "]" << __LINE__;
                if( keepFinal == true )
                {
                    qDebug() << "対象のデータはKeepFinalにチェックがついています。" << "[" << row << "]" << __LINE__;
                    qDebug() << "対象のデータの最大タイムステップのデータをインポートします。" << "[" << row << "]" << __LINE__;
                    if constexpr (!std::is_same_v<Importer, void>)
                    {
                        m_merge->m_files_manager2[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ) ) );
                    }
                    else
                    {
                        m_merge->m_files_manager2[row]->setObject( m_merge->m_connect->generateParticles( maxTimeStep ) );
                    }
                    m_merge->m_files_manager2[row]->setAlreadyImportedTimeStep( maxTimeStep );
                }
            }
        }
    }
    else
    {
        qDebug() << "対象のデータはシーンに登録されています。" << "[" << row << "]" << __LINE__;
        auto* object = m_merge->m_screen->scene()->object( m_merge->m_files_manager2[row]->getIDs().first );

        if( display == true )
        {
            qDebug() << "対象のデータはDisplayにチェックがついています。" << "[" << row << "]" << __LINE__;
            if( m_request_time_step >= minTimeStep && m_request_time_step <= maxTimeStep )
            {
                qDebug() << "対象のデータは要求されたタイムステップの範囲内です。" << "[" << row << "]" << __LINE__;
                if( m_merge->m_files_manager2[row]->getAlreadyImportedTimeStep() == m_request_time_step )
                {
                    qDebug() << "既に要求されたタイムステップのデータをインポートしています。" << "[" << row << "]" << __LINE__;
                    if( object->isVisible() == false ) object->show();
                }
                else
                {
                    qDebug() << "要求されたタイムステップのデータをインポートします。" << "[" << row << "]" << __LINE__;
                    if constexpr (!std::is_same_v<Importer, void>)
                    {
                        m_merge->m_files_manager2[row]->setObject( new Importer( updateTimeStepInFileName( filePath, m_request_time_step ) ) );
                    }
                    else
                    {
                        m_merge->m_files_manager2[row]->setObject( m_merge->m_connect->generateParticles( m_request_time_step ) );
                    }
                    m_merge->m_files_manager2[row]->setAlreadyImportedTimeStep( m_request_time_step );
                    if( object->isVisible() == false ) object->show();
                }
            }

            if( m_request_time_step < minTimeStep )
            {
                qDebug() << "要求されたタイムステップが対象のデータの最小タイムステップよりも小さいです。" << "[" << row << "]" << __LINE__;
                if( keepInitial == true ) //KeepInitialが有効
                {
                    qDebug() << "対象のデータはKeepInitialにチェックがついています。" << "[" << row << "]" << __LINE__;
                    if( m_merge->m_files_manager2[row]->getAlreadyImportedTimeStep() <= minTimeStep )
                    {
                        qDebug() << "既に対象データの最小タイムステップのデータをインポートしています。" << "[" << row << "]" << __LINE__;
                        if( object->isVisible() == false ) object->show();
                    }
                    else
                    {
                        qDebug() << "対象のデータの最小タイムステップのデータをインポートします。" << "[" << row << "]" << __LINE__;
                        if constexpr (!std::is_same_v<Importer, void>)
                        {
                            m_merge->m_files_manager2[row]->setObject( new Importer( updateTimeStepInFileName( filePath, minTimeStep ) ) );
                        }
                        else
                        {
                            m_merge->m_files_manager2[row]->setObject( m_merge->m_connect->generateParticles( minTimeStep ) );
                        }
                        m_merge->m_files_manager2[row]->setAlreadyImportedTimeStep( minTimeStep );
                        if( object->isVisible() == false ) object->show();
                    }
                }
                else
                {
                    qDebug() << "対象のデータはKeepInitialにチェックがついていないため非表示にします。" << "[" << row << "]" << __LINE__;
                    object->hide();
                }
            }

            if( m_request_time_step > maxTimeStep )
            {
                qDebug() << "要求されたタイムステップが対象のデータの最大タイムステップよりも大きいです。" << "[" << row << "]" << __LINE__;
                if( keepFinal == true ) //KeepFinalが有効
                {
                    qDebug() << "対象のデータはKeepFinalにチェックがついています。" << "[" << row << "]" << __LINE__;
                    if( m_merge->m_files_manager2[row]->getAlreadyImportedTimeStep() >= maxTimeStep )
                    {
                        qDebug() << "既に対象データの最大タイムステップのデータをインポートしています。" << "[" << row << "]" << __LINE__;
                        if( object->isVisible() == false ) object->show();
                    }
                    else
                    {
                        qDebug() << "対象のデータの最大タイムステップのデータをインポートします。" << "[" << row << "]" << __LINE__;
                        if constexpr (!std::is_same_v<Importer, void>)
                        {
                            m_merge->m_files_manager2[row]->setObject( new Importer( updateTimeStepInFileName( filePath, maxTimeStep ) ) );
                        }
                        else
                        {
                            m_merge->m_files_manager2[row]->setObject( m_merge->m_connect->generateParticles( maxTimeStep ) );
                        }
                        m_merge->m_files_manager2[row]->setAlreadyImportedTimeStep( maxTimeStep );
                        if( object->isVisible() == false ) object->show();
                    }
                }
                else
                {
                    qDebug() << "対象のデータはKeepFinalにチェックがついていないため非表示にします。" << "[" << row << "]" << __LINE__;
                    object->hide();
                }
            }
        }
        else
        {
            qDebug() << "対象のデータはDisplayにチェックがついていないため非表示にします。" << "[" << row << "]" << __LINE__;;
            object->hide();
        }
    }
}

std::string MergePanel2::WorkerThread2::updateTimeStepInFileName(QString fileName, int nextTimeStep)
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

void MergePanel2::onWorkerThreadFinished()
{
    kvs::Xform before_object_manager_xform = m_screen->scene()->objectManager()->xform();
    for( int row = 0; row < m_files_manager2.size(); row++ ) //登録されているアイテム分ループを行う。
    {
        if( m_files_manager2[row]->getIDs().first == -1 && m_files_manager2[row]->getIDs().second == -1 ) //オブジェクトが登録されていない場合
        {
            if( m_files_manager2[row]->getObject() != nullptr ) //オブジェクトがインポートされていれば登録を行う
            {
                qDebug() << "オブジェクトがインポートされているため登録を行います。";
                m_files_manager2[row]->getObject()->setXform( before_object_manager_xform );

                if( kvs::PointObject* point_object = dynamic_cast<kvs::PointObject*>(m_files_manager2[row]->getObject()) )
                {
                    kvs::RendererBase* particle_based_renderer = new kvs::glsl::ParticleBasedRenderer;
                    m_shading_controller->applyShading( particle_based_renderer );
                    m_files_manager2[row]->setIDs( m_screen->scene()->registerObject( point_object, particle_based_renderer ) );
                }
                else if( kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>(m_files_manager2[row]->getObject()) )
                {
                    polygon_object->setColor( kvs::RGBColor( m_files_manager2[row]->getColor().red(), m_files_manager2[row]->getColor().green(), m_files_manager2[row]->getColor().blue() ) );
                    polygon_object->setOpacity( m_files_manager2[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_polygon_renderer = new kvs::StochasticPolygonRenderer;
                    m_shading_controller->applyShading( stochastic_polygon_renderer );
                    m_files_manager2[row]->setIDs( m_screen->scene()->registerObject( polygon_object, stochastic_polygon_renderer ) );
                    m_files_manager2[row]->setChangePolygonTransferFunction( false );
                }
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
                else if( kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>(m_files_manager2[row]->getObject()) )
                {
                    textured_polygon_object->setColor( kvs::RGBColor( m_files_manager2[row]->getColor().red(), m_files_manager2[row]->getColor().green(), m_files_manager2[row]->getColor().blue() ) );
                    textured_polygon_object->setOpacity( m_files_manager2[row]->getOpacity() * 255 );
                    kvs::RendererBase* stochastic_textured_polygon_renderer = new kvs::StochasticTexturedPolygonRenderer;
                    m_shading_controller->applyShading( stochastic_textured_polygon_renderer );
                    m_files_manager2[row]->setIds( m_screen->scene()->registerObject( textured_polygon_object, stochastic_textured_polygon_renderer ) );
                }
#endif
            }
        }
        else
        {
//            auto* object = m_screen->scene()->object( m_files_manager2[row]->getIDs().first );
            if( m_files_manager2[row]->getObject() != nullptr ) //オブジェクトがインポートされていれば交換を行う。
            {
                if( kvs::PointObject* point_object = dynamic_cast<kvs::PointObject*>(m_files_manager2[row]->getObject()) )
                {
                    m_screen->scene()->replaceObject(m_files_manager2[row]->getIDs().first, point_object );
                }
                else if( kvs::PolygonObject* polygon_object = dynamic_cast<kvs::PolygonObject*>(m_files_manager2[row]->getObject()) )
                {
                    polygon_object->setColor( kvs::RGBColor( m_files_manager2[row]->getColor().red(), m_files_manager2[row]->getColor().green(), m_files_manager2[row]->getColor().blue() ) );
                    polygon_object->setOpacity( m_files_manager2[row]->getOpacity() * 255 );
                    m_screen->scene()->replaceObject(m_files_manager2[row]->getIDs().first, polygon_object );
                }
#if defined( PBVR_SUPPORT_FBX ) || defined( PBVR_SUPPORT_3DS )
                else if( kvs::TexturedPolygonObject* textured_polygon_object = dynamic_cast<kvs::TexturedPolygonObject*>(m_files_manager2[row]->getObject()) )
                {
                    m_screen->scene()->replaceObject(m_files_manager2[row]->getIds().first, textured_polygon_object );
                }
#endif
            }

            if( m_files_manager2[row]->getChangePolygonTransferFunction() == true ) //色不透明度の変更がある場合はポリゴンを作り替える。
            {
                auto* polygonObject = dynamic_cast<kvs::PolygonObject*>( m_screen->scene()->object( m_files_manager2[row]->getIDs().first ) );

                kvs::RGBColor color( m_files_manager2[row]->getColor().red(), m_files_manager2[row]->getColor().green(), m_files_manager2[row]->getColor().blue() );
                int opacity = m_files_manager2[row]->getOpacity() * 255;

                kvs::PolygonObject* copiedObject = new kvs::PolygonObject( *polygonObject );
                copiedObject->setColor( color );
                copiedObject->setOpacity( opacity );

                m_screen->scene()->replaceObject( m_files_manager2[row]->getIDs().first, copiedObject );

                m_files_manager2[row]->setChangePolygonTransferFunction( false );
            }
        }
    }
    m_time_controller_b->getTimeControllerA()->getCurrentTimeStepLineEdit()->setValue( m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->value() );
    m_preference->setCurrentTimeStep( m_time_controller_b->getTimeControllerA()->getJumpTimeStepSpinBox()->value() );
    totalParticles();
    m_is_worker_thread_running = false;
    m_screen->update();
    m_time_controller_b->updateTimeStep();
}

void MergePanel2::totalParticles()
{
    int totalParticles = 0;
    for( int row = 0; row < m_files_manager2.size(); row++ )
    {
        if( m_files_manager2[row]->getFormat() == FilesManager2::ServerPointObjectCS ||
            m_files_manager2[row]->getFormat() == FilesManager2::PointObjectKVSML ||
            m_files_manager2[row]->getFormat() == FilesManager2::PointObjectLAS ||
            m_files_manager2[row]->getFormat() == FilesManager2::PointObjectPTS )
        {
            auto* object = m_screen->scene()->object( m_files_manager2[row]->getIDs().first );
            if( object->isVisible() )
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

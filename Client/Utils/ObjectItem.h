#ifndef OBJECTITEM_H
#define OBJECTITEM_H

#include <QStandardItem>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QDir>

class ObjectItem
{
public:
    enum Format
    {
        // Error
        Unknown                       = 0,  // Aka Error
        // Server Sise Object
        ServerPointObjectCS           = 1,  // Server side Point Object
        ServerPointObjectIS           = 2,  // Server side Point Object
        ServerGlyphObjectCS           = 3,  // Server side Point Object
        ServerGlyphObjectIS           = 4,  // Server side Point Object
        // Local Object
        PointObjectKVSML              = 5,  // Point Object(.kvsml)
        PointObjectLAS                = 6,  // Point Object(.las)
        PointObjectPTS                = 7,  // Point Object(.pts)
        PolygonObjectKVSML            = 8,  // Polygon Object(.kvsml)
        PolygonObjectSTL              = 9,  // Polygon Object(.stl)
        PolygonObject3DS              = 10, // Polygon Object(.3ds)
        PolygonObjectFBX              = 11, // Polygon Object(.fbx)
        LineObjectKVSML               = 12  // Line Object(.kvsml)
    };

    enum nameItemRole
    {
        // 全オブジェクト共通
        Name = Qt::UserRole + 1,    // ファイル名
        Extension,                  // 拡張子
        Format,                     // フォーマット
        DirectoryPath,              // ディレクトリーパス
        MinMaxTimeStep,             // 最小最大タイムステップ
        Object,                     // オブジェクト
        Ids,                        // Ids<ObjectID,RendererID>
        RequestReplace,             // 同一タイムステップ再生成再読み込み要求
        ImportedTimeStep,           // インポート済みのタイムステップ
        TemporaryFocus,             // フォーカス(未確定)
        CurrentFocus,               // フォーカス(確定)
        InitialMinObjectCoord,      // 初期最小オブジェクト座標
        InitialMaxObjectCoord,      // 初期最大オブジェクト座標
        InitialMinExternalCoord,    // 初期最小?座標
        InitialMaxExternalCoord,    // 初期最大?座標
        CurrentMinObjectCoord,      // 選択した最小オブジェクト座標
        CurrentMaxObjectCoord,      // 選択した最大オブジェクト座標

        // Client Server(CS)用
        NumberOfVectors,            // 成分数
        NumberOfElements,           // 要素数
        NumberOfSubVolume,          // 分割数
        NumberOfNodes,              // 頂点数
        ElementType,                // 要素タイプ
        FileType,                   // ファイルタイプ...(フィルタの分割形式)
        StepNumber,                 // タイムステップ数...(不要?)
        TemporaryCoordinate1,       // 座標軸変換式1(未確定)
        CurrentCoordinate1,         // 座標軸変換式1(確定)
        TemporaryCoordinate2,       // 座標軸変換式2(未確定)
        CurrentCoordinate2,         // 座標軸変換式2(確定)
        TemporaryCoordinate3,       // 座標軸変換式3(未確定)
        CurrentCoordinate3,         // 座標軸変換式3(確定)
        RequestExport,              // エクスポート要求
        ExportFilePath,             // エクスポートファイルパス

        // Client Server(CS),In-Situ(IS)共通
        TemporaryParticleLimit,     // パーティクルリミット(未確定)
        CurrentParticleLimit,       // パーティクルリミット(確定)
        TemporaryDensity,           // デンシティ(未確定)
        CurrentDensity,             // デンシティ(確定)

        // テクスチャ無しポリゴンオブジェクト用
        TemporaryPolygonColor,      // ポリゴンカラー(未確定)
        CurrentPolygonColor,        // ポリゴンカラー(確定)
        TemporaryPolygonOpacity,    // ポリゴンオパシティ(未確定)
        CurrentPolygonOpacity       // ポリゴンオパシティ(確定)
    };

    enum FormatItemRole
    {
        FormatValue     = Qt::UserRole + 1
    };

    enum DisplayCheckBoxItemRole
    {
        Isdisplay       = Qt::UserRole + 1
    };

    enum KeepInitialCheckBoxItemRole
    {
        IskeepInitial   = Qt::UserRole + 1
    };

    enum KeepFinalCheckBoxItemRole
    {
        IskeepFinal     = Qt::UserRole + 1
    };

    ObjectItem() {}

    QList<QStandardItem*> createItems() const
    {
        QList<QStandardItem*> row;

        // 全オブジェクト共通
        QVariant name                       = "";
        QVariant extension                  = "";
        QVariant format                     = "";
        QVariant directoryPath              = "";
        QVariant minMaxTimeStep             = "";
        QVariant object;
        QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
        QVariant requestReplace             = QVariant::fromValue( false );
        QVariant importedTimeStep           = QVariant::fromValue( -1 );
        QVariant temporaryFocus             = QVariant::fromValue( false );
        QVariant currentFocus               = QVariant::fromValue( false );
        QVariant initialMinObjectCoord;
        QVariant initialMaxObjectCoord;
        QVariant initialMinExternalCoord;
        QVariant initialMaxExternalCoord;
        QVariant currentMinObjectCoord;
        QVariant currentMaxObjectCoord;
        // Client Server(CS)用
        QVariant numberOfVectors;
        QVariant numberOfElements;
        QVariant numberOfSubVolume;
        QVariant numberOfNodes;
        QVariant elementType;
        QVariant fileType;
        QVariant stepNumber;
        QVariant temporaryCoordinate1       = QVariant::fromValue( QString( "" ) );
        QVariant currentCoordinate1         = QVariant::fromValue( QString( "" ) );
        QVariant temporaryCoordinate2       = QVariant::fromValue( QString( "" ) );
        QVariant currentCoordinate2         = QVariant::fromValue( QString( "" ) );
        QVariant temporaryCoordinate3       = QVariant::fromValue( QString( "" ) );
        QVariant currentCoordinate3         = QVariant::fromValue( QString( "" ) );
        QVariant requestExport              = QVariant::fromValue( false );
        QVariant exportFilePath             = QVariant::fromValue( QString( "" ) );
        // Client Server(CS), In-Situ(IS)共通
        QVariant temporaryParticleLimit     = QVariant::fromValue( 10000000 );
        QVariant currentParticleLimit       = QVariant::fromValue( 10000000 );
        QVariant temporaryDensity           = QVariant::fromValue( 1 );
        QVariant currentDensity             = QVariant::fromValue( 1 );
        // テクスチャ無しポリゴンオブジェクト用
        QVariant temporaryPolygonColor      = QVariant::fromValue( QColor( 128, 128, 128 ) );
        QVariant currentPolygonColor        = QVariant::fromValue( QColor( 128, 128, 128 ) );
        QVariant temporaryPolygonOpacity    = QVariant::fromValue( 0.5 );
        QVariant currentPolygonOpacity      = QVariant::fromValue( 0.5 );

        QStandardItem* nameItem = new QStandardItem( "" );
        nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
        nameItem->setData( name                     , nameItemRole::Name );
        nameItem->setData( extension                , nameItemRole::Extension );
        nameItem->setData( format                   , nameItemRole::Format );
        nameItem->setData( directoryPath            , nameItemRole::DirectoryPath );
        nameItem->setData( minMaxTimeStep           , nameItemRole::MinMaxTimeStep );
        nameItem->setData( object                   , nameItemRole::Object );
        nameItem->setData( ids                      , nameItemRole::Ids );
        nameItem->setData( requestReplace           , nameItemRole::RequestReplace );
        nameItem->setData( importedTimeStep         , nameItemRole::ImportedTimeStep );
        nameItem->setData( temporaryFocus           , nameItemRole::TemporaryFocus );
        nameItem->setData( currentFocus             , nameItemRole::CurrentFocus );
        nameItem->setData( initialMinObjectCoord    , nameItemRole::InitialMinObjectCoord );
        nameItem->setData( initialMaxObjectCoord    , nameItemRole::InitialMaxObjectCoord );
        nameItem->setData( initialMinExternalCoord  , nameItemRole::InitialMinExternalCoord );
        nameItem->setData( initialMaxExternalCoord  , nameItemRole::InitialMaxExternalCoord );
        nameItem->setData( currentMinObjectCoord    , nameItemRole::CurrentMinObjectCoord );
        nameItem->setData( currentMaxObjectCoord    , nameItemRole::CurrentMaxObjectCoord );
        nameItem->setData( numberOfVectors          , nameItemRole::NumberOfVectors );
        nameItem->setData( numberOfElements         , nameItemRole::NumberOfElements );
        nameItem->setData( numberOfSubVolume        , nameItemRole::NumberOfSubVolume );
        nameItem->setData( numberOfNodes            , nameItemRole::NumberOfNodes );
        nameItem->setData( elementType              , nameItemRole::ElementType );
        nameItem->setData( fileType                 , nameItemRole::FileType );
        nameItem->setData( stepNumber               , nameItemRole::StepNumber );
        nameItem->setData( temporaryCoordinate1     , nameItemRole::TemporaryCoordinate1 );
        nameItem->setData( currentCoordinate1       , nameItemRole::CurrentCoordinate1 );
        nameItem->setData( temporaryCoordinate2     , nameItemRole::TemporaryCoordinate2 );
        nameItem->setData( currentCoordinate2       , nameItemRole::CurrentCoordinate2 );
        nameItem->setData( temporaryCoordinate3     , nameItemRole::TemporaryCoordinate3 );
        nameItem->setData( currentCoordinate3       , nameItemRole::CurrentCoordinate3 );
        nameItem->setData( requestExport            , nameItemRole::RequestExport );
        nameItem->setData( exportFilePath           , nameItemRole::ExportFilePath );
        nameItem->setData( temporaryParticleLimit   , nameItemRole::TemporaryParticleLimit );
        nameItem->setData( currentParticleLimit     , nameItemRole::CurrentParticleLimit );
        nameItem->setData( temporaryDensity         , nameItemRole::TemporaryDensity );
        nameItem->setData( currentDensity           , nameItemRole::CurrentDensity );
        nameItem->setData( temporaryPolygonColor    , nameItemRole::TemporaryPolygonColor );
        nameItem->setData( currentPolygonColor      , nameItemRole::CurrentPolygonColor );
        nameItem->setData( temporaryPolygonOpacity  , nameItemRole::TemporaryPolygonOpacity );
        nameItem->setData( currentPolygonOpacity    , nameItemRole::CurrentPolygonOpacity );

        QStandardItem* formatItem = new QStandardItem();
        formatItem->setData( format, FormatValue );
        formatItem->setText( formatToString( format.value<enum Format>() ) );       // 表示用

        QStandardItem* displayCheckBoxItem = new QStandardItem();
        displayCheckBoxItem->setCheckable( true );
        displayCheckBoxItem->setCheckState( Qt::Checked );
        displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
        QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
        displayCheckBoxItem->setData( isdisplay, Isdisplay );

        QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
        keepInitialCheckBoxItem->setCheckable( true );
        keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
        keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
        QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
        keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

        QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
        keepFinalCheckBoxItem->setCheckable( true );
        keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
        keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
        QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
        keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

        row.append( nameItem );
        row.append( formatItem );
        row.append( displayCheckBoxItem );
        row.append( keepInitialCheckBoxItem );
        row.append( keepFinalCheckBoxItem );

        return row;  // QList<QStandardItem*> を返す
    }

    // ローカルオブジェクト用
    QList<QStandardItem*> createItems( const QFileInfo& fileInfo ) const
    {
        QList<QStandardItem*> row;

        // 全オブジェクト共通
        QVariant name                       = extractBaseName( fileInfo );
        QVariant extension                  = extractExtension( fileInfo );
        QVariant format                     = extractFormat( fileInfo );
        QVariant directoryPath              = extractDirectoryPath( fileInfo );
        QVariant minMaxTimeStep             = QVariant::fromValue( extractMinMaxTimeStep( fileInfo ) );
        QVariant object;
        QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
        QVariant requestReplace             = QVariant::fromValue( false );
        QVariant importedTimeStep           = QVariant::fromValue( -1 );
        QVariant temporaryFocus             = QVariant::fromValue( false );
        QVariant currentFocus               = QVariant::fromValue( false );
        QVariant initialMinObjectCoord;
        QVariant initialMaxObjectCoord;
        QVariant initialMinExternalCoord;
        QVariant initialMaxExternalCoord;
        QVariant currentMinObjectCoord;
        QVariant currentMaxObjectCoord;
        // Client Server(CS)用
        QVariant numberOfVectors;
        QVariant numberOfElements;
        QVariant numberOfSubVolume;
        QVariant numberOfNodes;
        QVariant elementType;
        QVariant fileType;
        QVariant stepNumber;
        QVariant temporaryCoordinate1       = QVariant::fromValue( QString( "" ) );
        QVariant currentCoordinate1         = QVariant::fromValue( QString( "" ) );
        QVariant temporaryCoordinate2       = QVariant::fromValue( QString( "" ) );
        QVariant currentCoordinate2         = QVariant::fromValue( QString( "" ) );
        QVariant temporaryCoordinate3       = QVariant::fromValue( QString( "" ) );
        QVariant currentCoordinate3         = QVariant::fromValue( QString( "" ) );
        QVariant requestExport              = QVariant::fromValue( false );
        QVariant exportFilePath             = QVariant::fromValue( QString( "" ) );
        // Client Server(CS), In-Situ(IS)共通
        QVariant temporaryParticleLimit     = QVariant::fromValue( 10000000 );
        QVariant currentParticleLimit       = QVariant::fromValue( 10000000 );
        QVariant temporaryDensity           = QVariant::fromValue( 1 );
        QVariant currentDensity             = QVariant::fromValue( 1 );
        // テクスチャ無しポリゴンオブジェクト用
        QVariant temporaryPolygonColor      = QVariant::fromValue( QColor( 128, 128, 128 ) );
        QVariant currentPolygonColor        = QVariant::fromValue( QColor( 128, 128, 128 ) );
        QVariant temporaryPolygonOpacity    = QVariant::fromValue( 0.5 );
        QVariant currentPolygonOpacity      = QVariant::fromValue( 0.5 );

        QStandardItem* nameItem = new QStandardItem( extractBaseName( fileInfo ) );
        nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
        nameItem->setData( name                     , nameItemRole::Name );
        nameItem->setData( extension                , nameItemRole::Extension );
        nameItem->setData( format                   , nameItemRole::Format );
        nameItem->setData( directoryPath            , nameItemRole::DirectoryPath );
        nameItem->setData( minMaxTimeStep           , nameItemRole::MinMaxTimeStep );
        nameItem->setData( object                   , nameItemRole::Object );
        nameItem->setData( ids                      , nameItemRole::Ids );
        nameItem->setData( requestReplace           , nameItemRole::RequestReplace );
        nameItem->setData( importedTimeStep         , nameItemRole::ImportedTimeStep );
        nameItem->setData( temporaryFocus           , nameItemRole::TemporaryFocus );
        nameItem->setData( currentFocus             , nameItemRole::CurrentFocus );
        nameItem->setData( initialMinObjectCoord    , nameItemRole::InitialMinObjectCoord );
        nameItem->setData( initialMaxObjectCoord    , nameItemRole::InitialMaxObjectCoord );
        nameItem->setData( initialMinExternalCoord  , nameItemRole::InitialMinExternalCoord );
        nameItem->setData( initialMaxExternalCoord  , nameItemRole::InitialMaxExternalCoord );
        nameItem->setData( currentMinObjectCoord    , nameItemRole::CurrentMinObjectCoord );
        nameItem->setData( currentMaxObjectCoord    , nameItemRole::CurrentMaxObjectCoord );
        nameItem->setData( numberOfVectors          , nameItemRole::NumberOfVectors );
        nameItem->setData( numberOfElements         , nameItemRole::NumberOfElements );
        nameItem->setData( numberOfSubVolume        , nameItemRole::NumberOfSubVolume );
        nameItem->setData( numberOfNodes            , nameItemRole::NumberOfNodes );
        nameItem->setData( elementType              , nameItemRole::ElementType );
        nameItem->setData( fileType                 , nameItemRole::FileType );
        nameItem->setData( stepNumber               , nameItemRole::StepNumber );
        nameItem->setData( temporaryCoordinate1     , nameItemRole::TemporaryCoordinate1 );
        nameItem->setData( currentCoordinate1       , nameItemRole::CurrentCoordinate1 );
        nameItem->setData( temporaryCoordinate2     , nameItemRole::TemporaryCoordinate2 );
        nameItem->setData( currentCoordinate2       , nameItemRole::CurrentCoordinate2 );
        nameItem->setData( temporaryCoordinate3     , nameItemRole::TemporaryCoordinate3 );
        nameItem->setData( currentCoordinate3       , nameItemRole::CurrentCoordinate3 );
        nameItem->setData( requestExport            , nameItemRole::RequestExport );
        nameItem->setData( exportFilePath           , nameItemRole::ExportFilePath );
        nameItem->setData( temporaryParticleLimit   , nameItemRole::TemporaryParticleLimit );
        nameItem->setData( currentParticleLimit     , nameItemRole::CurrentParticleLimit );
        nameItem->setData( temporaryDensity         , nameItemRole::TemporaryDensity );
        nameItem->setData( currentDensity           , nameItemRole::CurrentDensity );
        nameItem->setData( temporaryPolygonColor    , nameItemRole::TemporaryPolygonColor );
        nameItem->setData( currentPolygonColor      , nameItemRole::CurrentPolygonColor );
        nameItem->setData( temporaryPolygonOpacity  , nameItemRole::TemporaryPolygonOpacity );
        nameItem->setData( currentPolygonOpacity    , nameItemRole::CurrentPolygonOpacity );

        QStandardItem* formatItem = new QStandardItem();
        formatItem->setData( format, FormatValue );
        formatItem->setText( formatToString( format.value<enum Format>() ) );       // 表示用

        QStandardItem* displayCheckBoxItem = new QStandardItem();
        displayCheckBoxItem->setCheckable( true );
        displayCheckBoxItem->setCheckState( Qt::Checked );
        displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
        QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
        displayCheckBoxItem->setData( isdisplay, Isdisplay );

        QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
        keepInitialCheckBoxItem->setCheckable( true );
        keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
        keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
        QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
        keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

        QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
        keepFinalCheckBoxItem->setCheckable( true );
        keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
        keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
        QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
        keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

        row.append( nameItem );
        row.append( formatItem );
        row.append( displayCheckBoxItem );
        row.append( keepInitialCheckBoxItem );
        row.append( keepFinalCheckBoxItem );

        if( isValid( row ) )
        {
            return row;  // QList<QStandardItem*> を返す
        }
        else
        {
            return isValid( row ) ? row : QList<QStandardItem*>();
        }
    }

private:
    QString extractBaseName( const QFileInfo& fileInfo ) const
    {
        QString baseName = fileInfo.baseName();
        int underScorePosition = baseName.indexOf( '_' );
        if( underScorePosition != -1 )
        {
            return baseName.left( underScorePosition );
        }
        return "";
    }

    QString extractExtension( const QFileInfo& fileInfo ) const
    {
        QString extension = "." + fileInfo.suffix();
        return extension;
    }

    enum Format extractFormat( const QFileInfo& fileInfo ) const
    {
        QString extension = "." + fileInfo.suffix().toLower(); // 拡張子に '.' を付加

        QStringList validExtensions = {
            ".pfi", ".pfl", ".stl", ".vtp", ".xyz", ".vtr",
            ".vtk", ".vti", ".vts", ".pvts", ".inp", ".vtu",
            ".pvtu", ".vtm", ".case"
        };

        if( extension == ".kvsml" )
        {
            QFile file( fileInfo.absoluteFilePath() );

            if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            {
                qWarning() << "Failed to open file:" << fileInfo.absoluteFilePath();
                return Format::Unknown;
            }

            QXmlStreamReader xml( &file );

            // 終端でなく、エラーがない場合ループ
            while( !xml.atEnd() && !xml.hasError() )
            {
                QXmlStreamReader::TokenType token = xml.readNext();
                if( token == QXmlStreamReader::StartElement )
                {
                    QString tagName = xml.name().toString();
                    if( tagName == "PointObject" )
                    {
                        return Format::PointObjectKVSML;
                    }
                    else if( tagName == "PolygonObject" )
                    {
                        return Format::PolygonObjectKVSML;
                    }
                    else if( tagName == "LineObject" )
                    {
                        return Format::LineObjectKVSML;
                    }
                }
            }

            // XMLパースエラー
            if( xml.hasError() )
            {
                return Format::Unknown;
            }
        }
        else if( extension == ".stl" )
        {
            return Format::PolygonObjectSTL;
        }
        else if( extension == ".fbx" )
        {
            return Format::PolygonObjectFBX;
        }
        else if( extension == ".3ds" )
        {
            return Format::PolygonObject3DS;
        }
        else if( extension == ".las" )
        {
            return Format::PointObjectLAS;
        }
        else if( extension == ".pts" )
        {
            return Format::PointObjectPTS;
        }
        else if( validExtensions.contains( extension ) )
        {
            return Format::ServerPointObjectCS;
        }

        return Format::Unknown;
    }

    QString formatToString( enum Format format ) const // enum FormatからQStringに変換を行うメソッド。
    {
        switch( format )
        {
        case Unknown:
            return QStringLiteral( "Unknown" );
        case ServerPointObjectCS:
            return QStringLiteral( "ServerPointObject(CS)" );
        case ServerPointObjectIS:
            return QStringLiteral( "ServerPointObject(IS)" );
        case ServerGlyphObjectCS:
            return QStringLiteral( "ServerGlyphObject(CS)" );
        case ServerGlyphObjectIS:
            return QStringLiteral( "ServerGlyphObject(IS)" );
        case PointObjectKVSML:
            return QStringLiteral( "PointObject(KVSML)" );
        case PointObjectLAS:
            return QStringLiteral( "las" );
        case PointObjectPTS:
            return QStringLiteral( "pts" );
        case PolygonObjectKVSML:
            return QStringLiteral( "PolygonObject(KVSML)" );
        case PolygonObjectSTL:
            return QStringLiteral( "stl" );
        case PolygonObject3DS:
            return QStringLiteral( "3ds" );
        case PolygonObjectFBX:
            return QStringLiteral( "fbx" );
        case LineObjectKVSML:
            return QStringLiteral( "LineObject(KVSML)" );
        default:
            return QStringLiteral( "Unknown" );
        }
    }

    QString extractDirectoryPath( const QFileInfo& fileInfo ) const
    {
        return fileInfo.dir().path();
    }

    QPair<int ,int> extractMinMaxTimeStep( const QFileInfo& fileInfo ) const
    {
        QString baseName = fileInfo.baseName(); // 拡張子を除いた基本部分を取得
        qInfo() << "Processing file:" << baseName << "," << __LINE__;

        int underscorePosition = baseName.indexOf( '_' ); // アンダースコアの位置を確認
        if( underscorePosition == -1 )
        {
            qWarning() << "No underscore found in filename:" << baseName;
            return QPair<int, int>( -1, -1 );
        }

        QString prefix = baseName.left( underscorePosition ); // アンダースコア前の部分
        qInfo() << "Prefix detected:" << prefix;

        // ちょうど5桁の数字のみにマッチする正規表現パターン
        QRegularExpression regularExpression( "^" + QRegularExpression::escape( prefix ) + "_(\\d{5})(?:\\..*)?$" );

        int minTimeStep = std::numeric_limits<int>::max();
        int maxTimeStep = std::numeric_limits<int>::min();
        bool foundValidFile = false; // 5桁の数値を含むファイルがあったかどうか

        // ディレクトリ内のすべてのファイルを確認
        for( const QFileInfo &entry : fileInfo.dir().entryInfoList( QDir::Files ) )
        {
            QRegularExpressionMatch match = regularExpression.match( entry.fileName() );

            if( match.hasMatch() )
            {
                int numericalValue = match.captured( 1 ).toInt();
                qInfo() << "Found valid 5-digit timestep:" << numericalValue << "in file:" << entry.fileName();

                minTimeStep = qMin( minTimeStep, numericalValue );
                maxTimeStep = qMax( maxTimeStep, numericalValue );
                foundValidFile = true;
            }
        }

        // 5桁の数値が見つかった場合のみ値を返す
        if( foundValidFile )
        {
            return QPair<int, int>( minTimeStep, maxTimeStep );
        }
        else
        {
            qWarning() << "No valid 5-digit timestep found in directory.";
            return QPair<int, int>( -1, -1 );
        }
    }

    bool isValid( QList<QStandardItem*> row ) const
    {
        QStandardItem* nameItem                 = row.at( 0 );
        QStandardItem* formatItem               = row.at( 1 );
        QStandardItem* displayCheckBoxItem      = row.at( 2 );
        QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
        QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

        if( nameItem->text().isEmpty() )
        {
            qWarning() << "ファイル名の形式が正しくありません。 例: file_XXXXX.extension";
            return false;
        }

        QPair<int,int> mitMaxTimeStep = nameItem->data( MinMaxTimeStep ).value<QPair<int,int>>();
        if( mitMaxTimeStep.first == -1 || mitMaxTimeStep.second == -1 )
        {
            qWarning() << "ファイル名にタイムステップが記載されていません。 例: file_XXXXX.extension";
            return false;
        }

        if( formatItem->data( FormatItemRole::FormatValue ).value<enum Format>() == Format::Unknown )
        {
            qWarning() << "未対応のフォーマットです。";
            return false;
        }
        return true;
    }
};

// #include <QStandardItem>
// #include <QFileInfo>
// #include <QXmlStreamReader>
// #include <QRegularExpression>
// #include <QDir>
// #include <QFileInfo>
// #include <QList>

// class ObjectItem
// {

// public:
//     enum Format // PBVRでサポートしているオブジェクト一覧
//     {
//         Unknown                       = 0,  // Aka Error
//         // ServerSidePointObject
//         ServerPointObjectCS           = 1,  // Server side Point Object
//         ServerPointObjectIS           = 2,  // Server side Point Object
//         // ServerSideGlyphObject
//         ServerGlyphObjectCS           = 3,  // Server side Point Object
//         ServerGlyphObjectIS           = 4,  // Server side Point Object
//         // LocalSideObjects
//         PointObjectKVSML              = 5,  // Point Object(.kvsml)
//         PointObjectLAS                = 6,  // Point Object(.las)
//         PointObjectPTS                = 7,  // Point Object(.pts)
//         PolygonObjectKVSML            = 8,  // Polygon Object(.kvsml)
//         PolygonObjectSTL              = 9,  // Polygon Object(.stl)
//         PolygonObject3DS              = 10, // Polygon Object(.3ds)
//         PolygonObjectFBX              = 11, // Polygon Object(.fbx)
//         LineObjectKVSML               = 12  // Line Object(.kvsml)
//     };

//     enum nameItemRole
//     {
        // Extension               = Qt::UserRole + 1,
        // DirectoryPath           = Qt::UserRole + 2,
        // MinMaxTimeStep          = Qt::UserRole + 3,
        // Ids                     = Qt::UserRole + 4,
        // TmpPolygonColor         = Qt::UserRole + 5,
        // IsPolygonColor          = Qt::UserRole + 6,
        // TmpPolygonOpacity       = Qt::UserRole + 7,
        // IsPolygonOpacity        = Qt::UserRole + 8,
        // NeedSameTimeStepReplace = Qt::UserRole + 9,
        // FocusCheckBox           = Qt::UserRole + 10,
        // IsFocus                 = Qt::UserRole + 11,
        // InitialMinObjectCoord   = Qt::UserRole + 12,
        // InitialMaxObjectCoord   = Qt::UserRole + 13,
        // InitialMinExternalCoord = Qt::UserRole + 14,
        // InitialMaxExternalCoord = Qt::UserRole + 15,
        // CurrentMinObjectCoords  = Qt::UserRole + 16,
        // CurrentMaxObjectCoords  = Qt::UserRole + 17,
        // AlreadyImportedTimeStep = Qt::UserRole + 18,
        // Object                  = Qt::UserRole + 19,
        // NumberOfVectors         = Qt::UserRole + 20, // CSオブジェクトのみ?
        // NumberOfElements        = Qt::UserRole + 21, // CSオブジェクトのみ?
        // NumberOfSubVolume       = Qt::UserRole + 22, // CSオブジェクトのみ?
        // ElementType             = Qt::UserRole + 23, // CSオブジェクトのみ?
        // NumberOfNodes           = Qt::UserRole + 24, // CSオブジェクトのみ?
        // StepNumber              = Qt::UserRole + 25, // CSオブジェクトのみ?
        // FileType                = Qt::UserRole + 26, // CSオブジェクトのみ?
        // TmpParticleLimit        = Qt::UserRole + 27,
        // IsParticleLimit         = Qt::UserRole + 28,
        // TmpDensity              = Qt::UserRole + 29,
        // IsDensity               = Qt::UserRole + 30,
        // TmpCoordinate1          = Qt::UserRole + 31,
        // IsCoordinate1           = Qt::UserRole + 32,
        // TmpCoordinate2          = Qt::UserRole + 33,
        // IsCoordinate2           = Qt::UserRole + 34,
        // TmpCoordinate3          = Qt::UserRole + 35,
        // IsCoordinate3           = Qt::UserRole + 36,
        // IsExport                = Qt::UserRole + 37,
        // ExportFilePath          = Qt::UserRole + 38,
//     };

//     enum FormatItemRole
//     {
//         FormatValue = Qt::UserRole + 1
//     };

//     enum DisplayCheckBoxItemRole
//     {
//         Isdisplay       = Qt::UserRole + 1
//     };

//     enum KeepInitialCheckBoxItemRole
//     {
//         IskeepInitial   = Qt::UserRole + 1
//     };

//     enum KeepFinalCheckBoxItemRole
//     {
//         IskeepFinal     = Qt::UserRole + 1
//     };

//     // コンストラクタ
//     ObjectItem(){}

//     // QFileInfoを引数として受け取り、QList<QStandardItem*>を返す
//     QList<QStandardItem*> createItemsFromFileInfo( const QFileInfo& fileInfo ) const
//     {
//         QList<QStandardItem*> row;

//         // ファイル名アイテム
//         QStandardItem* nameItem = new QStandardItem( extractBaseName( fileInfo ) );
//         nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
//         QVariant extension                  = extractExtension( fileInfo );
//         QVariant directoryPath              = extractDirectoryPath( fileInfo );
//         QVariant minMaxTimeStep             = QVariant::fromValue( extractMinMaxTimeStep( fileInfo ) );
//         QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
//         QVariant tmpPolygonColor            = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant isPolygonColor             = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant tmpPolygonOpacity          = QVariant::fromValue( 0.5 );
//         QVariant isPolygonOpacity           = QVariant::fromValue( 0.5 );
//         QVariant needSameTimeStepReplace    = QVariant::fromValue( false );
//         QVariant focusCheckBox              = QVariant::fromValue( false );
//         QVariant isFocus                    = QVariant::fromValue( false );
//         QVariant initialMinObjectCoord;
//         QVariant initialMaxObjectCoord;
//         QVariant initialMinExternalCoord;
//         QVariant initialMaxExternalCoord;
//         QVariant currentMinObjectCoords;
//         QVariant currentMaxObjectCoords;
//         QVariant alreadyImportedTimeStep    = QVariant::fromValue( -1 );
//         QVariant object;
//         nameItem->setData( extension,               Extension );
//         nameItem->setData( directoryPath,           DirectoryPath );
//         nameItem->setData( minMaxTimeStep,          MinMaxTimeStep );
//         nameItem->setData( ids,                     Ids );
//         nameItem->setData( tmpPolygonColor,         TmpPolygonColor );
//         nameItem->setData( isPolygonColor,          IsPolygonColor );
//         nameItem->setData( tmpPolygonOpacity,       TmpPolygonOpacity );
//         nameItem->setData( isPolygonOpacity,        IsPolygonOpacity );
//         nameItem->setData( needSameTimeStepReplace, NeedSameTimeStepReplace );
//         nameItem->setData( focusCheckBox,           FocusCheckBox );
//         nameItem->setData( isFocus,                 IsFocus );
//         nameItem->setData( initialMinObjectCoord,   InitialMinObjectCoord );
//         nameItem->setData( initialMaxObjectCoord,   InitialMaxObjectCoord );
//         nameItem->setData( initialMinExternalCoord, InitialMinExternalCoord );
//         nameItem->setData( initialMaxExternalCoord, InitialMaxExternalCoord );
//         nameItem->setData( currentMinObjectCoords, CurrentMinObjectCoords );
//         nameItem->setData( currentMaxObjectCoords, CurrentMaxObjectCoords );
//         nameItem->setData( alreadyImportedTimeStep, AlreadyImportedTimeStep );
//         nameItem->setData( object,                  Object );

//         QVariant formatValue = extractFormat( fileInfo );
//         QStandardItem* formatItem = new QStandardItem();
//         formatItem->setData( formatValue, FormatValue );
//         formatItem->setText( formatToString( formatValue.value<Format>() ) );       // 表示用

//         QStandardItem* displayCheckBoxItem = new QStandardItem();
//         displayCheckBoxItem->setCheckable( true );
//         displayCheckBoxItem->setCheckState( Qt::Checked );
//         displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
//         displayCheckBoxItem->setData( isdisplay, Isdisplay );

//         QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
//         keepInitialCheckBoxItem->setCheckable( true );
//         keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

//         QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
//         keepFinalCheckBoxItem->setCheckable( true );
//         keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

//         row.append( nameItem );
//         row.append( formatItem );
//         row.append( displayCheckBoxItem );
//         row.append( keepInitialCheckBoxItem );
//         row.append( keepFinalCheckBoxItem );

//         if( isValid( row ) )
//         {
//             return row;  // QList<QStandardItem*> を返す
//         }
//         else
//         {
//             return isValid( row ) ? row : QList<QStandardItem*>();
//         }
//     }

//     QList<QStandardItem*> createItemsFromServerPointObject( QString volumeDataFilePATH, int minTimeStep, int maxTimeStep )
//     {
//         QFileInfo fileInfo( volumeDataFilePATH );
//         QString baseName = fileInfo.completeBaseName(); // 拡張子なしファイル名
//         QString fileExtension = fileInfo.suffix();          // 拡張子（例: "dat"）

//         QList<QStandardItem*> row;

//         // ファイル名アイテム
//         QStandardItem* nameItem = new QStandardItem( baseName );
//         nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
//         QVariant extension                  = fileExtension;
//         QVariant directoryPath              = volumeDataFilePATH;
//         QVariant minMaxTimeStep             = QVariant::fromValue( QPair<int,int>( minTimeStep, maxTimeStep ) );
//         QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
//         QVariant tmpPolygonColor            = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant isPolygonColor             = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant tmpPolygonOpacity          = QVariant::fromValue( 0.5 );
//         QVariant isPolygonOpacity           = QVariant::fromValue( 0.5 );
//         QVariant needSameTimeStepReplace    = QVariant::fromValue( false );
//         QVariant focusCheckBox              = QVariant::fromValue( false );
//         QVariant isFocus                    = QVariant::fromValue( false );
//         QVariant initialMinObjectCoord;
//         QVariant initialMaxObjectCoord;
//         QVariant initialMinExternalCoord;
//         QVariant initialMaxExternalCoord;
//         QVariant currentMinObjectCoords;
//         QVariant currentMaxObjectCoords;
//         QVariant alreadyImportedTimeStep    = QVariant::fromValue( -1 );
//         QVariant object;
//         QVariant tmpParticleLimit              = QVariant::fromValue( 10000000 );
//         QVariant isParticleLimit              = QVariant::fromValue( 10000000 );
//         QVariant tmpDensity                    = QVariant::fromValue( 1 );
//         QVariant isDensity                    = QVariant::fromValue( 1 );
//         QVariant tmpCoordinate1 = QVariant::fromValue(QString(""));  // OK（QStringは対応型）
//         QVariant isCoordinate1 = QVariant::fromValue(QString(""));  // OK（QStringは対応型）
//         QVariant tmpCoordinate2 = QVariant::fromValue(QString(""));  // OK（QStringは対応型）
//         QVariant isCoordinate2 = QVariant::fromValue(QString(""));  // OK（QStringは対応型）
//         QVariant tmpCoordinate3 = QVariant::fromValue(QString(""));  // OK（QStringは対応型）
//         QVariant isCoordinate3 = QVariant::fromValue(QString(""));  // OK（QStringは対応型）
//         QVariant isExport                    = QVariant::fromValue( false );
//         QVariant exportFilePath = QVariant::fromValue(QString(""));  // OK（QStringは対応型）


//         nameItem->setData( extension,               Extension );
//         nameItem->setData( directoryPath,           DirectoryPath );
//         nameItem->setData( minMaxTimeStep,          MinMaxTimeStep );
//         nameItem->setData( ids,                     Ids );
//         nameItem->setData( tmpPolygonColor,         TmpPolygonColor );
//         nameItem->setData( isPolygonColor,          IsPolygonColor );
//         nameItem->setData( tmpPolygonOpacity,       TmpPolygonOpacity );
//         nameItem->setData( isPolygonOpacity,        IsPolygonOpacity );
//         nameItem->setData( needSameTimeStepReplace, NeedSameTimeStepReplace );
//         nameItem->setData( focusCheckBox,           FocusCheckBox );
//         nameItem->setData( isFocus,                 IsFocus );
//         nameItem->setData( initialMinObjectCoord,   InitialMinObjectCoord );
//         nameItem->setData( initialMaxObjectCoord,   InitialMaxObjectCoord );
//         nameItem->setData( initialMinExternalCoord, InitialMinExternalCoord );
//         nameItem->setData( initialMaxExternalCoord, InitialMaxExternalCoord );
//         nameItem->setData( currentMinObjectCoords, CurrentMinObjectCoords );
//         nameItem->setData( currentMaxObjectCoords, CurrentMaxObjectCoords );
//         nameItem->setData( alreadyImportedTimeStep, AlreadyImportedTimeStep );
//         nameItem->setData( object,                  Object );
//         nameItem->setData( tmpParticleLimit       , TmpParticleLimit);
//         nameItem->setData( isParticleLimit        , IsParticleLimit);
//         nameItem->setData( tmpDensity             , TmpDensity );
//         nameItem->setData( isDensity              , IsDensity );
//         nameItem->setData( tmpCoordinate1         , TmpCoordinate1 );
//         nameItem->setData( isCoordinate1         , IsCoordinate1 );
//         nameItem->setData( tmpCoordinate2         , TmpCoordinate2 );
//         nameItem->setData( isCoordinate2         , IsCoordinate2 );
//         nameItem->setData( tmpCoordinate2         , TmpCoordinate3 );
//         nameItem->setData( isCoordinate2         , IsCoordinate3 );
//         nameItem->setData( isExport         , IsExport );
//         nameItem->setData( exportFilePath         , ExportFilePath );

//         QVariant formatValue = extractFormat( fileInfo );
//         QStandardItem* formatItem = new QStandardItem();
//         formatItem->setData( formatValue, FormatValue );
//         formatItem->setText( formatToString( formatValue.value<Format>() ) );       // 表示用

//         QStandardItem* displayCheckBoxItem = new QStandardItem();
//         displayCheckBoxItem->setCheckable( true );
//         displayCheckBoxItem->setCheckState( Qt::Checked );
//         displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
//         displayCheckBoxItem->setData( isdisplay, Isdisplay );

//         QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
//         keepInitialCheckBoxItem->setCheckable( true );
//         keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

//         QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
//         keepFinalCheckBoxItem->setCheckable( true );
//         keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

//         row.append( nameItem );
//         row.append( formatItem );
//         row.append( displayCheckBoxItem );
//         row.append( keepInitialCheckBoxItem );
//         row.append( keepFinalCheckBoxItem );

//         if( isValid( row ) )
//         {
//             return row;  // QList<QStandardItem*> を返す
//         }
//         else
//         {
//             return isValid( row ) ? row : QList<QStandardItem*>();
//         }
//     }

//     QList<QStandardItem*> createItemsFromServerGlyphObjectCS( QString volumeDataFilePATH, int minTimeStep, int maxTimeStep )
//     {
//         QFileInfo fileInfo( volumeDataFilePATH );
//         QString baseName = fileInfo.completeBaseName(); // 拡張子なしファイル名
//         QString fileExtension = fileInfo.suffix();          // 拡張子（例: "dat"）

//         QList<QStandardItem*> row;

//         // ファイル名アイテム
//         QStandardItem* nameItem = new QStandardItem( baseName );
//         nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
//         QVariant extension                  = fileExtension;
//         QVariant directoryPath              = volumeDataFilePATH;
//         QVariant minMaxTimeStep             = QVariant::fromValue( QPair<int,int>( minTimeStep, maxTimeStep ) );
//         QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
//         QVariant tmpPolygonColor            = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant isPolygonColor             = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant tmpPolygonOpacity          = QVariant::fromValue( 0.5 );
//         QVariant isPolygonOpacity           = QVariant::fromValue( 0.5 );
//         QVariant needSameTimeStepReplace    = QVariant::fromValue( false );
//         QVariant focusCheckBox              = QVariant::fromValue( false );
//         QVariant isFocus                    = QVariant::fromValue( false );
//         QVariant initialMinObjectCoord;
//         QVariant initialMaxObjectCoord;
//         QVariant initialMinExternalCoord;
//         QVariant initialMaxExternalCoord;
//         QVariant currentMinObjectCoords;
//         QVariant currentMaxObjectCoords;
//         QVariant alreadyImportedTimeStep    = QVariant::fromValue( -1 );
//         QVariant object;
//         nameItem->setData( extension,               Extension );
//         nameItem->setData( directoryPath,           DirectoryPath );
//         nameItem->setData( minMaxTimeStep,          MinMaxTimeStep );
//         nameItem->setData( ids,                     Ids );
//         nameItem->setData( tmpPolygonColor,         TmpPolygonColor );
//         nameItem->setData( isPolygonColor,          IsPolygonColor );
//         nameItem->setData( tmpPolygonOpacity,       TmpPolygonOpacity );
//         nameItem->setData( isPolygonOpacity,        IsPolygonOpacity );
//         nameItem->setData( needSameTimeStepReplace, NeedSameTimeStepReplace );
//         nameItem->setData( focusCheckBox,           FocusCheckBox );
//         nameItem->setData( isFocus,                 IsFocus );
//         nameItem->setData( initialMinObjectCoord,   InitialMinObjectCoord );
//         nameItem->setData( initialMaxObjectCoord,   InitialMaxObjectCoord );
//         nameItem->setData( initialMinExternalCoord, InitialMinExternalCoord );
//         nameItem->setData( initialMaxExternalCoord, InitialMaxExternalCoord );
//         nameItem->setData( currentMinObjectCoords, CurrentMinObjectCoords );
//         nameItem->setData( currentMaxObjectCoords, CurrentMaxObjectCoords );
//         nameItem->setData( alreadyImportedTimeStep, AlreadyImportedTimeStep );
//         nameItem->setData( object,                  Object );

//         QVariant formatValue = Format::ServerGlyphObjectCS;
//         QStandardItem* formatItem = new QStandardItem();
//         formatItem->setData( formatValue, FormatValue );
//         formatItem->setText( formatToString( formatValue.value<Format>() ) );       // 表示用

//         QStandardItem* displayCheckBoxItem = new QStandardItem();
//         displayCheckBoxItem->setCheckable( true );
//         displayCheckBoxItem->setCheckState( Qt::Checked );
//         displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
//         displayCheckBoxItem->setData( isdisplay, Isdisplay );

//         QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
//         keepInitialCheckBoxItem->setCheckable( true );
//         keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

//         QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
//         keepFinalCheckBoxItem->setCheckable( true );
//         keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

//         row.append( nameItem );
//         row.append( formatItem );
//         row.append( displayCheckBoxItem );
//         row.append( keepInitialCheckBoxItem );
//         row.append( keepFinalCheckBoxItem );

//         if( isValid( row ) )
//         {
//             return row;  // QList<QStandardItem*> を返す
//         }
//         else
//         {
//             return isValid( row ) ? row : QList<QStandardItem*>();
//         }
//     }

//     QList<QStandardItem*> createItemsFromServerPointObjectIS( QString volumeDataFilePATH, int minTimeStep, int maxTimeStep )
//     {
//         QFileInfo fileInfo( volumeDataFilePATH );
//         QString baseName = fileInfo.completeBaseName(); // 拡張子なしファイル名
//         QString fileExtension = fileInfo.suffix();          // 拡張子（例: "dat"）

//         QList<QStandardItem*> row;

//         // ファイル名アイテム
//         QStandardItem* nameItem = new QStandardItem( baseName );
//         nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
//         QVariant extension                  = fileExtension;
//         QVariant directoryPath              = volumeDataFilePATH;
//         QVariant minMaxTimeStep             = QVariant::fromValue( QPair<int,int>( minTimeStep, maxTimeStep ) );
//         QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
//         QVariant tmpPolygonColor            = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant isPolygonColor             = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant tmpPolygonOpacity          = QVariant::fromValue( 0.5 );
//         QVariant isPolygonOpacity           = QVariant::fromValue( 0.5 );
//         QVariant needSameTimeStepReplace    = QVariant::fromValue( false );
//         QVariant focusCheckBox              = QVariant::fromValue( false );
//         QVariant isFocus                    = QVariant::fromValue( false );
//         QVariant initialMinObjectCoord;
//         QVariant initialMaxObjectCoord;
//         QVariant initialMinExternalCoord;
//         QVariant initialMaxExternalCoord;
//         QVariant currentMinObjectCoords;
//         QVariant currentMaxObjectCoords;
//         QVariant alreadyImportedTimeStep    = QVariant::fromValue( -1 );
//         QVariant object;
//         QVariant tmpParticleLimit              = QVariant::fromValue( 10000000 );
//         QVariant isParticleLimit              = QVariant::fromValue( 10000000 );
//         QVariant tmpDensity                    = QVariant::fromValue( 1 );
//         QVariant isDensity                    = QVariant::fromValue( 1 );
//         nameItem->setData( extension,               Extension );
//         nameItem->setData( directoryPath,           DirectoryPath );
//         nameItem->setData( minMaxTimeStep,          MinMaxTimeStep );
//         nameItem->setData( ids,                     Ids );
//         nameItem->setData( tmpPolygonColor,         TmpPolygonColor );
//         nameItem->setData( isPolygonColor,          IsPolygonColor );
//         nameItem->setData( tmpPolygonOpacity,       TmpPolygonOpacity );
//         nameItem->setData( isPolygonOpacity,        IsPolygonOpacity );
//         nameItem->setData( needSameTimeStepReplace, NeedSameTimeStepReplace );
//         nameItem->setData( focusCheckBox,           FocusCheckBox );
//         nameItem->setData( isFocus,                 IsFocus );
//         nameItem->setData( initialMinObjectCoord,   InitialMinObjectCoord );
//         nameItem->setData( initialMaxObjectCoord,   InitialMaxObjectCoord );
//         nameItem->setData( initialMinExternalCoord, InitialMinExternalCoord );
//         nameItem->setData( initialMaxExternalCoord, InitialMaxExternalCoord );
//         nameItem->setData( currentMinObjectCoords, CurrentMinObjectCoords );
//         nameItem->setData( currentMaxObjectCoords, CurrentMaxObjectCoords );
//         nameItem->setData( alreadyImportedTimeStep, AlreadyImportedTimeStep );
//         nameItem->setData( object,                  Object );
//         nameItem->setData( tmpParticleLimit       , TmpParticleLimit);
//         nameItem->setData( isParticleLimit        , IsParticleLimit);
//         nameItem->setData( tmpDensity             , TmpDensity );
//         nameItem->setData( isDensity              , IsDensity );

//         QVariant formatValue =  Format::ServerPointObjectIS;
//         QStandardItem* formatItem = new QStandardItem();
//         formatItem->setData( formatValue, FormatValue );
//         formatItem->setText( formatToString( formatValue.value<Format>() ) );       // 表示用

//         QStandardItem* displayCheckBoxItem = new QStandardItem();
//         displayCheckBoxItem->setCheckable( true );
//         displayCheckBoxItem->setCheckState( Qt::Checked );
//         displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
//         displayCheckBoxItem->setData( isdisplay, Isdisplay );

//         QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
//         keepInitialCheckBoxItem->setCheckable( true );
//         keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

//         QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
//         keepFinalCheckBoxItem->setCheckable( true );
//         keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

//         row.append( nameItem );
//         row.append( formatItem );
//         row.append( displayCheckBoxItem );
//         row.append( keepInitialCheckBoxItem );
//         row.append( keepFinalCheckBoxItem );

//         if( isValid( row ) )
//         {
//             return row;  // QList<QStandardItem*> を返す
//         }
//         else
//         {
//             return isValid( row ) ? row : QList<QStandardItem*>();
//         }
//     }

//     QList<QStandardItem*> createItemsFromServerGlyphObjectIS( QString volumeDataFilePATH, int minTimeStep, int maxTimeStep )
//     {
//         QFileInfo fileInfo( volumeDataFilePATH );
//         QString baseName = fileInfo.completeBaseName(); // 拡張子なしファイル名
//         QString fileExtension = fileInfo.suffix();          // 拡張子（例: "dat"）

//         QList<QStandardItem*> row;

//         // ファイル名アイテム
//         QStandardItem* nameItem = new QStandardItem( baseName );
//         nameItem->setFlags( nameItem->flags() & ~Qt::ItemIsEditable );  // 編集不可
//         QVariant extension                  = fileExtension;
//         QVariant directoryPath              = volumeDataFilePATH;
//         QVariant minMaxTimeStep             = QVariant::fromValue( QPair<int,int>( minTimeStep, maxTimeStep ) );
//         QVariant ids                        = QVariant::fromValue( QPair<int, int>( -1, -1 ) );
//         QVariant tmpPolygonColor            = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant isPolygonColor             = QVariant::fromValue( QColor( 128, 128, 128 ) );
//         QVariant tmpPolygonOpacity          = QVariant::fromValue( 0.5 );
//         QVariant isPolygonOpacity           = QVariant::fromValue( 0.5 );
//         QVariant needSameTimeStepReplace    = QVariant::fromValue( false );
//         QVariant focusCheckBox              = QVariant::fromValue( false );
//         QVariant isFocus                    = QVariant::fromValue( false );
//         QVariant initialMinObjectCoord;
//         QVariant initialMaxObjectCoord;
//         QVariant initialMinExternalCoord;
//         QVariant initialMaxExternalCoord;
//         QVariant currentMinObjectCoords;
//         QVariant currentMaxObjectCoords;
//         QVariant alreadyImportedTimeStep    = QVariant::fromValue( -1 );
//         QVariant object;
//         nameItem->setData( extension,               Extension );
//         nameItem->setData( directoryPath,           DirectoryPath );
//         nameItem->setData( minMaxTimeStep,          MinMaxTimeStep );
//         nameItem->setData( ids,                     Ids );
//         nameItem->setData( tmpPolygonColor,         TmpPolygonColor );
//         nameItem->setData( isPolygonColor,          IsPolygonColor );
//         nameItem->setData( tmpPolygonOpacity,       TmpPolygonOpacity );
//         nameItem->setData( isPolygonOpacity,        IsPolygonOpacity );
//         nameItem->setData( needSameTimeStepReplace, NeedSameTimeStepReplace );
//         nameItem->setData( focusCheckBox,           FocusCheckBox );
//         nameItem->setData( isFocus,                 IsFocus );
//         nameItem->setData( initialMinObjectCoord,   InitialMinObjectCoord );
//         nameItem->setData( initialMaxObjectCoord,   InitialMaxObjectCoord );
//         nameItem->setData( initialMinExternalCoord, InitialMinExternalCoord );
//         nameItem->setData( initialMaxExternalCoord, InitialMaxExternalCoord );
//         nameItem->setData( currentMinObjectCoords, CurrentMinObjectCoords );
//         nameItem->setData( currentMaxObjectCoords, CurrentMaxObjectCoords );
//         nameItem->setData( alreadyImportedTimeStep, AlreadyImportedTimeStep );
//         nameItem->setData( object,                  Object );

//         QVariant formatValue = Format::ServerGlyphObjectIS;
//         QStandardItem* formatItem = new QStandardItem();
//         formatItem->setData( formatValue, FormatValue );
//         formatItem->setText( formatToString( formatValue.value<Format>() ) );       // 表示用

//         QStandardItem* displayCheckBoxItem = new QStandardItem();
//         displayCheckBoxItem->setCheckable( true );
//         displayCheckBoxItem->setCheckState( Qt::Checked );
//         displayCheckBoxItem->setFlags( displayCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant isdisplay = QVariant::fromValue( Qt::CheckState::Unchecked );
//         displayCheckBoxItem->setData( isdisplay, Isdisplay );

//         QStandardItem* keepInitialCheckBoxItem = new QStandardItem();
//         keepInitialCheckBoxItem->setCheckable( true );
//         keepInitialCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepInitialCheckBoxItem->setFlags( keepInitialCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepInitial = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepInitialCheckBoxItem->setData( iskeepInitial, IskeepInitial );

//         QStandardItem* keepFinalCheckBoxItem = new QStandardItem();
//         keepFinalCheckBoxItem->setCheckable( true );
//         keepFinalCheckBoxItem->setCheckState( Qt::Unchecked );
//         keepFinalCheckBoxItem->setFlags( keepFinalCheckBoxItem->flags() & ~Qt::ItemIsEditable );
//         QVariant iskeepFinal = QVariant::fromValue( Qt::CheckState::Unchecked );
//         keepFinalCheckBoxItem->setData( iskeepFinal, IskeepFinal );

//         row.append( nameItem );
//         row.append( formatItem );
//         row.append( displayCheckBoxItem );
//         row.append( keepInitialCheckBoxItem );
//         row.append( keepFinalCheckBoxItem );

//         if( isValid( row ) )
//         {
//             return row;  // QList<QStandardItem*> を返す
//         }
//         else
//         {
//             return isValid( row ) ? row : QList<QStandardItem*>();
//         }
//     }

// private:
//     QString extractBaseName( const QFileInfo& fileInfo ) const
//     {
//         QString baseName = fileInfo.baseName();
//         int underScorePosition = baseName.indexOf( '_' );
//         if( underScorePosition != -1 )
//         {
//             return baseName.left( underScorePosition );
//         }
//         return "";
//     }

//     QString extractExtension( const QFileInfo& fileInfo ) const
//     {
//         QString extension = "." + fileInfo.suffix();
//         return extension;
//     }

//     QString extractDirectoryPath( const QFileInfo& fileInfo ) const
//     {
//         return fileInfo.dir().path();
//     }

//     QPair<int ,int> extractMinMaxTimeStep( const QFileInfo& fileInfo ) const
//     {
//         QString baseName = fileInfo.baseName(); // 拡張子を除いた基本部分を取得
//         qInfo() << "Processing file:" << baseName << "," << __LINE__;

//         int underscorePosition = baseName.indexOf('_'); // アンダースコアの位置を確認
//         if (underscorePosition == -1) {
//             qWarning() << "No underscore found in filename:" << baseName;
//             return QPair<int, int>(-1, -1);
//         }

//         QString prefix = baseName.left(underscorePosition); // アンダースコア前の部分
//         qInfo() << "Prefix detected:" << prefix;

//         // ちょうど5桁の数字のみにマッチする正規表現パターン
//         QRegularExpression regularExpression("^" + QRegularExpression::escape(prefix) + "_(\\d{5})(?:\\..*)?$");

//         int minTimeStep = std::numeric_limits<int>::max();
//         int maxTimeStep = std::numeric_limits<int>::min();
//         bool foundValidFile = false; // 5桁の数値を含むファイルがあったかどうか

//         // ディレクトリ内のすべてのファイルを確認
//         for (const QFileInfo &entry : fileInfo.dir().entryInfoList(QDir::Files)) {
//             QRegularExpressionMatch match = regularExpression.match(entry.fileName());

//             if (match.hasMatch()) {
//                 int numericalValue = match.captured(1).toInt();
//                 qInfo() << "Found valid 5-digit timestep:" << numericalValue << "in file:" << entry.fileName();

//                 minTimeStep = qMin(minTimeStep, numericalValue);
//                 maxTimeStep = qMax(maxTimeStep, numericalValue);
//                 foundValidFile = true;
//             }
//         }

//         // 5桁の数値が見つかった場合のみ値を返す
//         if (foundValidFile) {
//             return QPair<int, int>(minTimeStep, maxTimeStep);
//         } else {
//             qWarning() << "No valid 5-digit timestep found in directory.";
//             return QPair<int, int>(-1, -1);
//         }
//     }

//     Format extractFormat( const QFileInfo& fileInfo ) const
//     {
//         QString extension = "." + fileInfo.suffix().toLower(); // 拡張子に '.' を付加

//         QStringList validExtensions = {
//             ".pfi", ".pfl", ".stl", ".vtp", ".xyz", ".vtr",
//             ".vtk", ".vti", ".vts", ".pvts", ".inp", ".vtu",
//             ".pvtu", ".vtm", ".case"
//         };

//         if( extension == ".kvsml" )
//         {
//             QFile file( fileInfo.absoluteFilePath() );

//             if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
//             {
//                 qWarning() << "Failed to open file:" << fileInfo.absoluteFilePath();
//                 return Format::Unknown;
//             }

//             QXmlStreamReader xml( &file );

//             // 終端でなく、エラーがない場合ループ
//             while( !xml.atEnd() && !xml.hasError() )
//             {
//                 QXmlStreamReader::TokenType token = xml.readNext();
//                 if( token == QXmlStreamReader::StartElement )
//                 {
//                     QString tagName = xml.name().toString();
//                     if( tagName == "PointObject" )
//                     {
//                         return Format::PointObjectKVSML;
//                     }
//                     else if( tagName == "PolygonObject" )
//                     {
//                         return Format::PolygonObjectKVSML;
//                     }
//                     else if( tagName == "LineObject" )
//                     {
//                         return Format::LineObjectKVSML;
//                     }
//                 }
//             }

//             // XMLパースエラー
//             if( xml.hasError() )
//             {
//                 return Format::Unknown;
//             }
//         }
//         else if( extension == ".stl" )
//         {
//             return Format::PolygonObjectSTL;
//         }
//         else if( extension == ".fbx" )
//         {
//             return Format::PolygonObjectFBX;
//         }
//         else if( extension == ".3ds" )
//         {
//             return Format::PolygonObject3DS;
//         }
//         else if( extension == ".las" )
//         {
//             return Format::PointObjectLAS;
//         }
//         else if( extension == ".pts" )
//         {
//             return Format::PointObjectPTS;
//         }
//         else if( validExtensions.contains( extension ) )
//         {
//             return Format::ServerPointObjectCS;
//         }

//         return Format::Unknown;
//     }

//     QString formatToString( Format format ) const // enum FormatからQStringに変換を行うメソッド。
//     {
//         switch ( format )
//         {
//         case Unknown:
//             return QStringLiteral( "Unknown" );
//         case ServerPointObjectCS:
//             return QStringLiteral( "ServerPointObject(CS)" );
//         case ServerPointObjectIS:
//             return QStringLiteral( "ServerPointObject(IS)" );
//         case ServerGlyphObjectCS:
//             return QStringLiteral( "ServerGlyphObject(CS)" );
//         case ServerGlyphObjectIS:
//             return QStringLiteral( "ServerGlyphObject(IS)" );
//         case PointObjectKVSML:
//             return QStringLiteral( "PointObject(KVSML)" );
//         case PointObjectLAS:
//             return QStringLiteral( "las" );
//         case PointObjectPTS:
//             return QStringLiteral( "pts" );
//         case PolygonObjectKVSML:
//             return QStringLiteral( "PolygonObject(KVSML)" );
//         case PolygonObjectSTL:
//             return QStringLiteral( "stl" );
//         case PolygonObject3DS:
//             return QStringLiteral( "3ds" );
//         case PolygonObjectFBX:
//             return QStringLiteral( "fbx" );
//         case LineObjectKVSML:
//             return QStringLiteral( "LineObject(KVSML)" );
//         default:
//             return QStringLiteral( "Unknown" );
//         }
//     }

//     bool isValid( QList<QStandardItem*> row ) const
//     {
//         QStandardItem* nameItem                 = row.at( 0 );
//         QStandardItem* formatItem               = row.at( 1 );
//         QStandardItem* displayCheckBoxItem      = row.at( 2 );
//         QStandardItem* keepInitialCheckBoxItem  = row.at( 3 );
//         QStandardItem* keepFinalCheckBoxItem    = row.at( 4 );

//         if( nameItem->text().isEmpty() )
//         {
//             qWarning() << "ファイル名の形式が正しくありません。 例: file_XXXXX.extension";
//             return false;
//         }

//         QPair<int,int> mitMaxTimeStep = nameItem->data( MinMaxTimeStep ).value<QPair<int,int>>();
//         if( mitMaxTimeStep.first == -1 || mitMaxTimeStep.second == -1 )
//         {
//             qWarning() << "ファイル名にタイムステップが記載されていません。 例: file_XXXXX.extension";
//             return false;
//         }

//         if( formatItem->data( Extension ).value<Format>() == Format::Unknown )
//         {
//             qWarning() << "未対応のフォーマットです。";
//             return false;
//         }
//     }
// };

#endif // OBJECTITEM_H

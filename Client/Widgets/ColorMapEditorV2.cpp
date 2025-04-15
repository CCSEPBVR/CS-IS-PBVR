#include "ColorMapEditorV2.h"
#include "ui_ColorMapEditorV2.h"

ColorMapEditorV2::ColorMapEditorV2( QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::ColorMapEditorV2 )
    , m_default_colors( QVector<QColor>{ QColor( 255, 0, 0 ), QColor( 0, 255, 0 ), QColor( 0, 0, 255 ) } )
{
    ui->setupUi( this );
    initialize();
}

ColorMapEditorV2::~ColorMapEditorV2()
{
    delete ui;
}

void ColorMapEditorV2::setDefaultColorMap( const QVector<QColor>& colors )
{
    m_default_colors = colors;
    ui->colorMapPalette->setColors( colors );
}

QVector<QColor> ColorMapEditorV2::getColorMap()
{
    return ui->colorMapPalette->getColors();
}

void ColorMapEditorV2::initialize()
{
    ui->tabWidget->setCurrentIndex( 0 );
    resize( 800, 500 );

    initializePreset();
    initializeFreeformCurve();
    initializeExpression();
    initializeControlPoints();

    connect( ui->resetPushButton, &QPushButton::clicked, this, &ColorMapEditorV2::onReset );
    connect( ui->undoPushButton, &QPushButton::clicked, ui->colorMapPalette, &ColorMapPalette::undo );
    connect( ui->redoPushButton, &QPushButton::clicked,ui->colorMapPalette, &ColorMapPalette::redo );

    connect( ui->tabWidget, &QTabWidget::currentChanged, this, &ColorMapEditorV2::onTabChanged );
    connect( ui->colorMapTableWidget, &QTableWidget::cellDoubleClicked, this, &ColorMapEditorV2::onPresetColorMapDoubleClicked );
    connect( ui->drawingColorClickableLabel, &ClickableLabel::doubleClicked, this, &ColorMapEditorV2::onDrawingColorDoubleClicked );

    connect( ui->redLineEdit, &QLineEdit::textEdited, this, &ColorMapEditorV2::onExpressionChanged );
    connect( ui->greenLineEdit, &QLineEdit::textEdited, this, &ColorMapEditorV2::onExpressionChanged );
    connect( ui->blueLineEdit, &QLineEdit::textEdited, this, &ColorMapEditorV2::onExpressionChanged );

    connect( ui->numberOfControlPointsSpinBox, &QSpinBox::valueChanged, this, &ColorMapEditorV2::onNumberOfControlPointsChanged );
    connect( ui->controlPointsTableWidget, &QTableWidget::cellChanged, this, &ColorMapEditorV2::onControlPointChanged );

    connect( ui->cancelPushButton, &QPushButton::clicked, this, &ColorMapEditorV2::close );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &ColorMapEditorV2::accept );
}

void ColorMapEditorV2::initializePreset()
{
    ui->colorMapTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );
    ui->colorMapTableWidget->setSelectionBehavior( QAbstractItemView::SelectItems );

    ui->colorMapTableWidget->setColumnCount( 2 );
    ui->colorMapTableWidget->horizontalHeader()->setVisible( false ); // 横（列）のヘッダーを非表示
    ui->colorMapTableWidget->verticalHeader()->setVisible( false );   // 縦（行）のヘッダーを非表示
    ui->colorMapTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );

    QList<QPair<QString, QVector<QColor>>> colorList = loadDefaultColorMap( ":DefaultColorMap.json" );
    if( colorList.isEmpty() )
    {
        qWarning() << "No color maps loaded!";
        return;
    }

    // テーブルの行数を設定
    int rows = ( colorList.size() + 1 ) / 2;
    ui->colorMapTableWidget->setRowCount( rows );

    int minRowHeight = 70;
    int maxRowHeight = 100;
    ui->colorMapTableWidget->verticalHeader()->setMinimumSectionSize( minRowHeight );
    ui->colorMapTableWidget->verticalHeader()->setMaximumSectionSize( maxRowHeight );

    // 色マップをテーブルに設定
    for( int i = 0; i < colorList.size(); i++ )
    {
        int row = i / 2;
        int col = i % 2;

        QWidget* cellWidget = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout( cellWidget );
        layout->setAlignment( Qt::AlignCenter );  // 中央寄せ
        layout->setSpacing( 5 );  // 適宜スペース調整
        layout->setContentsMargins( 5, 5, 5, 5 ); // 必要に応じてマージン

        QLabel* nameLabel = new QLabel( colorList[i].first );
        nameLabel->setAlignment( Qt::AlignCenter );
        QFont font = nameLabel->font();
        font.setBold( true );
        nameLabel->setFont( font );

        ColorMap* ccolorMap = new ColorMap( this, colorList[i].second, 256 );
        ccolorMap->setFixedSize( 256, 20 );  // サイズ固定

        cellWidget->setMinimumHeight( minRowHeight );
        cellWidget->setMaximumHeight( maxRowHeight );

        layout->addWidget( nameLabel );
        layout->addWidget( ccolorMap, 0, Qt::AlignCenter );  // 中央配置

        ui->colorMapTableWidget->setCellWidget( row, col, cellWidget );
    }
}

void ColorMapEditorV2::initializeFreeformCurve()
{
    QPalette palette = ui->drawingColorClickableLabel->palette();
    palette.setColor( QPalette::Window, Qt::black );
    ui->drawingColorClickableLabel->setAutoFillBackground( true );
    ui->drawingColorClickableLabel->setPalette( palette );
}

void ColorMapEditorV2::initializeExpression()
{
    ui->redLineEdit->setText( "1.0" );
    ui->greenLineEdit->setText( "1.4*sin(3.14*x)" );
    ui->blueLineEdit->setText( "-3*x+1.5" );
}

void ColorMapEditorV2::initializeControlPoints()
{
    ui->controlPointsTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    ui->controlPointsTableWidget->setRowCount( ui->numberOfControlPointsSpinBox->value() );
}

QList<QPair<QString, QVector<QColor>>> ColorMapEditorV2::loadDefaultColorMap( const QString& filePath )
{
    QList<QPair<QString, QVector<QColor>>> colorList;

    QFile file( filePath );
    if( !file.open( QIODevice::ReadOnly ) )
    {
        qWarning() << "Failed to open file:" << filePath;
        return colorList;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson( jsonData );
    if( !doc.isArray() )
    {
        qWarning() << "Invalid JSON format!";
        return colorList;
    }

    QJsonArray jsonArray = doc.array();
    for( const QJsonValue& value : jsonArray )
    {
        if( !value.isObject() ) continue;
        QJsonObject obj = value.toObject();

        QString name = obj["name"].toString();
        QJsonArray colorsArray = obj["colors"].toArray();

        QVector<QColor> colors;
        for( int i = 0; i + 2 < colorsArray.size(); i += 3 )
        {
            int r = colorsArray[i].toInt();
            int g = colorsArray[i + 1].toInt();
            int b = colorsArray[i + 2].toInt();
            colors.append( QColor( r, g, b ) );
        }

        colorList.append( qMakePair( name, colors ) );
    }

    return colorList;
}

void ColorMapEditorV2::onReset()
{
    ui->colorMapPalette->setColors( m_default_colors );
}

void ColorMapEditorV2::onTabChanged( int index )
{
    /*
     * 0: Preset
     * 1: Freeform Curve
     * 2: Expression
     * 3: Control Points
     */
    if( index == 0 )
    {
        resize( width(), 500 );
    }
    else if( index == 1 )
    {
        resize( width(), minimumHeight() );
    }
    else if( index == 2 )
    {
        onExpressionChanged();
        resize( width(), minimumHeight() );
    }
    else if( index == 3 )
    {
        onControlPointChanged();
        resize( width(), 500 );
    }
}

void ColorMapEditorV2::onPresetColorMapDoubleClicked( int row, int column )
{
    // セル内のウィジェット（cellWidget）を取得
    QWidget* cellWidget = ui->colorMapTableWidget->cellWidget( row, column );
    if( !cellWidget ) return;

    // cellWidget の子ウィジェットの中から ColorMap を探す
    ColorMap* colorMap = cellWidget->findChild<ColorMap*>();
    if( colorMap )
    {
        // cm3 が取得できたら、必要な処理を書く
        // 例: カラーマップのデータを取得
        auto colors = colorMap->getColors();  // getColorMap() が必要に応じて実装されていること
        ui->colorMapPalette->setColors( colors );
    }
}

void ColorMapEditorV2::onDrawingColorDoubleClicked()
{
    QColorDialog colorDialog;
    colorDialog.adjustSize();

    if( colorDialog.exec() == QDialog::Accepted )
    {  // カラーダイアログがOKされた場合
        QColor color = colorDialog.selectedColor();

        QPalette palette = ui->drawingColorClickableLabel->palette();
        palette.setColor( QPalette::Window, color );

        ui->drawingColorClickableLabel->setPalette( palette );
        ui->colorMapPalette->setDrawingColor( color );
    }

    raise();
}

void ColorMapEditorV2::onExpressionChanged()
{
    // RGB関数の式を取得
    std::string redFunctionExpression;
    std::string greenFunctionExpression;
    std::string blueFunctionExpression;

#ifdef Q_OS_WIN
    redFunctionExpression = ui->redLineEdit->text().toLocal8Bit().constData();
    greenFunctionExpression = ui->greenLineEdit->text().toLocal8Bit().constData();
    blueFunctionExpression = ui->blueLineEdit->text().toLocal8Bit().constData();
#else
    redFunctionExpression = ui->redLineEdit->text().toStdString();
    greenFunctionExpression = ui->greenLineEdit->text().toStdString();
    blueFunctionExpression = ui->blueLineEdit->text().toStdString();
#endif

    const float min_value = 0.0;
    const float max_value = 1.0;

    // 変数と関数の準備
    FuncParser::Variables variables;
    FuncParser::Variable variable_x;
    FuncParser::Function redFunction, greenFunction, blueFunction;

    char charx1[2] = "x";
    variable_x.tag( charx1 );
    variables.push_back( variable_x );

    // 数式パーサーの初期化
    FuncParser::FunctionParser redFunctionParse(redFunctionExpression, (int)redFunctionExpression.size() + 1 );
    FuncParser::FunctionParser greenFunctionParse(greenFunctionExpression, (int)greenFunctionExpression.size() + 1 );
    FuncParser::FunctionParser blueFunctionParse(blueFunctionExpression, (int)blueFunctionExpression.size() + 1 );

    FuncParser::FunctionParser::Error errorRed = redFunctionParse.express( redFunction, variables );
    FuncParser::FunctionParser::Error errorGreen = greenFunctionParse.express( greenFunction, variables );
    FuncParser::FunctionParser::Error errorBlue = blueFunctionParse.express( blueFunction, variables );

    // 入力エラーがある場合、エディットボックスの背景を赤にする
    ui->redLineEdit->setStyleSheet( errorRed != FuncParser::FunctionParser::ERR_NONE ? "background-color: red;" : "" );
    ui->greenLineEdit->setStyleSheet( errorGreen != FuncParser::FunctionParser::ERR_NONE ? "background-color: red;" : "" );
    ui->blueLineEdit->setStyleSheet( errorBlue != FuncParser::FunctionParser::ERR_NONE ? "background-color: red;" : "" );

    // すべての関数が正常に解釈された場合、カラーマップを作成
    if( errorRed == FuncParser::FunctionParser::ERR_NONE &&
        errorGreen == FuncParser::FunctionParser::ERR_NONE &&
        errorBlue == FuncParser::FunctionParser::ERR_NONE )
    {
        QVector<QColor> colorMap;
        const float stride = ( max_value - min_value ) / ( 256 - 1 );
        float x = min_value;

        for( int i = 0; i < 256; i++, x += stride )
        {
            variable_x = x;
            float r = redFunction.eval();
            float g = greenFunction.eval();
            float b = blueFunction.eval();

            r = std::clamp( r, 0.0f, 1.0f );
            g = std::clamp( g, 0.0f, 1.0f );
            b = std::clamp( b, 0.0f, 1.0f );

            int red = static_cast<int>( r * 255 );
            int green = static_cast<int>( g * 255 );
            int blue = static_cast<int>( b * 255 );

            colorMap.append( QColor( red, green, blue ) );
        }

        // 作成したカラーマップを UI に反映
        ui->colorMapPalette->setColors( colorMap );
    }
}

void ColorMapEditorV2::onNumberOfControlPointsChanged( int value )
{
    ui->controlPointsTableWidget->setRowCount( value );
    onControlPointChanged();
}

void ColorMapEditorV2::onControlPointChanged()
{
    const int width = 256;
    QVector<QPair<float, QColor>> colorPoints;

    for( int i = 0; i < ui->controlPointsTableWidget->rowCount(); i++ )
    {
        bool validRow = true;
        float rowValues[4] = { 0.0, 0.0, 0.0, 0.0 };

        for( int c = 0; c < 4; c++ )
        {
            QTableWidgetItem* item = ui->controlPointsTableWidget->item( i, c );
            if( item )
            {
                bool validFloat;
                QString text = item->text();
                rowValues[c] = text.toFloat( &validFloat );

                if( !validFloat || rowValues[c] < 0.0 || rowValues[c] > 1.0 )
                {
                    validRow = false;
                    item->setForeground( QBrush( Qt::red ) );
                }
                else
                {
                    item->setForeground( QBrush( Qt::black ) );
                }
            }
            else
            {
                validRow = false;
            }
        }

        if( validRow )
        {
            float x = rowValues[0]; // 0.0 〜 1.0
            int r = static_cast<int>(rowValues[1] * 255.0);
            int g = static_cast<int>(rowValues[2] * 255.0);
            int b = static_cast<int>(rowValues[3] * 255.0);

            QColor color( r, g, b );
            colorPoints.append( qMakePair( x, color ) );
        }
    }

    // 色の補間を行って QVector<QColor> に変換
    if( !colorPoints.isEmpty() )
    {
        QVector<QColor> colorMap;
        colorMap.reserve( width );

        // 先にX軸でソート（念のため）
        std::sort( colorPoints.begin(), colorPoints.end(), []( const QPair<float, QColor>& a, const QPair<float, QColor>& b )
                  {
                      return a.first < b.first;
                  } );

        for( int i = 0; i < width; i++ )
        {
            float pos = static_cast<float>( i ) / ( width - 1 );

            // 補間対象の区間を探す
            QColor interpolatedColor;
            if( pos <= colorPoints.first().first )
            {
                interpolatedColor = colorPoints.first().second;
            }
            else if( pos >= colorPoints.last().first )
            {
                interpolatedColor = colorPoints.last().second;
            }
            else
            {
                for( int j = 0; j < colorPoints.size() - 1; j++ )
                {
                    float x0 = colorPoints[j].first;
                    float x1 = colorPoints[j + 1].first;

                    if( pos >= x0 && pos <= x1 )
                    {
                        QColor c0 = colorPoints[j].second;
                        QColor c1 = colorPoints[j + 1].second;
                        float t = ( pos - x0 ) / ( x1 - x0 );

                        int r = static_cast<int>( ( 1 - t ) * c0.red() + t * c1.red() );
                        int g = static_cast<int>( ( 1 - t ) * c0.green() + t * c1.green() );
                        int b = static_cast<int>( ( 1 - t ) * c0.blue() + t * c1.blue() );

                        interpolatedColor = QColor( r, g, b );
                        break;
                    }
                }
            }

            colorMap.append( interpolatedColor );
        }

        // カラーマップをUIやColorMapWidgetに反映
        ui->colorMapPalette->setColors( colorMap );
    }
}

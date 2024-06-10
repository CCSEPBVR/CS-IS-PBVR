#include "ColorMapEditor.h"
#include "TFEColorMapBar.h"
#include "ui_ColorMapEditor.h"

#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QColor>
#include <QColorDialog>

#include <kvs/DivergingColorMap>
#include "FunctionParser/function_parser.h"

ColorMapEditor::ColorMapEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorMapEditor)
{    
    ui->setupUi(this);
    m_undo_stack = new QUndoStack;
    m_undo_stack->setUndoLimit( 50 );
    ui->colorMapPalette->setUndoStack( m_undo_stack );

    ui->colorMapBarTWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->colorMapBarTWidget->verticalHeader()->setDefaultSectionSize(60);

    ui->controlPointsTWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    readJsonFile();

    ui->colorMapBarTabWidget->setCurrentIndex( 0 );
    resize( width(), 500 );

    ui->colorMapPalette->setDrawingColor( kvs::RGBColor::Black() );
    QPalette palette = ui->drawingColorCLbl->palette();
    palette.setColor( QPalette::Window, Qt::black );
    ui->drawingColorCLbl->setAutoFillBackground( true );
    ui->drawingColorCLbl->setPalette( palette );

    connect( ui->colorMapBarTabWidget, &QTabWidget::currentChanged, this, &ColorMapEditor::onCurrentTabChanged );
    connect( ui->drawingColorCLbl, &ClickableLabel::doubleClicked, this, &ColorMapEditor::onDrawingColorDoubleClicked );
    connect( ui->resetPBtn, &QPushButton::clicked, this, &ColorMapEditor::onResetButtonClicked );
    connect( ui->undoPBtn, &QPushButton::clicked, m_undo_stack, &QUndoStack::undo );
    connect( ui->redoPBtn, &QPushButton::clicked, m_undo_stack, &QUndoStack::redo );
    connect( ui->colorMapBarTWidget, &QTableWidget::cellDoubleClicked, this, &ColorMapEditor::onColorMapBarTableWidgetCellDoubleClicked );

    connect( ui->redLEdit, &QLineEdit::textChanged, this, &ColorMapEditor::onExpressionChanged );
    connect( ui->greenLEdit, &QLineEdit::textChanged, this, &ColorMapEditor::onExpressionChanged );
    connect( ui->blueLEdit, &QLineEdit::textChanged, this, &ColorMapEditor::onExpressionChanged );

    connect( ui->numberOfControlPointsSBox, &QSpinBox::valueChanged, this, &ColorMapEditor::onNumberOfControlPointsChabged );
    ui->controlPointsTWidget->setRowCount( ui->numberOfControlPointsSBox->value() );

    connect( ui->controlPointsTWidget, &QTableWidget::cellChanged, this, &ColorMapEditor::onControlPointChanged );
    connect( ui->cancelPBtn, &QPushButton::clicked, this, &ColorMapEditor::close );
    connect( ui->applyPBtn, &QPushButton::clicked, this, &ColorMapEditor::accept );

    QPalette app_palette = QApplication::palette();
    QColor backgroundColor = app_palette.color(QPalette::Window);
    if( backgroundColor.value() < 128 )
    {
        m_is_dark_mode = true;
    }
    else
    {
        m_is_dark_mode = false;
    }
}

ColorMapEditor::~ColorMapEditor()
{
    delete ui;
}

void ColorMapEditor::setColorMap( kvs::ColorMap colorMap )
{
    ui->colorMapPalette->setColorMap( colorMap );
}

void ColorMapEditor::setInitialColorMap( kvs::ColorMap colorMap )
{
    ui->colorMapPalette->setInitialColorMap( colorMap );
}

kvs::ColorMap ColorMapEditor::getColorMap()
{
    return ui->colorMapPalette->getColor();
}

void ColorMapEditor::readJsonFile()
{
    QString filePath = ":/Resources/json/preset_color_map.json";
    QFile file(filePath);
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qDebug() << "Failed to open preset.json.";
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // JSON Document
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);

    // Root JSON Array
    QJsonArray rootArray = jsonDoc.array();

    int count = 0;
    for (const QJsonValue &value : rootArray)
    {
        {
            kvs::ColorMap color_map;

            int row_position = ui->colorMapBarTWidget->rowCount();
            if( count % 2 == 0)
            {
                ui->colorMapBarTWidget->insertRow( row_position );
                count = 0;
            }
            // Convert value to object
            QJsonObject jsonObject = value.toObject();
            QString colorSpace = jsonObject.value("ColorSpace").toString();
            QString name = jsonObject.value("Name").toString();
            QJsonArray rgbPointsArray = jsonObject.value("RGBPoints").toArray();

            float min_value = rgbPointsArray.at(0).toDouble();
            float max_value = rgbPointsArray.at(rgbPointsArray.size() - 4).toDouble();

            if( colorSpace == "Diverging" )
            {
                if( rgbPointsArray.size() / 4 == 3 )
                {
                    int r1 = rgbPointsArray.at( 1 ).toDouble() * 255;
                    int g1 = rgbPointsArray.at( 2 ).toDouble() * 255;
                    int b1 = rgbPointsArray.at( 3 ).toDouble() * 255;

                    int r2 = rgbPointsArray.at( rgbPointsArray.size() - 3 ).toDouble() * 255;
                    int g2 = rgbPointsArray.at( rgbPointsArray.size() - 2 ).toDouble() * 255;
                    int b2 = rgbPointsArray.at( rgbPointsArray.size() - 1 ).toDouble() * 255;

                    color_map = kvs::DivergingColorMap::Create( kvs::RGBColor( r1, g1, b1 ), kvs::RGBColor( r2, g2, b2 ), 256 );
                }
                else
                {
                    color_map = kvs::ColorMap( 256, min_value, max_value );
                    for (int i = 0; i < rgbPointsArray.size(); i += 4)
                    {
                        float value = rgbPointsArray.at(i).toDouble();
                        int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                        int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                        int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                        color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                    }
                    color_map.create();
                }
            }
            else if( colorSpace == "Lab" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for (int i = 0; i < rgbPointsArray.size(); i += 4)
                {
                    float value = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                }
                color_map.create();
            }
            else if( colorSpace == "RGB" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for (int i = 0; i < rgbPointsArray.size(); i += 4)
                {
                    float value = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                }
                color_map.create();
            }
            else if( colorSpace == "CIELAB" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for (int i = 0; i < rgbPointsArray.size(); i += 4)
                {
                    float value = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                }
                color_map.create();
            }
            else if( colorSpace == "HSV" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                if( name == "Blue to Red Rainbow" )
                {
                    color_map.addPoint(0/4.0, kvs::RGBColor( 0, 0, 255 ));
                    color_map.addPoint(1/4.0, kvs::RGBColor( 0, 255, 255 ));
                    color_map.addPoint(2/4.0, kvs::RGBColor( 0, 255, 0 ));
                    color_map.addPoint(3/4.0, kvs::RGBColor( 255, 255, 0 ));
                    color_map.addPoint(4/4.0, kvs::RGBColor( 255, 0, 0 ));
                }
                else
                {
                    for (int i = 0; i < rgbPointsArray.size(); i += 4)
                    {
                        float value = rgbPointsArray.at(i).toDouble();
                        int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                        int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                        int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                        color_map.addPoint( value, kvs::RGBColor( r, g, b ) );
                    }
                }
                color_map.create();
            }
            else if( colorSpace == "Step" )
            {
                color_map = kvs::ColorMap( 256, min_value, max_value );
                for ( int i = 0; i < rgbPointsArray.size(); i += 4 )
                {
                    float value1 = rgbPointsArray.at(i).toDouble();
                    int r = rgbPointsArray.at(i + 1).toDouble() * 255;
                    int g = rgbPointsArray.at(i + 2).toDouble() * 255;
                    int b = rgbPointsArray.at(i + 3).toDouble() * 255;
                    if( i < 8 )
                    {
                        color_map.addPoint( value1, kvs::RGBColor( r, g, b ) );
                    }
                    else
                    {
                        float value2 = rgbPointsArray.at(i - 4).toDouble();
                        color_map.addPoint( value2, kvs::RGBColor( r, g, b ) );
                        color_map.addPoint( value1, kvs::RGBColor( r, g, b ) );
                    }
                }
                color_map.create();
            }

            TFEColorMapBar* colorMapBar = new TFEColorMapBar();
            colorMapBar->setColorMap( color_map );
            colorMapBar->setFixedWidth( 200 );
            colorMapBar->setMaximumHeight( 25 );

            QLabel* label = new QLabel( name );
            label->setWordWrap( true );
            label->setMinimumWidth( 10 );
            label->setMargin( 0 );

            QHBoxLayout* layout = new QHBoxLayout();
            layout->addWidget( colorMapBar );
            layout->addWidget( label );

            QWidget* containerWidget = new QWidget();
            containerWidget->setLayout(layout);
            ui->colorMapBarTWidget->setCellWidget(ui->colorMapBarTWidget->rowCount() - 1, count, containerWidget);
            count++;

        }
    }
}

/*===========================================================================*/
/**
 *  @brief  showEvent function.
 *  @note   This is to address the problem that the ColorMap of the Function selected in TransferFunctionEditor
 *  @note   is not immediately reflected when ColorMapEditor is opened in a linux environment.
 */
/*===========================================================================*/
void ColorMapEditor::showEvent(QShowEvent *event)
{
    Q_UNUSED( event );
    ui->colorMapPalette->update();
}

//Presets        0
//Freeform curve 1
//Expression     2
//Control Point  3
void ColorMapEditor::onCurrentTabChanged( int index )
{    
    if( index == 0 || index == 3)
    {
        resize( width(), 500 );
    }
    else if( index == 1 )
    {
        resize( width(), minimumHeight() );
    }
    else if( index == 2 )
    {
        resize( width(), minimumHeight() );
        onExpressionChanged();
    }
}

void ColorMapEditor::onDrawingColorDoubleClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"));

    QPalette palette = ui->drawingColorCLbl->palette();
    palette.setColor( QPalette::Window, color );

    ui->drawingColorCLbl->setPalette( palette );

    ui->colorMapPalette->setDrawingColor( kvs::RGBColor( color.red(), color.green(), color.blue() ) );
    raise();
}

void ColorMapEditor::onResetButtonClicked()
{
    ui->colorMapPalette->reset();
}

void ColorMapEditor::onColorMapBarTableWidgetCellDoubleClicked( int row, int column )
{
    //警告:nullptrの処理を追記すること。
    QWidget* cellWidget = ui->colorMapBarTWidget->cellWidget( row, column );
    TFEColorMapBar* colorMapBar2 = qobject_cast<TFEColorMapBar*>(cellWidget->layout()->itemAt(0)->widget());
    ui->colorMapPalette->setColorMap( colorMapBar2->getColor() );
    ui->colorMapPalette->update();
}

//IMPORT FROM OLD PBVR(colormappalette.cpp setColorMapEquation)
void ColorMapEditor::onExpressionChanged()
{
    std::string rfe = ui->redLEdit->text().toStdString();
    std::string gfe = ui->greenLEdit->text().toStdString();
    std::string bfe = ui->blueLEdit->text().toStdString();

    const float min_value = 0.0;
    const float max_value = 1.0;

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function rf, gf, bf;

    char charx1[2] = "x";
    var_x.tag(charx1);
    vars.push_back(var_x);

    FuncParser::FunctionParser rf_parse(rfe, (int)rfe.size() + 1);
    FuncParser::FunctionParser gf_parse(gfe, (int)gfe.size() + 1);
    FuncParser::FunctionParser bf_parse(bfe, (int)bfe.size() + 1);
    FuncParser::FunctionParser::Error err_r = rf_parse.express(rf, vars);
    FuncParser::FunctionParser::Error err_g = gf_parse.express(gf, vars);
    FuncParser::FunctionParser::Error err_b = bf_parse.express(bf, vars);

    if (err_r != FuncParser::FunctionParser::ERR_NONE)
    {
        ui->redLEdit->setStyleSheet("background-color: red;");
    }
    else
    {
        ui->redLEdit->setStyleSheet("");
    }

    if (err_g != FuncParser::FunctionParser::ERR_NONE)
    {        
        ui->greenLEdit->setStyleSheet("background-color: red;");
    }
    else
    {
        ui->greenLEdit->setStyleSheet("");
    }

    if (err_b != FuncParser::FunctionParser::ERR_NONE)
    {        
        ui->blueLEdit->setStyleSheet("background-color: red;");
    }
    else
    {
        ui->blueLEdit->setStyleSheet("");
    }

    //RGBの式にエラーがなければカラーマップを生成
    if (err_r == FuncParser::FunctionParser::ERR_NONE &&
        err_g == FuncParser::FunctionParser::ERR_NONE &&
        err_b == FuncParser::FunctionParser::ERR_NONE)
    {        
        kvs::ColorMap cmap( 256, min_value, max_value);

        const float stride = (max_value - min_value) / ( 256 - 1 );
        float x = min_value;
        for (size_t i = 0; i < 256; ++i, x += stride)
        {
            // int r,g,b;
            float r, g, b; // kawamura

            var_x = x;
            r = rf.eval();
            g = gf.eval();
            b = bf.eval();

            /*
            r = ( r > 255 )? 255: ( r < 0 )? 0: r;
            g = ( g > 255 )? 255: ( g < 0 )? 0: g;
            b = ( b > 255 )? 255: ( b < 0 )? 0: b;
            */
            // kawamura
            r = (r > 1.0) ? 1.0 : (r < 0) ? 0 : r;
            g = (g > 1.0) ? 1.0 : (g < 0) ? 0 : g;
            b = (b > 1.0) ? 1.0 : (b < 0) ? 0 : b;

            r *= 255;
            g *= 255;
            b *= 255;

            kvs::RGBColor color((int)r, (int)g, (int)b);
            cmap.addPoint(x, color);
        }
        cmap.create();

        ui->colorMapPalette->setColorMap( cmap );
        ui->colorMapPalette->update();
    }
}

void ColorMapEditor::onNumberOfControlPointsChabged( int value )
{
    ui->controlPointsTWidget->setRowCount( value );
    onControlPointChanged();
}

void ColorMapEditor::onControlPointChanged()
{
    const float max_value = 1.0;
    const float min_value = 0.0;

    kvs::ColorMap cmap( 256, min_value, max_value );

    bool valid_float;
    bool valid_row;

    for ( int n = 0; n < ui->controlPointsTWidget->rowCount(); n++ )
    {
        valid_row=true;
        float row_values[4]={0.0,0.0,0.0,0.0};
        for (int c=0; c < 4; c++)
        {
            if ( ui->controlPointsTWidget->item(n,c) )
            {
                QString text= ui->controlPointsTWidget->item(n,c)->text();
                row_values[c]=text.toFloat(&valid_float);
                valid_row=valid_row&valid_float;
                if( m_is_dark_mode )
                {
                    ui->controlPointsTWidget->item(n,c)->setForeground(valid_float?Qt::white:Qt::red);
                }
                else
                {
                    ui->controlPointsTWidget->item(n,c)->setForeground(valid_float?Qt::black:Qt::red);
                }
            }
            else
            {
                valid_row=false;
            }
        }

        if (valid_row)
        {
            float x = row_values[0];
            int   r = row_values[1] *255.0;
            int   g = row_values[2] *255.0;
            int   b = row_values[3] *255.0;
            kvs::RGBColor color(r,g,b);
            cmap.addPoint( x, color );
        }
    }
    cmap.create();
    ui->colorMapPalette->setColorMap( cmap );
    ui->colorMapPalette->update();
}

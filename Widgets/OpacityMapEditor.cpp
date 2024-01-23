#include "OpacityMapEditor.h"
#include "ui_OpacityMapEditor.h"
#include "FunctionParser/function_parser.h"
OpacityMapEditor::OpacityMapEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpacityMapEditor)
{
    ui->setupUi(this);

    ui->controlPointsTWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->opacityMapBarTabWidget->setCurrentIndex( 0 );

    connect( ui->opacityMapBarTabWidget, &QTabWidget::currentChanged, this, &OpacityMapEditor::onCurrentTabChanged );
    connect( ui->numberOfControlPointsSBox, &QSpinBox::valueChanged, this, &OpacityMapEditor::onNumberOfControlPointsChabged );
    ui->controlPointsTWidget->setRowCount( ui->numberOfControlPointsSBox->value() );

    connect( ui->oLEdit, &QLineEdit::textChanged, this, &OpacityMapEditor::onExpressionChanged );
    connect( ui->controlPointsTWidget, &QTableWidget::cellChanged, this, &OpacityMapEditor::onControlPointChanged );

    connect( ui->applyPBtn, &QPushButton::clicked, this, &OpacityMapEditor::accept );
}

OpacityMapEditor::~OpacityMapEditor()
{
    delete ui;
}

kvs::OpacityMap OpacityMapEditor::getOpacityMap()
{
    return ui->opacityMapPalette->getOpacity();
}

//Freeform curve 0
//Expression     1
//Control Point  2
void OpacityMapEditor::onCurrentTabChanged( int index )
{
    if( index == 0 )
    {
        resize( width(), minimumHeight() );
    }
    else if( index == 1 )
    {
        resize( width(), minimumHeight() );
        onExpressionChanged();
    }
    else if( index == 2 )
    {
        resize( width(), 500 );
    }
}

void OpacityMapEditor::onExpressionChanged()
{
    std::string ofe = ui->oLEdit->text().toStdString();

    const float min_value = 0.0;
    const float max_value = 1.0;

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function of;

    char charx1[2] = "x";
    var_x.tag(charx1);
    vars.push_back(var_x);

    FuncParser::FunctionParser of_parse( ofe, ofe.size() + 1 );
    FuncParser::FunctionParser::Error err_o = of_parse.express( of, vars );

    if ( err_o != FuncParser::FunctionParser::ERR_NONE )
    {
        ui->oLEdit->setStyleSheet("background-color: red;");
    }
    else
    {
        ui->oLEdit->setStyleSheet("");
    }

    if( err_o == FuncParser::FunctionParser::ERR_NONE )
    {
        kvs::OpacityMap omap( 256, min_value, max_value );

        const float stride = ( max_value - min_value ) / ( 256 - 1 );
        float x = min_value;
        for ( size_t i = 0; i < 256; ++i, x += stride )
        {
            float opacity;

            var_x = x;
            opacity = of.eval();
            opacity = ( opacity > 1.0 ) ? 1.0 : ( opacity < 0 ) ? 0 : opacity;

            omap.addPoint( x, opacity );
        }
        omap.create();

        ui->opacityMapPalette->setOpacityMap( omap );
        ui->opacityMapPalette->update();
    }
}

void OpacityMapEditor::onNumberOfControlPointsChabged( int value )
{
    ui->controlPointsTWidget->setRowCount( value );
    onControlPointChanged();
}

void OpacityMapEditor::onControlPointChanged()
{
    const float max_value = 1.0;
    const float min_value = 0.0;

    kvs::OpacityMap omap( 256, min_value, max_value );
    omap.addPoint( 0.0, 0.0 );
    omap.addPoint( 1.0, 0.0 );

    bool valid_float;
    bool valid_row;

    for ( int n = 0; n < ui->controlPointsTWidget->rowCount(); n++ )
    {
        valid_row=true;
        float row_values[2]={0.0,0.0};
        for (int c=0; c < 2; c++)
        {
            if (    ui->controlPointsTWidget->item(n,c) )
            {
                QString text= ui->controlPointsTWidget->item(n,c)->text();
                row_values[c]=text.toFloat(&valid_float);
                valid_row=valid_row&valid_float;
                ui->controlPointsTWidget->item(n,c)->setForeground(valid_float?Qt::white:Qt::red);
            }
            else
            {
                valid_row=false;
            }
        }

        if (valid_row)
        {
            float x      = row_values[0];
            float alpha  = row_values[1];
            omap.removePoint(x);
            omap.addPoint(x,alpha);
        }
    }

    omap.create();
    ui->opacityMapPalette->setOpacityMap( omap );
    ui->opacityMapPalette->update();
}

#include "OpacityMapEditor.h"
#include "ui_OpacityMapEditor.h"

OpacityMapEditor::OpacityMapEditor(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OpacityMapEditor)
    ,m_default_opacities( QVector<float> {0.0, 0.5, 1.0} )
{
    ui->setupUi(this);
    initialize();
}

OpacityMapEditor::~OpacityMapEditor()
{
    delete ui;
}

void OpacityMapEditor::setDefaultOpacityMap( const QVector<float>& opacities )
{
    m_default_opacities = opacities;
    ui->opacityMapPalette->setOpacities( opacities );
}

QVector<float> OpacityMapEditor::getOpacityMap()
{
    return ui->opacityMapPalette->getOpacities();
}

void OpacityMapEditor::initialize()
{
    ui->tabWidget->setCurrentIndex( 0 );
    resize( width(), minimumHeight() );

    initializeExpression();
    initializeControlPoints();

    connect( ui->resetPushButton, &QPushButton::clicked, this, &OpacityMapEditor::onReset );
    connect( ui->undoPushButton, &QPushButton::clicked, ui->opacityMapPalette, &OpacityMapPalette::undo );
    connect( ui->redoPushButton, &QPushButton::clicked,ui->opacityMapPalette, &OpacityMapPalette::redo );

    connect( ui->tabWidget, &QTabWidget::currentChanged, this, &OpacityMapEditor::onTabChanged );

    connect( ui->opacityLineEdit, &QLineEdit::textEdited, this, &OpacityMapEditor::onExpressionChanged );

    connect( ui->numberOfControlPointsSpinBox, &QSpinBox::valueChanged, this, &OpacityMapEditor::onNumberOfControlPointsChanged );
    connect( ui->controlPointsTableWidget, &QTableWidget::cellChanged, this, &OpacityMapEditor::onControlPointChanged );

    connect( ui->cancelPushButton, &QPushButton::clicked, this, &OpacityMapEditor::close );
    connect( ui->applyPushButton, &QPushButton::clicked, this, &OpacityMapEditor::accept );
}

void OpacityMapEditor::initializeExpression()
{
    ui->opacityLineEdit->setText( "x * 1" );
}

void OpacityMapEditor::initializeControlPoints()
{
    ui->controlPointsTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    ui->controlPointsTableWidget->setRowCount( ui->numberOfControlPointsSpinBox->value() );
}

void OpacityMapEditor::onReset()
{
    ui->opacityMapPalette->setOpacities( m_default_opacities );
}

void OpacityMapEditor::onTabChanged( int index )
{
    /*
     // * 0: Preset
     // * 1: Freeform Curve
     * 0: Expression
     * 1: Control Points
     */
    if( index == 0 )
    {
        onExpressionChanged();
        resize( width(), minimumHeight() );
    }
    else if( index == 1 )
    {
        onControlPointChanged();
        resize( width(), 500 );
    }
}

void OpacityMapEditor::onExpressionChanged()
{
    std::string opacityFunctionExpression;

#ifdef Q_OS_WIN
    opacityFunctionExpression = ui->opacityLineEdit->text().toLocal8Bit().constData();
#else
    opacityFunctionExpression = ui->opacityLineEdit->text().toStdString();
#endif

    const float min_value = 0.0;
    const float max_value = 1.0;

    FuncParser::Variables variables;
    FuncParser::Variable variable_x;
    FuncParser::Function opacityFunction;

    char charx1[2] = "x";
    variable_x.tag(charx1);
    variables.push_back(variable_x);

    FuncParser::FunctionParser opacityFunctionParse( opacityFunctionExpression, opacityFunctionExpression.size() + 1 );
    FuncParser::FunctionParser::Error errorOpacity = opacityFunctionParse.express( opacityFunction, variables );

    ui->opacityLineEdit->setStyleSheet( errorOpacity != FuncParser::FunctionParser::ERR_NONE ? "background-color: red;" : "" );

    if( errorOpacity == FuncParser::FunctionParser::ERR_NONE )
    {
        QVector<float> opacityMap;

        const float stride = ( max_value - min_value ) / ( 256 - 1 );
        float x = min_value;
        for ( std::size_t i = 0; i < 256; ++i, x += stride )
        {
            float opacity;

            variable_x = x;
            opacity = opacityFunction.eval();
            opacity = ( opacity > 1.0 ) ? 1.0 : ( opacity < 0 ) ? 0 : opacity;

            opacityMap.append( opacity );
        }

        ui->opacityMapPalette->setOpacities( opacityMap );
        ui->opacityMapPalette->update();
    }
}

void OpacityMapEditor::onNumberOfControlPointsChanged( int value )
{
    ui->controlPointsTableWidget->setRowCount( value );
    onControlPointChanged();
}

void OpacityMapEditor::onControlPointChanged()
{
    const float max_value = 1.0f;
    const float min_value = 0.0f;

    QVector<QPair<float, float>> controlPoints;
    bool valid_row;

    const int row_count = ui->controlPointsTableWidget->rowCount();

    for (int row = 0; row < row_count; ++row)
    {
        valid_row = true;
        float x = 0.0f;
        float alpha = 0.0f;

        for (int col = 0; col < 2; ++col)
        {
            QTableWidgetItem* item = ui->controlPointsTableWidget->item(row, col);
            if (!item)
            {
                valid_row = false;
                continue;
            }

            bool ok = false;
            float value = item->text().toFloat(&ok);

            if (!ok)
            {
                valid_row = false;
            }

            // フォアグラウンド色を適切に更新（ダークモード対応）
            // QColor fg_color = ok ? (m_is_dark_mode ? Qt::white : Qt::black) : Qt::red;
            // item->setForeground(fg_color);

            if (col == 0) x = value;
            if (col == 1) alpha = value;
        }

        if (valid_row)
        {
            controlPoints.append({x, alpha});
        }
    }

    // 256分割で線形補間による不透明度マップを生成
    QVector<float> opacityMap(256, 0.0f);

    std::sort(controlPoints.begin(), controlPoints.end(),
              [](const QPair<float, float>& a, const QPair<float, float>& b)
              {
                  return a.first < b.first;
              });

    if (controlPoints.isEmpty())
    {
        ui->opacityMapPalette->setOpacities(opacityMap);
        ui->opacityMapPalette->update();
        return;
    }

    const float stride = (max_value - min_value) / (256 - 1);
    for (int i = 0; i < 256; ++i)
    {
        float x = min_value + i * stride;

        // 線形補間で値を決定
        float alpha = 0.0f;
        if (x <= controlPoints.first().first)
        {
            alpha = controlPoints.first().second;
        }
        else if (x >= controlPoints.last().first)
        {
            alpha = controlPoints.last().second;
        }
        else
        {
            for (int j = 0; j < controlPoints.size() - 1; ++j)
            {
                float x0 = controlPoints[j].first;
                float a0 = controlPoints[j].second;
                float x1 = controlPoints[j + 1].first;
                float a1 = controlPoints[j + 1].second;

                if (x >= x0 && x <= x1)
                {
                    float t = (x - x0) / (x1 - x0);
                    alpha = (1.0f - t) * a0 + t * a1;
                    break;
                }
            }
        }

        // 値のクリップ（0〜1）
        opacityMap[i] = std::clamp(alpha, 0.0f, 1.0f);
    }

    ui->opacityMapPalette->setOpacities(opacityMap);
    ui->opacityMapPalette->update();
}

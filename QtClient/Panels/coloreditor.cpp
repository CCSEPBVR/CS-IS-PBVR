#include "coloreditor.h"
#include "ui_coloreditor.h"
#include <QDebug>
#define STATUS_STYLE(S) S?"color:red":""

#define PREVENT_EVENT_RECURSION      static bool PREVENT_EVENT_RECURSION_FLAG; if(PREVENT_EVENT_RECURSION_FLAG) return; else PREVENT_EVENT_RECURSION_FLAG=true;
#define PREVENT_EVENT_RECURSION_END  PREVENT_EVENT_RECURSION_FLAG=false;

ColorEditor::ColorEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorEditor),
    m_max_value(0.0f),
    m_min_value(0.0f),
    m_resolution(256),
    m_visible(false)
{
    //ADD BY)T.Osaki 2020.02.28
    m_undoStack = new QUndoStack;
    m_undoStack->setUndoLimit(50);
    ui->setupUi(this);
    m_color_palette =ui->qglColorPalette;// new QGlue::ColorPalette( ui->qglColormap );
    m_color_map_palette =ui->qglColormap;//new QGlue::ColorMapPalette( ui->qglColorPalette );

    ui->colormap_selection->addItems(m_color_map_palette->getLibraryEntries());
    ui->colormap_selection->setCurrentRow( 0 );
    ui->colormap_selection->currentItem()->setSelected(true);

    // Temporary strings just to prevent crash in windows debug mode
    std::string tmprfe= ui->expression_r->text().toStdString();
    std::string tmpgfe= ui->expression_g->text().toStdString();
    std::string tmpbfe= ui->expression_b->text().toStdString();


    this->rfe = tmprfe;
    this->gfe = tmpgfe;
    this->bfe = tmpbfe;

    //ADD BY)T.Osaki 2020.02.28
    this->m_color_map_palette->m_cundoStack = m_undoStack;

    this->m_color_map_palette->setEditable(false,0);

    connect(ui->expression_r, &QLineEdit::textChanged, this, &ColorEditor::onExpressionChange);
    connect(ui->expression_g, &QLineEdit::textChanged, this, &ColorEditor::onExpressionChange);
    connect(ui->expression_b, &QLineEdit::textChanged, this, &ColorEditor::onExpressionChange );

    connect(ui->controlPointTable, &QTableWidget::cellChanged, this, &ColorEditor::onControlPointChange);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(ui->toolBox, &QToolBox::currentChanged, this, &ColorEditor::onToolBoxChange);

    connect(ui->colormap_selection, &QListWidget::currentRowChanged,
            this, &ColorEditor::onColormapSelectionChange);

    //ADD BY)T.Osaki 2020.01.29
    connect(ui->pushButton_3, &QPushButton::clicked,
            this,&ColorEditor::onResetButtonClicked);

    //ADD BY)T.Osaki 2020.02.28
    connect(ui->pushButton_4,&QPushButton::clicked,
            m_undoStack,&QUndoStack::undo);
    connect(ui->pushButton_5,&QPushButton::clicked,
            m_undoStack,&QUndoStack::redo);




    ui->toolBox->setCurrentIndex(0);
}

/**
 * @brief ColorEditor::onToolBoxChange
 *        Toolbox selection change handler
 * @param index
 */
void ColorEditor::onToolBoxChange(int index)
{
    //ADD BY)T.Osaki 2020.02.04
    if (index==0){
        m_color_map_palette->setEditable(false,NULL);
    }
    if (index==1){
        m_color_map_palette->setEditable(true, m_color_palette);
    }
    if (index==2){
        onExpressionChange();
        m_color_map_palette->setEditable(false, NULL);
    }
    if (index==3){
        onControlPointChange(0,0);
        m_color_map_palette->setEditable(false, NULL);
    }
}

/**
 * @brief ColorEditor::onExpressionChange
 *        Color map expression change handler
 */
void ColorEditor::onExpressionChange(){

    this->rfe = ui->expression_r->text().toStdString();
    this->gfe = ui->expression_g->text().toStdString();
    this->bfe = ui->expression_b->text().toStdString();

    uint status=this->m_color_map_palette->setColorMapEquation(rfe,gfe,bfe);

    bool redStatus   = status & QGlue::ColorMapPalette::RED_EXPRESSION_ERROR;
    bool greenStatus = status & QGlue::ColorMapPalette::GREEN_EXPRESSION_ERROR;
    bool blueStatus  = status & QGlue::ColorMapPalette::BLUE_EXPRESSION_ERROR;

    ui->expression_r->setStyleSheet(STATUS_STYLE(redStatus));
    ui->expression_g->setStyleSheet(STATUS_STYLE(greenStatus));
    ui->expression_b->setStyleSheet(STATUS_STYLE(blueStatus));

}

/**
 * @brief ColorEditor::onColormapSelectionChange
 *        Predefined Color Map Library list change handler.
 * @param val
 */
void ColorEditor::onColormapSelectionChange(int val)
{
    m_color_map_palette->setColorMapIndex(val);
}

/**
 * @brief ColorEditor::onControlPointChange
 *        Control Point Table entry change handler
 * @param row
 * @param column
 */
void ColorEditor::onControlPointChange(int row, int column)
{
    PREVENT_EVENT_RECURSION

    const float max_value = 1.0;
    const float min_value = 0.0;

    kvs::ColorMap cmap( m_resolution, min_value, max_value );

    bool valid_float;
    bool valid_row;

    for ( int n = 0; n < 10; n++ )
    {
        valid_row=true;
        float row_values[4]={0.0,0.0,0.0,0.0};
        for (int c=0; c < 4; c++){
            if (    ui->controlPointTable->item(n,c) ){
                QString text= ui->controlPointTable->item(n,c)->text();
                row_values[c]=text.toFloat(&valid_float);
                valid_row=valid_row&valid_float;
                ui->controlPointTable->item(n,c)->setForeground(valid_float?Qt::black:Qt::red);
            }
            else{
                valid_row=false;
            }
        }

        if (valid_row){
            float x = row_values[0];
            int   r = row_values[1] *255.0;
            int   g = row_values[2] *255.0;
            int   b = row_values[3] *255.0;
            kvs::RGBColor color(r,g,b);
            cmap.addPoint( x, color );
        }
    }

    cmap.create();
    m_color_map_palette->setColorMap( cmap );

    PREVENT_EVENT_RECURSION_END
}

//ADD BY)T.Osaki 2020.01.29
void ColorEditor::onResetButtonClicked()
{
    qInfo("Clicked Color Editor reset button");
    m_color_map_palette->resetColorMapPalette();
}

ColorEditor::~ColorEditor()
{
    delete ui;
}



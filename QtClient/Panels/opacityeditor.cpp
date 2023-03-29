#include "opacityeditor.h"
#include "ui_opacityeditor.h"
#include <QDebug>

#define STATUS_STYLE(S) S?"":"color:red"

#define PREVENT_EVENT_RECURSION      static bool PREVENT_EVENT_RECURSION_FLAG; if(PREVENT_EVENT_RECURSION_FLAG) {return;} else{ PREVENT_EVENT_RECURSION_FLAG=true;}
#define PREVENT_EVENT_RECURSION_END  PREVENT_EVENT_RECURSION_FLAG=false;

OpacityEditor::OpacityEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpacityEditor),
    m_resolution(256)
{
    //ADD BY)T.Osaki 2020.02.28
    m_undoStack = new QUndoStack;
    m_undoStack->setUndoLimit(50);
    ui->setupUi(this);
    m_opacity_map_palette = ui->qglOpacityMapPalette;
    m_opacity_map_palette->setEditable(true);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //ADD BY)T.Osaki 2020.02.28
    this->m_opacity_map_palette->m_cundoStack = m_undoStack;

    connect(ui->expression_o, &QLineEdit::textChanged, this, &OpacityEditor::onOpacityExpressionChanged);
    connect(ui->toolBox,&QToolBox::currentChanged,this,&OpacityEditor::onToolBoxChanged);
    connect(ui->controlPointTable,&QTableWidget::cellChanged,this,&OpacityEditor::onControlPointChanged);

    //ADD BY)T.Osaki 2020.02.04
    connect(ui->resetButton,&QPushButton::clicked,
            this,&OpacityEditor::onResetButtonClicked);

    //ADD BY)T.Osaki 2020.02.28
    connect(ui->undoButton,&QPushButton::clicked,
            m_undoStack,&QUndoStack::undo);
    connect(ui->redoButton,&QPushButton::clicked,
            m_undoStack,&QUndoStack::redo);

    ui->toolBox->setCurrentIndex(0);
}

OpacityEditor::~OpacityEditor()
{
    delete ui;
}

/**
 * @brief OpacityEditor::onOpacityExpressionChanged
 *        Event handler for change of Opacity expression
 * @param arg1
 */
void OpacityEditor::onOpacityExpressionChanged(const QString &arg1)
{
    QPushButton* save_button = ui->buttonBox->button(QDialogButtonBox::Save);
    this->ofe =  arg1.toStdString();
    bool status=this->m_opacity_map_palette->setOpacityExpression(this->ofe);
    ui->expression_o->setStyleSheet(STATUS_STYLE(status==0));
    if (status==0){
        m_opacity_map_palette->update();
        save_button->setEnabled(true);
    }
    else{
        save_button->setDisabled(true);
    }
}

/**
 * @brief OpacityEditor::onToolBoxChanged, event handler for toolbox selection
 * @param index
 */
void OpacityEditor::onToolBoxChanged(int index)
{
    QPushButton* save_button = ui->buttonBox->button(QDialogButtonBox::Save);
    if (index==0){
        m_opacity_map_palette->setEditable(true);
        save_button->setEnabled(true);
    }
    if (index==1){
        m_opacity_map_palette->setEditable(false);
        onOpacityExpressionChanged(ui->expression_o->text());
    }
    if (index==2){
        m_opacity_map_palette->setEditable(false);
        onControlPointChanged(0,0);
        save_button->setEnabled(true);
    }
}

/**
 * @brief OpacityEditor::onControlPointChanged, event handler for control point change
 * @param row
 * @param column
 */
void OpacityEditor::onControlPointChanged(int row, int column)
{
    PREVENT_EVENT_RECURSION

            const float max_value = 1.0;
    const float min_value = 0.0;

    kvs::OpacityMap omap( m_resolution, min_value, max_value );
    omap.addPoint( 0.0, 0.0 );
    omap.addPoint( 1.0, 0.0 );

    bool valid_float;
    bool valid_row;

    for ( int n = 0; n < 10; n++ )
    {
        valid_row=true;
        float row_values[2]={0.0,0.0};
        for (int c=0; c < 2; c++){
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
            float x      = row_values[0];
            float alpha  = row_values[1];
            omap.removePoint(x);
            omap.addPoint(x,alpha);
        }
    }

    omap.create();
    m_opacity_map_palette->setOpacityMap( omap );

    PREVENT_EVENT_RECURSION_END
}

//ADD BY)T.Osaki 2020.02.04
void OpacityEditor::onResetButtonClicked()
{
    std::cout << "Clicked opacity editor reset button" << std::endl;
    m_opacity_map_palette->resetOpacityMapPalette();
}

#include "coloreditdialog.h"
#include "ui_coloreditdialog.h"


ColorEditDialog::ColorEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorEditDialog)
{
    ui->setupUi(this);
    m_color_palette =ui->oglw;
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ColorEditDialog::~ColorEditDialog()
{
    delete ui;
}

/**
 * @brief ColorEditDialog::getColorSelection
 *        Executes modal dialog
 * @return Dialog color selection
 */
kvs::RGBColor ColorEditDialog::getColorSelection( )
{
    if ( this->exec() == QDialog::Accepted){
        selected_color=m_color_palette->color();
    }
    return selected_color;
}



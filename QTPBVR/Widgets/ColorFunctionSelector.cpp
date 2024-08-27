#include "ColorFunctionSelector.h"
#include "ui_ColorFunctionSelector.h"

ColorFunctionSelector::ColorFunctionSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorFunctionSelector)
{
    ui->setupUi(this);
    connect( ui->comboBox, &QComboBox::currentIndexChanged, this, &ColorFunctionSelector::onColorFunctionChanged );
}

ColorFunctionSelector::~ColorFunctionSelector()
{
    delete ui;
}

void ColorFunctionSelector::populateColorFunctionLists(int n){
    QStringList itemList;
    for (int i=1;i <=n;i++ )
    {
        itemList.append(QString("C%1").arg(i));
    }
    ui->comboBox->clear();
    ui->comboBox->addItems(itemList);
}

void ColorFunctionSelector::onColorFunctionChanged( int index )
{
    const NamedTransferFunctionParameter *transfer_function_color = m_extended_transfer_function_message->getColorTransferFunction( index + 1 );
    if( transfer_function_color != NULL )
    {
        kvs::ColorMap color_map = transfer_function_color->colorMap();
        m_color_map_bar->setColorMap( color_map );
        m_color_map_bar->setRange( m_extended_transfer_function_message->getColorTransferFunction(index + 1)->m_color_variable_min, m_extended_transfer_function_message->getColorTransferFunction(index + 1)->m_color_variable_max );
        m_screen->update();
    }
}

void ColorFunctionSelector::updateColorMap()
{
    const NamedTransferFunctionParameter *transfer_function_color = m_extended_transfer_function_message->getColorTransferFunction( ui->comboBox->currentIndex() + 1  );
    if( transfer_function_color != NULL )
    {
        kvs::ColorMap color_map = transfer_function_color->colorMap();
        m_color_map_bar->setColorMap( color_map );
        m_color_map_bar->setRange( m_extended_transfer_function_message->getColorTransferFunction( ui->comboBox->currentIndex() + 1)->m_color_variable_min, m_extended_transfer_function_message->getColorTransferFunction( ui->comboBox->currentIndex() + 1)->m_color_variable_max );
        m_screen->update();
    }
}

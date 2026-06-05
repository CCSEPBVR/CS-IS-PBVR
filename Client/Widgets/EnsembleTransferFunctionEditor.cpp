#include "EnsembleTransferFunctionEditor.h"
#include "ui_EnsembleTransferFunctionEditor.h"

#include <QButtonGroup>

EnsembleTransferFunctionEditor::EnsembleTransferFunctionEditor( QWidget* parent )
    : QDialog( parent )
    , ui( new Ui::EnsembleTransferFunctionEditor )
{
    ui->setupUi( this );
    initializeButtonGroups();

    // Future signal/slot extension point:
    // connect statistics and MinMax radio button changes to ensemble TF state updates here.
}

EnsembleTransferFunctionEditor::~EnsembleTransferFunctionEditor()
{
    delete ui;
}

void EnsembleTransferFunctionEditor::initializeButtonGroups()
{
    m_statistics_group = new QButtonGroup( this );
    m_statistics_group->setObjectName( QStringLiteral( "StatisticsGroup" ) );
    m_statistics_group->addButton( ui->m_average_radio_button );
    m_statistics_group->addButton( ui->m_variance_radio_button );
    m_statistics_group->addButton( ui->m_coefficient_variation_radio_button );
    m_statistics_group->setExclusive( true );

    m_average_min_max_group = new QButtonGroup( this );
    m_average_min_max_group->setObjectName( QStringLiteral( "AverageMinMaxGroup" ) );
    m_average_min_max_group->addButton( ui->m_average_server_min_max_radio_button );
    m_average_min_max_group->addButton( ui->m_average_user_min_max_radio_button );
    m_average_min_max_group->setExclusive( true );

    m_variance_min_max_group = new QButtonGroup( this );
    m_variance_min_max_group->setObjectName( QStringLiteral( "VarianceMinMaxGroup" ) );
    m_variance_min_max_group->addButton( ui->m_variance_server_min_max_radio_button );
    m_variance_min_max_group->addButton( ui->m_variance_user_min_max_radio_button );
    m_variance_min_max_group->setExclusive( true );

    m_coefficient_variation_min_max_group = new QButtonGroup( this );
    m_coefficient_variation_min_max_group->setObjectName( QStringLiteral( "CoefficientVariationMinMaxGroup" ) );
    m_coefficient_variation_min_max_group->addButton( ui->m_coefficient_variation_server_min_max_radio_button );
    m_coefficient_variation_min_max_group->addButton( ui->m_coefficient_variation_user_min_max_radio_button );
    m_coefficient_variation_min_max_group->setExclusive( true );
}

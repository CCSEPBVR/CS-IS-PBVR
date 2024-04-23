#include "Coordinates.h"
#include "ui_Coordinates.h"

#include "FunctionParser/ExpressionTokenizer.h"
#include "FunctionParser/ExpressionConverter.h"

Coordinates::Coordinates(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Coordinates),
    m_client_message( nullptr )
{
    ui->setupUi(this);
    connect( ui->applyPBtn, &QPushButton::clicked, this, &Coordinates::onApplyButtonClicked );
}

Coordinates::~Coordinates()
{
    delete ui;
}

//jpv::ParticleTransferClientMessage::EquationToken Coordinates::convertToken( std::string expression )
//{
//    FuncParser::ExpressionTokenizer tokenizer;
//    FuncParser::ExpressionConverter exprconv;

//    jpv::ParticleTransferClientMessage::EquationToken eq_token;

//    tokenizer.tokenizeString( expression );
//    exprconv.convertExpToken( tokenizer.m_exp_token );
//    int size = exprconv.token_array.size();
//    if( size > 128 ){ printf("Equation length too long\n");}

//    for( int i = 0; i < 128; i++ )
//    {
//        if( i < size )
//        {
//            eq_token.exp_token[i]   = exprconv.token_array[i];
//            eq_token.var_name[i]    = exprconv.var_array[i];
//            eq_token.value_array[i] = exprconv.value_array[i];
//        }
//        else
//        {
//            eq_token.exp_token[i]   = 0;
//            eq_token.var_name[i]    = 0;
//            eq_token.value_array[i] = 0;
//        }
//    }

//    std::cout << "exp" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.exp_token[i] << ",";
//    }
//    std::cout << std::endl;
//    std::cout << "var_name" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.var_name[i] << ",";
//    }
//    std::cout << std::endl;
//    std::cout << "value_array" << std::endl;
//    for(int i = 0; i < 128; i++)
//    {
//        std::cout << eq_token.value_array[i] << ",";
//    }
//    std::cout << std::endl;

//    return eq_token;
//}

void Coordinates::onApplyButtonClicked()
{
    m_client_message->m_x_synthesis = ui->coordinateXLEdit->text().toStdString();
    m_client_message->m_y_synthesis = ui->coordinateYLEdit->text().toStdString();
    m_client_message->m_z_synthesis = ui->coordinateZLEdit->text().toStdString();

//    if( !m_client_message->m_x_synthesis.empty() ) m_client_message->x_synthesis_token = this->convertToken( m_client_message->m_x_synthesis );
//    if( !m_client_message->m_y_synthesis.empty() ) m_client_message->y_synthesis_token = this->convertToken( m_client_message->m_y_synthesis );
//    if( !m_client_message->m_z_synthesis.empty() ) m_client_message->z_synthesis_token = this->convertToken( m_client_message->m_z_synthesis );
}

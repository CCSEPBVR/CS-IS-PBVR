#include "ExpressionTokenizer.h"

namespace FuncParser
{

FuncParser::ExpressionTokenizer::ExpressionTokenizer( void )
{
    m_exp_token = new ExpToken[TOKEN_ARRAY_SIZE];
}

}// end of namespace FuncPaarse

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include "ExpressionTokenizer.h"
#include "ExpressionConverter.h"
#include "ReversePolishNotation.h"

int main( void )
{
    FuncParser::ExpressionTokenizer tokenizer;

    FuncParser::ExpressionConverter exprconv;

    FuncParser::ReversePolishNotation rpn;

    tokenizer.tokenizeString("-gauss(x+1,y*2)+1");

    exprconv.convertExpToken( tokenizer.m_exp_token );

    tokenizer.tokenizeString("(x+y)*(x+sin(z+log(7+9)))");

    exprconv.convertExpToken( tokenizer.m_exp_token );

    tokenizer.tokenizeString("((c1+c2)^2-a1)*gauss(dq2z,dq2y)");

    exprconv.convertExpToken( tokenizer.m_exp_token );

    // sqrt(1+3)-2*2^(1+3-1) = -14
    tokenizer.tokenizeString("sqrt(x+q2)-2*2^(1+dq3y-c4)");
    exprconv.convertExpToken( tokenizer.m_exp_token );

    int size = exprconv.token_array.size();
    std::cout<<"token size = "<<size<<std::endl;

    int exp_token[128];
    int var_name[128];
    float value_array[128];
    float var_value[128];

    for( int i = 0; i < size; i++ )
    {
        exp_token[i] = exprconv.token_array[i];
        var_name[i] = exprconv.var_array[i];
        value_array[i] = exprconv.value_array[i];
    }

    rpn.setExpToken( &exp_token[0] );
    rpn.setVariableName( &var_name[0] );
    rpn.setNumber( &value_array[0] );

    for( int i = 0; i < 128; i++)
        var_value[i] = 0.0;

    var_value[X] = 1;
    var_value[Q2] = 3;
    var_value[DQ3Y] = 3;
    var_value[C4] = 1;

    rpn.setVariableValue( &var_value[0] );

    std::cout<<"result = "<< rpn.eval() <<std::endl;

    return 0;
}

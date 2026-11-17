#include <vismodule/EnsembleTransferFunction>
#include <vismodule/TransferFunctionSynthesizer>

EquationToken EnsembleTransferFunction::convert_token( const std::string expression )
{
    FuncParser::ExpressionTokenizer tokenizer;
    FuncParser::ExpressionConverter exprconv;
    EquationToken eq_token;

    tokenizer.tokenizeString( expression );
    exprconv.convertExpToken( tokenizer.m_exp_token );

    int size = exprconv.token_array.size();
    if( size > 128 ) printf("Equation length too long\n");

    for( int i = 0; i < 128; i++ )
    {
        if( i < size )
        {
            eq_token.exp_token[i] = exprconv.token_array[i];
            eq_token.var_name[i]  = exprconv.var_array[i];
            eq_token.val_array[i] = exprconv.value_array[i];
        }
        else
        {
            eq_token.exp_token[i] = 0;
            eq_token.var_name[i]  = 0;
            eq_token.val_array[i] = 0;
        }
    }

    return eq_token;
}

/*****************************************************************************/
/**
 *  @file   function_parser.cpp
 *  @brief  function parser class
 *
 *  @author Yukio YASUHARA
 *  @date   2006/12/16 16:47:02
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: yukky $
 *  $Date: 2006/12/18 04:25:11 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS/Library/Numeric/function_parser.cpp,v $
 *  $Revision: 1.6 $
 */
/*****************************************************************************/
#include <cstdio>
#include <cmath>
#include <iostream>

#include "function_parser.h"
#include "function_op.h"

using namespace std;

namespace FuncParser
{

string FunctionParser::error_type_to_string[NUM_OF_ERROR] =
{
    "",
    "[ERROR]FunctionParser::No Operand",
    "[ERROR]FunctionParser::Divide by zero",
    "[ERROR]FunctionParser::No right quate",
    "[ERROR]FunctionParser::Syntax error"
};

string FunctionParser::math_func_to_string[NUM_OF_MATH_FUNC] =
{
    "sin",
    "cos",
    "tan",
    "sqrt",
    "log",
    "exp"
};

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  name
 *  @param  len
 */
/*===========================================================================*/
int FunctionParser::find_func( char* name, int len )
{
    for ( int i = 0; i < ( int )( sizeof( math_func_to_string ) / sizeof( math_func_to_string[0] ) ); i++ )
    {
        if ( !strncmp( name, math_func_to_string[i].c_str(), len ) )
        {
            return ( TK_RESERVE + 1 + i );
        }
    }
    return ( TK_ERROR );
}

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  token
 */
/*===========================================================================*/
void FunctionParser::unget_token( int token )
{
    m_backed_token = token;
}

/**
 * 変数の文字数のカウントを行う
 * 先頭が英字、２文字目以降が英数字である文字列の文字数を取得する
 * @param var		式文字列
 * @return		変数の文字数
 */
int FunctionParser::count_variable( const char *var )
{
    if (var == NULL) return 0;
    if (*var == '\0') return 0;
    char str = *var;
    int count = 0;
    // 1文字目は英字であること
    if (!isalpha(str)) return 0;
    count++;

    while (str != '\0') {
        // 英数字であるかチェックする
        str = *(var+count);
        if (isalnum(str)) {
            count++;
        }
        else {
            break;
        }
    }

    return count;
}


/*===========================================================================*/
/**
 *  @brief
 *
 */
/*===========================================================================*/
int FunctionParser::get_token( Variables& vars )
{
    if ( m_backed_token >= 0 )
    {
        int ret = m_backed_token;
        m_backed_token = -1;
        return ( ret );
    }

    while ( isspace( *m_top ) )
    {
        m_top++;
    }

    // add by @hira at 2016/12/01
    // 変数の文字数のカウントを行う
    int var_count = count_variable(m_top);

    if (var_count > 0) {
        for ( int i = 0; i < ( int )vars.size(); i++ )
        {
            string tmp( vars[i].tag() );
            int size = tmp.size();

            // modify by @hira at 2016/12/01
            // if ( !strcmp( vars[i].tag(), m_top, size ) )
            if ( !strncmp( vars[i].tag(), m_top, var_count ) )
            {
                m_var = i;

                for ( int j = 0; j < var_count; j++ )
                {
                    m_top++;
                }
                return ( TK_VARIABLE );
            }
        }
    }

    switch ( *m_top )
    {
    case '\0':
        return ( TK_EOF );
    case '+':
        m_top++;
        return ( TK_PLUS );
    case '-':
        m_top++;
        return ( TK_MINUS );
    case '*':
        m_top++;
        return ( TK_MULTI );
    case '/':
        m_top++;
        return ( TK_DIV );
    case '^':
        m_top++;
        return ( TK_POW );
    case '(':
        m_top++;
        return ( TK_LEFT );
    case ')':
        m_top++;
        return ( TK_RIGHT );

    default:
    {
        if ( isdigit( *m_top ) )
        {
            m_value = strtod( m_top, &m_top );
            return ( TK_NUM );
        }

        else if ( isalpha( *m_top ) )
        {
            char* p = m_top;

            while ( isalpha( *m_top ) )
            {
                m_top++;
            }

            int token = find_func( p, m_top - p );

            if ( token == TK_ERROR )
            {
                m_error_code = ERR_SYNTAX;
            }
            return ( token );
        }
        else
        {
            m_error_code = ERR_SYNTAX;
            return ( TK_ERROR );
        }
    }
    }
}

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  vars
 */
/*===========================================================================*/
Function FunctionParser::prim( Variables& vars )
{
    Function val;
    int token = get_token( vars );

    switch ( token )
    {
    case TK_NUM:
    {
        Function num = m_value;
        return ( num );
    }
    case TK_LEFT:
        val = expr( vars );
        if ( get_token( vars ) != TK_RIGHT )
        {
            m_error_code = ERR_RIGHT;
            Function func;
            func = 0;
            return ( func );
        }
        return ( val );

    case TK_MINUS:
    {
        Function func;
        func = -1 * prim( vars );
        return ( func );
    }
    case TK_SIN:
        return ( Sin( prim( vars ) ) );
    case TK_COS:
        return ( Cos( prim( vars ) ) );
    case TK_TAN:
        return ( Tan( prim( vars ) ) );
    case TK_SQRT:
        return ( Sqrt( prim( vars ) ) );
    case TK_LOG:
        return ( Log( prim( vars ) ) );
    case TK_EXP:
        return ( Exp( prim( vars ) ) );
    case TK_VARIABLE:
        return ( vars[m_var] );
    default:
    {
        m_error_code = ERR_OPERAND;
        Function func;
        func = 0;
        return ( func );
    }
    }
}

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  vars
 */
/*===========================================================================*/
Function FunctionParser::factor( Variables& vars )
{
    Function val = prim( vars );

    while ( 1 )
    {
        int token = get_token( vars );

        switch ( token )
        {
        case TK_POW:
            val = Pow( val, prim( vars ) );
            break;

        default:
            unget_token( token );
            return ( val );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  vars
 */
/*===========================================================================*/
Function FunctionParser::term( Variables& vars )
{
    Function val = factor( vars );

    while ( 1 )
    {
        int token = get_token( vars );

        switch ( token )
        {
        case TK_MULTI:
        {
            val = val * factor( vars );
            break;
        }
        case TK_DIV:
        {
            Function val2 = factor( vars );
            val = val / val2;
            break;
        }
        default:
            unget_token( token );
            return ( val );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  vars
 */
/*===========================================================================*/
Function FunctionParser::expr( Variables& vars )
{
    Function val = term( vars );

    while ( 1 )
    {
        int token = get_token( vars );

        switch ( token )
        {
        case TK_PLUS:
            val = val + term( vars );
            break;
        case TK_MINUS:
        {
            Function func = -1 * term( vars );
            val = val + func;
            break;
        }
        default:
            unget_token( token );
            return ( val );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief
 *
 *  @param  f
 *  @param  vars
 */
/*===========================================================================*/
FunctionParser::Error FunctionParser::express( Function& f, Variables& vars )
{
    f = expr( vars );

    if ( getToken( vars ) != TK_EOF )
    {
        return ( ERR_SYNTAX );
    }

    return ( getErrorCode() );
}

} // end of namespace FuncParser


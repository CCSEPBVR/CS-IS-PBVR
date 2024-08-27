/*****************************************************************************/
/**
 *  @file   function_parser.h
 *  @brief  function parser class
 *
 *  @author Yukio YASUHARA
 *  @date   2007/11/16 13:55:15
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: naohisa $
 *  $Date: 2007/11/16 11:38:51 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS/Library/Numeric/function_parser.h,v $
 *  $Revision: 1.6 $
 */
/*****************************************************************************/
#ifndef KVS__FUNCTION_PARSER_H_INCLUDE
#define KVS__FUNCTION_PARSER_H_INCLUDE

#include <string>
#include "function.h"


namespace FuncParser
{

/*===========================================================================*/
/**
*  @brief  Function parser class
*/
/*===========================================================================*/
class FunctionParser
{
public:

    enum Error
    {
        ERR_NONE     = 0,
        ERR_OPERAND  = 1,
        ERR_DIVZERO  = 2,
        ERR_RIGHT    = 3,
        ERR_SYNTAX   = 4,
        NUM_OF_ERROR = 5
    };

    static std::string error_type_to_string[NUM_OF_ERROR];

    enum MathFunc
    {
        SIN              = 0,
        COS              = 1,
        TAN              = 2,
        SQRT             = 3,
        LOG              = 4,
        EXP              = 5,
        NUM_OF_MATH_FUNC = 6
    };

    static std::string math_func_to_string[NUM_OF_MATH_FUNC];

    enum Token
    {
        TK_EOF          = 0,
        TK_NUM          = 1,
        TK_PLUS         = 2,
        TK_MINUS        = 3,
        TK_MULTI        = 4,
        TK_DIV          = 5,
        TK_POW          = 6,
        TK_LEFT         = 7,
        TK_RIGHT        = 8,
        TK_ERROR        = 9,
        TK_RESERVE      = 10,
        TK_SIN          = 11,
        TK_COS          = 12,
        TK_TAN          = 13,
        TK_SQRT         = 14,
        TK_LOG          = 15,
        TK_EXP          = 16,
        TK_VARIABLE     = 17,
        NUM_OF_TOKEN    = 18
    };

protected:

    char*     m_func;
    char*     m_top;          //!< position of anlysis
    double    m_value;        //!< value when token is number
    int       m_backed_token;
    int       m_var;
    Error     m_error_code;   //!< type of error

public:

    FunctionParser( std::string& func, const int& size )
    {
        m_func = new char [size];
        if ( !m_func )
        {
            //KVS_MACRO_ERROR_MESSAGE("Cannot allocate memory for the function.");
            std::cout << "Cannot allocate memory for the function.\n";
            return;
        }
        memcpy( m_func, ( char* )func.c_str(), size );

        m_top          = m_func;
        m_backed_token = -1;
        m_var          = -1;
        m_error_code   = ERR_NONE;
    };

    virtual ~FunctionParser( void )
    {
        if ( m_func )
        {
            delete [] m_func;
            m_func = NULL;
        }
    };

public:

    Error express( Function& f, Variables& vars );

    int getToken( Variables& vars )
    {
        return ( get_token( vars ) );
    };

    Error getErrorCode( void )
    {
        return ( m_error_code );
    };

private:

    int      find_func( char* name, int len );

    void     unget_token( int token );

    int      get_token( Variables& vars );

    Function prim( Variables& vars );

    Function factor( Variables& vars );

    Function term( Variables& vars );

    Function expr( Variables& vars );

    int count_variable( const char *var );
};

} // end of namespace FuncParser

#endif // KVS__FUNCTION_PARSER_H_INCLUDE

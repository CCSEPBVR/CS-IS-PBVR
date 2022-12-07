/*****************************************************************************/
/**
 *  @file   test.cpp
 *  @brief  Test program for libnumeric.
 *
 *  @author Naohisa Sakamoto
 *  @date   2006/09/07 12:04:44
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: naohisa $
 *  $Date: 2006/09/07 03:04:39 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS/Library/Numeric/test.cpp,v $
 *  $Revision: 1.4 $
 */
/*****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
//#include <kvs/Timer>

#include "function.h"
#include "function_op.h"
#include "function_parser.h"


using namespace std;
using namespace kvs;

#define TEST_COUT_INDENT (cout << "\t")
#define TEST_LOOP_COUNT  100000

/*===========================================================================*/
/**
 *  @brief  Output test function title tag.
 */
/*===========================================================================*/
inline void TEST_FUNC( string func_name )
{
    cout << "=======================================================" << endl;
    cout << "TEST PROGRAM: " << func_name << "()" << endl;
    cout << "=======================================================" << endl;
}

/*===========================================================================*/
/**
 *  @brief  Test program for function parser.
 */
/*===========================================================================*/
void test_function_parser( void )
{
    TEST_FUNC( "test_function_parser" );

    string str;
    cout << "Input function : ( x , y , z )" << endl;
    cin >> str;

    // Defined the variables. You must defined tags.
    Variable x;
    x.tag( "x" );
    Variable y;
    y.tag( "y" );
    Variable z;
    z.tag( "z" );

    Variables vars;
    vars.push_back( x );
    vars.push_back( y );
    vars.push_back( z );

    Function f;

    // Parse the input function.

    FunctionParser func_parse( str, str.size() + 1 );

    FunctionParser::Error err = func_parse.express( f, vars );


    // Output result.
    if ( err == FunctionParser::ERR_NONE )
    {
        cout << "Function : " << f.str() << endl << endl;


        float tmp;
        cout << "input\nx=";
        cin >> tmp;
        x = tmp;
        cout << std::endl;
        cout << "input\ny=";
        cin >> tmp;
        y = tmp;
        cout << std::endl;
        cout << "input\nz=";
        cin >> tmp;
        z = tmp;
        cout << std::endl;
        std::cout << "f(x,y,z)=" << f.eval() << std::endl;
    }
    else
    {
        char out[100];
        sprintf( out, "error %s", FunctionParser::error_type_to_string[err].c_str() );
        string error( out );
        cout << error << endl;

        return;
    }

}

/*===========================================================================*/
/**
 *  @brief  Main function.
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
    test_function_parser();

    return ( 1 );
}

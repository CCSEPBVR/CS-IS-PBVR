#ifndef _TOKEN
#define _TOKEN

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>

//変数名のリスト
enum VarName {
    NOTHING,//0
    X, Y, Z,//3
    Q1,DQ1X,DQ1Y,DQ1Z,  Q2,DQ2X,DQ2Y,DQ2Z,  Q3,DQ3X,DQ3Y,DQ3Z,//15
    Q4,DQ4X,DQ4Y,DQ4Z,  Q5,DQ5X,DQ5Y,DQ5Z,  Q6,DQ6X,DQ6Y,DQ6Z,//27
    Q7,DQ7X,DQ7Y,DQ7Z,  Q8,DQ8X,DQ8Y,DQ8Z,  Q9,DQ9X,DQ9Y,DQ9Z,//39
    Q10,DQ10X,DQ10Y,DQ10Z,  Q11,DQ11X,DQ11Y,DQ11Z,  Q12,DQ12X,DQ12Y,DQ12Z,//51
    Q13,DQ13X,DQ13Y,DQ13Z,  Q14,DQ14X,DQ14Y,DQ14Z,  Q15,DQ15X,DQ15Y,DQ15Z,//63
    Q16,DQ16X,DQ16Y,DQ16Z,  Q17,DQ17X,DQ17Y,DQ17Z,  Q18,DQ18X,DQ18Y,DQ18Z,//75
    Q19,DQ19X,DQ19Y,DQ19Z,  Q20,DQ20X,DQ20Y,DQ20Z,  Q21,DQ21X,DQ21Y,DQ21Z,//87
    Q22,DQ22X,DQ22Y,DQ22Z,  Q23,DQ23X,DQ23Y,DQ23Z,  //95
    C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16,//111
    A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16 //127
};

//数式表現のためのトークン
enum Token {
    //なにもなし
    NON,//0
    //変数
    VARIABLE,//1
    //数値
    VALUE,//2
    //二項演算子
    PLUS, MINUS, MULTI, DIVIDE, POW,//7
    //単項演算子としてのマイナス
    MINUSF,//8
    //単項関数
    SQRT, CBRT, LOG, LOG10, EXP, ABS, FLOOR, CEIL,//16
    SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, ASINH, ACOSH, ATANH,//28
    HEAVI,RECTFUNC,//30
    //二項関数
    SIGMOID, GAUSS, MIN, MAX, //34
    //関数内のカンマ
    COMMA,//35
    //終了
    END,//36
    //ここまでが、逆ポーランド記法と共有できるトークンの値
    //左括弧、右括弧
    LEFT, RIGHT
};

class ExpString
{
public:
    std::string m_str;
    Token m_token;
    VarName m_var;

    ExpString( void ){}

    void set( std::string str, Token token )
        {
            m_str = str;
            m_token = token;
        }

    void set( std::string str, VarName var )
        {
            m_str = str;
            m_var = var;
        }

    bool operator < ( const ExpString& org ) const
        {
            std::string left = org.m_str;
            return m_str.size() < left.size();
        }

    bool operator > ( const ExpString& org ) const
        {
            std::string left = org.m_str;
            return m_str.size() > left.size();
        }
};

class ExpToken
{
public:
    Token m_token;
    VarName m_var;
    double m_value;

    ExpToken( void )
        {
            m_token = NON;
            m_var = NOTHING;
            m_value = 0.0;
        }
};

class Node {
public:
    ExpToken m_exp[256];
    Node* m_left;
    Node* m_right;

    Node( void ){}

    //トークン配列のENDの位置を検出
    const unsigned int exp_end_pos( void ) const
        {
            unsigned int i = 0;
            while( m_exp[i].m_token != END )
            {
                i++;
            }

            return i;
        }
};

#endif

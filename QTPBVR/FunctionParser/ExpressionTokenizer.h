#ifndef _EXPRESSION_TOKENIZER
#define _EXPRESSION_TOKENIZER

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include "Token.h"

#define TOKEN_ARRAY_SIZE 256

namespace FuncParser
{

class ExpressionTokenizer
{
public:
    ExpToken* m_exp_token;

    ExpressionTokenizer( void );
/*        {
            m_exp_token = new ExpToken[TOKEN_ARRAY_SIZE];
            }*/

    ~ExpressionTokenizer()
        {
            delete [] m_exp_token;
        }

    void tokenizeString( std::string expression )
        {
            //文字列からスペースを削除
            expression  = remove_space( expression );

            //std::cout<<"expression:"<<expression<<std::endl;

            //左側から文字列を比較し、トークンを作成
            string_2_token( expression );
        }

protected:
    //文字列からスペースを削除
    std::string remove_space( std::string expression )
        {
            unsigned int len = expression.length();
            unsigned int pos = expression.find_first_of(" ");
            //string::nposはfindやfind_first_ofで何も見つからなかった時の返り値
            //find_first_ofがnposを返せなかったので、長さで判定
            while( pos < len )
            {
                expression.erase( pos, 1 );
                pos = expression.find_first_of(" ");
            }

            return expression;
        }

    void string_2_token( std::string exp )
        {
            //演算子とトークンの組を定義
            std::vector<ExpString> ope_str(8);
            ope_str[0].set("+",PLUS);
            ope_str[1].set("-",MINUS);
            ope_str[2].set("*",MULTI);
            ope_str[3].set("/",DIVIDE);
            ope_str[4].set("^",POW);
            ope_str[5].set("(",LEFT);
            ope_str[6].set(")",RIGHT);
            ope_str[7].set(",",COMMA);

            //関数文字列とトークンの組を定義
            std::vector<ExpString> func_str(26);
            func_str[0].set("sqrt",SQRT);
            func_str[1].set("cbrt",CBRT);
            func_str[2].set("log",LOG);
            func_str[3].set("log10",LOG10);
            func_str[4].set("exp",EXP);
            func_str[5].set("abs",ABS);
            func_str[6].set("floor",FLOOR);
            func_str[7].set("ceil",CEIL);
            func_str[8].set("sin",SIN);
            func_str[9].set("cos",COS);
            func_str[10].set("tan",TAN);
            func_str[11].set("asin",ASIN);
            func_str[12].set("acos",ACOS);
            func_str[13].set("atan",ATAN);
            func_str[14].set("sinh",SINH);
            func_str[15].set("cosh",COSH);
            func_str[16].set("tanh",TANH);
            func_str[17].set("asinh",ASINH);
            func_str[18].set("acosh",ACOSH);
            func_str[19].set("atanh",ATANH);
            func_str[20].set("heavi",HEAVI);
            func_str[21].set("rect",RECTFUNC);
            func_str[22].set("sigmoid",SIGMOID);
            func_str[23].set("gauss",GAUSS);
            func_str[24].set("min",MIN);
            func_str[25].set("max",MAX);

            //変数文字列とトークンの組を定義
//            std::vector<ExpString> var_str(127);
            std::vector<ExpString> var_str(130);
            var_str[0].set("x",X);
            var_str[1].set("y",Y);
            var_str[2].set("z",Z);
            var_str[3].set("q1",Q1);
            var_str[4].set("dq1x",DQ1X);
            var_str[5].set("dq1y",DQ1Y);
            var_str[6].set("dq1z",DQ1Z);
            var_str[7].set("q2",Q2);
            var_str[8].set("dq2x",DQ2X);
            var_str[9].set("dq2y",DQ2Y);
            var_str[10].set("dq2z",DQ2Z);
            var_str[11].set("q3",Q3);
            var_str[12].set("dq3x",DQ3X);
            var_str[13].set("dq3y",DQ3Y);
            var_str[14].set("dq3z",DQ3Z);
            var_str[15].set("q4",Q4);
            var_str[16].set("dq4x",DQ4X);
            var_str[17].set("dq4y",DQ4Y);
            var_str[18].set("dq4z",DQ4Z);
            var_str[19].set("q5",Q5);
            var_str[20].set("dq5x",DQ5X);
            var_str[21].set("dq5y",DQ5Y);
            var_str[22].set("dq5z",DQ5Z);
            var_str[23].set("q6",Q6);
            var_str[24].set("dq6x",DQ6X);
            var_str[25].set("dq6y",DQ6Y);
            var_str[26].set("dq6z",DQ6Z);
            var_str[27].set("q7",Q7);
            var_str[28].set("dq7x",DQ7X);
            var_str[29].set("dq7y",DQ7Y);
            var_str[30].set("dq7z",DQ7Z);
            var_str[31].set("q8",Q8);
            var_str[32].set("dq8x",DQ8X);
            var_str[33].set("dq8y",DQ8Y);
            var_str[34].set("dq8z",DQ8Z);
            var_str[35].set("q9",Q9);
            var_str[36].set("dq9x",DQ9X);
            var_str[37].set("dq9y",DQ9Y);
            var_str[38].set("dq9z",DQ9Z);
            var_str[39].set("q10",Q10);
            var_str[40].set("dq10x",DQ10X);
            var_str[41].set("dq10y",DQ10Y);
            var_str[42].set("dq10z",DQ10Z);
            var_str[43].set("q11",Q11);
            var_str[44].set("dq11x",DQ11X);
            var_str[45].set("dq11y",DQ11Y);
            var_str[46].set("dq11z",DQ11Z);
            var_str[47].set("q12",Q12);
            var_str[48].set("dq12x",DQ12X);
            var_str[49].set("dq12y",DQ12Y);
            var_str[50].set("dq12z",DQ12Z);
            var_str[51].set("q13",Q13);
            var_str[52].set("dq13x",DQ13X);
            var_str[53].set("dq13y",DQ13Y);
            var_str[54].set("dq13z",DQ13Z);
            var_str[55].set("q14",Q14);
            var_str[56].set("dq14x",DQ14X);
            var_str[57].set("dq14y",DQ14Y);
            var_str[58].set("dq14z",DQ14Z);
            var_str[59].set("q15",Q15);
            var_str[60].set("dq15x",DQ15X);
            var_str[61].set("dq15y",DQ15Y);
            var_str[62].set("dq15z",DQ15Z);
            var_str[63].set("q16",Q16);
            var_str[64].set("dq16x",DQ16X);
            var_str[65].set("dq16y",DQ16Y);
            var_str[66].set("dq16z",DQ16Z);
            var_str[67].set("q17",Q17);
            var_str[68].set("dq17x",DQ17X);
            var_str[69].set("dq17y",DQ17Y);
            var_str[70].set("dq17z",DQ17Z);
            var_str[71].set("q18",Q18);
            var_str[72].set("dq18x",DQ18X);
            var_str[73].set("dq18y",DQ18Y);
            var_str[74].set("dq18z",DQ18Z);
            var_str[75].set("q19",Q19);
            var_str[76].set("dq19x",DQ19X);
            var_str[77].set("dq19y",DQ19Y);
            var_str[78].set("dq19z",DQ19Z);
            var_str[79].set("q20",Q20);
            var_str[80].set("dq20x",DQ20X);
            var_str[81].set("dq20y",DQ20Y);
            var_str[82].set("dq20z",DQ20Z);
            var_str[83].set("q21",Q21);
            var_str[84].set("dq21x",DQ21X);
            var_str[85].set("dq21y",DQ21Y);
            var_str[86].set("dq21z",DQ21Z);
            var_str[87].set("q22",Q22);
            var_str[88].set("dq22x",DQ22X);
            var_str[89].set("dq22y",DQ22Y);
            var_str[90].set("dq22z",DQ22Z);
            var_str[91].set("q23",Q23);
            var_str[92].set("dq23x",DQ23X);
            var_str[93].set("dq23y",DQ23Y);
            var_str[94].set("dq23z",DQ23Z);
            var_str[95].set("c1",C1);
            var_str[96].set("c2",C2);
            var_str[97].set("c3",C3);
            var_str[98].set("c4",C4);
            var_str[99].set("c5",C5);
            var_str[100].set("c6",C6);
            var_str[101].set("c7",C7);
            var_str[102].set("c8",C8);
            var_str[103].set("c9",C9);
            var_str[104].set("c10",C10);
            var_str[105].set("c11",C11);
            var_str[106].set("c12",C12);
            var_str[107].set("c13",C13);
            var_str[108].set("c14",C14);
            var_str[109].set("c15",C15);
            var_str[110].set("c16",C16);
            var_str[111].set("a1",A1);
            var_str[112].set("a2",A2);
            var_str[113].set("a3",A3);
            var_str[114].set("a4",A4);
            var_str[115].set("a5",A5);
            var_str[116].set("a6",A6);
            var_str[117].set("a7",A7);
            var_str[118].set("a8",A8);
            var_str[119].set("a9",A9);
            var_str[120].set("a10",A10);
            var_str[121].set("a11",A11);
            var_str[122].set("a12",A12);
            var_str[123].set("a13",A13);
            var_str[124].set("a14",A14);
            var_str[125].set("a15",A15);
            var_str[126].set("a16",A16);
            var_str[127].set("X",X);
            var_str[128].set("Y",Y);
            var_str[129].set("Z",Z);

            //関数文字列を降順でソート
            std::sort( func_str.begin(), func_str.end(), std::greater<ExpString>() );
			//std::sort(func_str.begin(), func_str.end(), [](const ExpString &left, const ExpString &right) { return left > right; });
			
            //変数文字列を降順でソート
            std::sort( var_str.begin(), var_str.end(), std::greater<ExpString>() );
			//std::sort(var_str.begin(), var_str.end(), [](const ExpString &left, const ExpString &right) { return left > right; });

            unsigned int pos = 0;//数式expの文字の位置
            unsigned int etp = 0;//m_exp_tokenの配列の位置
            unsigned int exp_len = exp.length();
            bool detection;

            while( pos < exp_len )
            {
                detection = false;

                //数字を検出
                if( isdigit( exp.c_str()[pos] ) )
                {
                    const char* startp = &(exp.c_str()[pos]);
                    char* endp;
                    double value = strtod( startp, &endp );
                    m_exp_token[etp].m_token = VALUE;
                    m_exp_token[etp].m_value = value;
                    etp++;
                    unsigned int diff = endp - startp;
                    //std::cout<<value<<std::endl;
                    pos+=diff;
                    detection=true;
                }
                //演算子を検出
                for( unsigned int i = 0; i < ope_str.size(); i++ )
                {
                    if( exp.compare( pos, 1, ope_str[i].m_str ) == 0 )
                    {
                        if( ope_str[i].m_token == MINUS ){//単項演算子のマイナスを検出
                            if( pos == 0) m_exp_token[etp].m_token = MINUSF;
                            else if( pos != 0 && exp.compare( pos-1, 1, "(" ) == 0 )
                                m_exp_token[etp].m_token = MINUSF;
                            else m_exp_token[etp].m_token = MINUS;
                        }
                        else  m_exp_token[etp].m_token = ope_str[i].m_token;
                        etp++;
                        //std::cout<<ope_str[i].m_str<<std::endl;
                        pos++;
                        detection=true;
                    }
                }
                //関数を検出
                for( unsigned int i = 0; i < func_str.size(); i++ )
                {
                    if( exp.compare( pos, func_str[i].m_str.size(), func_str[i].m_str ) == 0 )
                    {
                        m_exp_token[etp].m_token = func_str[i].m_token;
                        etp++;
                        //std::cout<<func_str[i].m_str<<std::endl;
                        pos += func_str[i].m_str.size();
                        detection=true;
                    }
                }
                //変数を検出
                for( unsigned int i = 0; i < var_str.size(); i++ )
                {
                    if( exp.compare( pos, var_str[i].m_str.size(), var_str[i].m_str ) == 0 )
                    {
                        m_exp_token[etp].m_token = VARIABLE;
                        m_exp_token[etp].m_var = var_str[i].m_var;
                        etp++;
                        //std::cout<<var_str[i].m_str<<std::endl;
                        pos += var_str[i].m_str.size();
                        detection=true;
                    }
                }

                if(detection == false)
                {
                    std::cout<<"Error String in Expression\n";
                    pos++;
                }
            }
            m_exp_token[etp].m_token = END;
        }
};

} // end of namespace FuncParser

#endif

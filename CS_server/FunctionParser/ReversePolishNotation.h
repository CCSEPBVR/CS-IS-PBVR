#ifndef _REVERSEPOLISHNOTATION
#define _REVERSEPOLISHNOTATION

#include <iostream>
#include <math.h>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "Token.h"

#define MAXVAL 128
#define NUMVAR 128 //変数の数と一致すること

namespace FuncParser
{

class ReversePolishNotation
{
public:
    //逆ポーランド記法のデータ構造
    int   *m_exp_token;
    int   *m_variable_name;
    float *m_variable_value;
// add FJ start
    float *m_variable_value_array[NUMVAR];
// add FJ  end
    float *m_number;

    //スタックとポインタ
    int m_sp;
    float* m_stack;
// add FJ start
    float *m_stack_array;
// add FJ  end
    
// add FJ start
private:
    // 配列サイズ
    int m_array_size;
// add FJ  end

public:
    ReversePolishNotation( void );

    ReversePolishNotation( int size )
    {
        m_exp_token = new int[size];
        m_variable_name = new int[size];
        m_variable_value = new float[NUMVAR];//変数の数だけ必要
        m_number = new float[size];
        m_stack = new float[size];
    }

    ~ReversePolishNotation( void )
        {
            //delete[] m_exp_token;
            //delete[] m_variable_name;
            //delete[] m_variable_value;
            //delete[] m_number;XVAL
            delete[] m_stack;
        }

public:
    void setExpToken( int* exp_token )
        {
            m_exp_token = exp_token;
        }

    void setVariableName( int* name )
        {
            m_variable_name = name;
        }

    void setNumber( float* num )
        {
            m_number = num;
        }

    void setVariableValue( float* value )
        {
            m_variable_value = value;
        }

    void setVariableValueArray( float* value[NUMVAR] )
        {
            for(int i=0; i<NUMVAR; i++){
                m_variable_value_array[i] = value[i];
            }
        }

    void setExpToken( int* exp_token, int size )
        {
            for( int i = 0; i < size; i++ )
            {
                m_exp_token[i] = exp_token[i];
            }
        }

    void setVariableName( int* name, int size )
        {
            for( int i = 0; i < size; i++ )
            {
                m_variable_name[i] = name[i];
            }
        }

    void setNumber( float* num, int size )
        {
            for( int i = 0; i < size; i++ )
            {
                m_number[i] = num[i];
            }
        }

    void setVariableValue( float* value, int size )
        {
            for( int i = 0; i < NUMVAR; i++ )
            {
                m_variable_value[i] = value[i];
            }
        }

    void show( void )
    {
        int p=0;
        std::cout<<"exp token: ";
        for(int i=0; i<5; i++)
        {
            std::cout<<m_exp_token[i]<<",";
        }
        std::cout<<std::endl;

        std::cout<<"var name: ";
        for(int i=0; i<5; i++)
        {
            std::cout<<m_variable_name[i]<<",";
        }
        std::cout<<std::endl;

        std::cout<<"val array: ";
        for(int i=0; i<5; i++)
        {
            std::cout<<m_number[i]<<",";
        }
        std::cout<<std::endl;
    }

public:
    float eval( void )
        {
            //this->show();

            m_sp = 0; //スタックポインタの初期化

            int*   expr = m_exp_token;
            int*   name = m_variable_name;
            float* value = m_variable_value;
            float* number = m_number;

            float tmp1, tmp2, tmp3;

            while( *expr != END )
            {
                //std::cout<<"while *expr="<<*expr<<std::endl;
                switch( *expr )
                {
                    //変数
                case VARIABLE:
                    push( value[*name] );
                    break;

                    //数値
                case VALUE:
                    push( *number );
                    break;

                    //二項演算子
                case PLUS:
                    tmp1 = pop();
                    tmp2 = pop();
                    tmp3 = tmp2 + tmp1;
                    push( tmp3 );
                    break;
                case MINUS:
                    tmp1 = pop();
                    tmp2 = pop();
                    tmp3 = tmp2 - tmp1;
                    push( tmp3 );
                    break;
                case MULTI:
                    tmp1 = pop();
                    tmp2 = pop();
                    tmp3 = tmp2 * tmp1;
                    push( tmp3 );
                    break;
                case DIVIDE:
                    tmp1 = pop();
                    tmp2 = pop();
                    tmp3 = tmp2 / tmp1;
                    push( tmp3 );
                    break;
                case POW:
                    tmp1 = pop();
                    tmp2 = pop();
                    tmp3 = pow( tmp2, tmp1 );
                    push( tmp3 );
                    break;

                    //単項演算子としてのマイナス
                case MINUSF:
                    tmp1 = pop();
                    tmp2 = -tmp1;
                    push( tmp2 );
                    break;

                    //単項関数
                case SQRT:
                    tmp1 = pop();
                    tmp2 = sqrt( tmp1 );
                    push( tmp2 );
                    break;
                case CBRT:
                    tmp1 = pop();
                    tmp2 = cbrt( tmp1 );
                    push( tmp2 );
                    break;
                case LOG:
                    tmp1 = pop();
                    tmp2 = log( tmp1 );
                    push( tmp2 );
                    break;
                case LOG10:
                    tmp1 = pop();
                    tmp2 = log10( tmp1 );
                    push( tmp2 );
                    break;
                case EXP:
                    tmp1 = pop();
                    tmp2 = exp( tmp1 );
                    push( tmp2 );
                    break;
                case ABS:
                    tmp1 = pop();
                    tmp2 = fabs( tmp1 );
                    push( tmp2 );
                    break;
                case FLOOR:
                    tmp1 = pop();
                    tmp2 = floor( tmp1 );
                    push( tmp2 );
                    break;
                case CEIL:
                    tmp1 = pop();
                    tmp2 = ceil( tmp1 );
                    push( tmp2 );
                    break;
                case SIN:
                    tmp1 = pop();
                    tmp2 = sin( tmp1 );
                    push( tmp2 );
                    break;
                case COS:
                    tmp1 = pop();
                    tmp2 = cos( tmp1 );
                    push( tmp2 );
                    break;
                case TAN:
                    tmp1 = pop();
                    tmp2 = tan( tmp1 );
                    push( tmp2 );
                    break;
                case ASIN:
                    tmp1 = pop();
                    tmp2 = asin( tmp1 );
                    push( tmp2 );
                    break;
                case ACOS:
                    tmp1 = pop();
                    tmp2 = acos( tmp1 );
                    push( tmp2 );
                    break;
                case ATAN:
                    tmp1 = pop();
                    tmp2 = atan( tmp1 );
                    push( tmp2 );
                    break;
                case SINH:
                    tmp1 = pop();
                    tmp2 = sinh( tmp1 );
                    push( tmp2 );
                    break;
                case COSH:
                    tmp1 = pop();
                    tmp2 = cosh( tmp1 );
                    push( tmp2 );
                    break;
                case TANH:
                    tmp1 = pop();
                    tmp2 = tanh( tmp1 );
                    push( tmp2 );
                    break;
                case ASINH:
                    tmp1 = pop();
                    tmp2 = asinh( tmp1 );
                    push( tmp2 );
                    break;
                case ACOSH:
                    tmp1 = pop();
                    tmp2 = acosh( tmp1 );
                    push( tmp2 );
                    break;
                case ATANH:
                    tmp1 = pop();
                    tmp2 = atanh( tmp1 );
                    push( tmp2 );
                    break;
                case HEAVI:
                    tmp1 = pop();
                    tmp2 = tmp1<=0 ? 0 : 1;
                    push( tmp2 );
                    break;
                case RECTFUNC:
                    tmp1 = pop();
                    tmp2 = fabs(tmp1)>0.5 ? 0 : 1;
                    push( tmp2 );
                    break;

                    //二項関数
                case SIGMOID:    // 1/(1+e^ax)
                    tmp1 = pop();//a
                    tmp2 = pop();//x
                    tmp3 = 1.0/(1.0+exp(tmp1*tmp2));
                    push( tmp3 );
                    break;
                case GAUSS:       //e^( -(x)^2 / 2s )
                    tmp1 = pop();//s
                    tmp2 = pop();//x
                    tmp3 = exp(-tmp2*tmp2/(2*tmp1) );
                    push( tmp3 );
                    break;

                case MIN:       //min (a, b)
                    tmp1 = pop();//a
                    tmp2 = pop();//b
                    tmp3 = ( ( tmp1 < tmp2 ) ? tmp1 : tmp2 );
                    push( tmp3 );
                    break;

                case MAX:       //max (a, b)
                    tmp1 = pop();//a
                    tmp2 = pop();//b
                    tmp3 = ( ( tmp1 > tmp2 ) ? tmp1 : tmp2 );
                    push( tmp3 );
                    break;

                    //カンマ
                case COMMA:
                    break;

                default:
                    std::cout<<"unknown token. ->"<<*expr<<std::endl;
                } // end of switch

                expr++;
                name++;
                number++;
            } //end of while

            tmp1 = pop();
            return tmp1;
        }

// add FJ start
    void evalArray( float *result, int array_size )
        {
            //this->show();
            /* スタック領域を確保 */
            //float stack_array[MAXVAL][array_size];
            //m_stack_array = &stack_array[0][0];
            m_stack_array = new float[MAXVAL * array_size];
            m_array_size = array_size; /* スタックの配列の要素数を設定 */
            
            m_sp = 0; //スタックポインタの初期化

            int*   expr = m_exp_token;
            int*   name = m_variable_name;
            float** value = m_variable_value_array;
            float* number = m_number;

            //float tmp1[array_size], tmp2[array_size], tmp3[array_size];
			float *tmp1, *tmp2, *tmp3;
			tmp1 = new float[array_size];
			tmp2 = new float[array_size];
			tmp3 = new float[array_size];
            
            while( *expr != END )
            {
                //std::cout<<"while *expr="<<*expr<<std::endl;
                switch( *expr )
                {
                    //変数
                case VARIABLE:
                    pushArray( value[*name] );
                    break;

                    //数値
                case VALUE:
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp1[i] = *number;
                    }
                    pushArray( tmp1 );
                    break;

                    //二項演算子
                case PLUS:
                    popArray( tmp1 );
                    popArray( tmp2 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = tmp2[i] + tmp1[i];
                    }
                    pushArray( tmp3 );
                    break;
                case MINUS:
                    popArray( tmp1 );
                    popArray( tmp2 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = tmp2[i] - tmp1[i];
                    }
                    pushArray( tmp3 );
                    break;
                case MULTI:
                    popArray( tmp1 );
                    popArray( tmp2 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = tmp2[i] * tmp1[i];
                    }
                    pushArray( tmp3 );
                    break;
                case DIVIDE:
                    popArray( tmp1 );
                    popArray( tmp2 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = tmp2[i] / tmp1[i];
                    }
                    pushArray( tmp3 );
                    break;
                case POW:
                    popArray( tmp1 );
                    popArray( tmp2 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = pow( tmp2[i], tmp1[i] );
                    }
                    pushArray( tmp3 );
                    break;

                    //単項演算子としてのマイナス
                case MINUSF:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = -tmp1[i];
                    }
                    pushArray( tmp2 );
                    break;

                    //単項関数
                case SQRT:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = sqrt( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case CBRT:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = cbrt( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case LOG:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = log( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case LOG10:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = log10( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case EXP:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = exp( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ABS:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = fabs( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case FLOOR:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = floor( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case CEIL:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = ceil( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case SIN:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = sin( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case COS:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = cos( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case TAN:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = tan( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ASIN:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = asin( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ACOS:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = acos( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ATAN:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = atan( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case SINH:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = sinh( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case COSH:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = cosh( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case TANH:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = tanh( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ASINH:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = asinh( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ACOSH:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = acosh( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case ATANH:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = atanh( tmp1[i] );
                    }
                    pushArray( tmp2 );
                    break;
                case HEAVI:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = tmp1[i]<=0 ? 0 : 1;
                    }
                    pushArray( tmp2 );
                    break;
                case RECTFUNC:
                    popArray( tmp1 );
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp2[i] = fabs(tmp1[i])>0.5 ? 0 : 1;
                    }
                    pushArray( tmp2 );
                    break;

                    //二項関数
                case SIGMOID:    // 1/(1+e^ax)
                    popArray( tmp1 );//a
                    popArray( tmp2 );//x
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = 1.0/(1.0+exp(tmp1[i]*tmp2[i]));
                    }
                    pushArray( tmp3 );
                    break;
                case GAUSS:       //e^( -(x)^2 / 2s )
                    popArray( tmp1 );//s
                    popArray( tmp2 );//x
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = exp(-tmp2[i]*tmp2[i]/(2*tmp1[i]) );
                    }
                    pushArray( tmp3 );
                    break;

                case MIN:       //min (a, b)
                    popArray( tmp1 );//s
                    popArray( tmp2 );//x
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = ( ( tmp1[i] < tmp2[i] ) ? tmp1[i] : tmp2[i] );
                    }
                    pushArray( tmp3 );
                    break;

                case MAX:       //max (a, b)
                    popArray( tmp1 );//s
                    popArray( tmp2 );//x
                    for ( int i=0; i<array_size; i++ )
                    {
                        tmp3[i] = ( ( tmp1[i] > tmp2[i] ) ? tmp1[i] : tmp2[i] );
                    }
                    pushArray( tmp3 );
                    break;

                    //カンマ
                case COMMA:
                    break;

                default:
                    std::cout<<"unknown token. ->"<<*expr<<std::endl;
                } // end of switch

                expr++;
                name++;
                number++;
            } //end of while

            popArray(tmp1);
            for(int i=0; i<array_size; i++){
                result[i] = tmp1[i];
            }
            
			delete[] tmp1;
			delete[] tmp2;
			delete[] tmp3;
			delete[] m_stack_array;

            return;
        }
// add FJ  end

protected:
    /* push: fを値スタックにプッシュする*/
    void push( float f )
        {
            if ( m_sp < MAXVAL )
            {
                m_stack[m_sp++] = f; // stack[sp]=f; sp++; と同じ
            }
            else
            {
                printf("error: stack full, can't push %g\n", f);
            }
        }

    /* pop: スタックをポップし，一番上の値を返す*/
    float pop( void )
        {
            if ( m_sp > 0 )
            {
                return m_stack[--m_sp]; // sp--; return stack[sp]; と同じ
            }
            else
            {
		printf("error: stack empty\n");
		return 0.0;
            }
        }

// add FJ start
    /* push: fを値スタックにプッシュする*/
    void pushArray( float *f )
    {
        if ( m_sp < MAXVAL )
        {
            for ( int i=0; i<m_array_size; i++ )
            {
                m_stack_array[m_sp*m_array_size+i] = f[i];
            }
            m_sp++;
        }
        else
            {
                printf("error: stack full, can't push %g\n", f);
            }
    }

    /* pop: スタックをポップし，一番上の値を返す*/
    void popArray( float *f )
    {
        if ( m_sp > 0 )
        {
            --m_sp;
            for ( int i=0; i<m_array_size; i++ )
            {
                f[i] = m_stack_array[m_sp*m_array_size+i];
            }
        }
        else
        {
            printf("error: stack empty\n");
        }
    }
// add FJ  end
};

} // end of namespace FuncParse

#endif

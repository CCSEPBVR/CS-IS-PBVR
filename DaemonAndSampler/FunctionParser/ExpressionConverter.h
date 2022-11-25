#ifndef _EXPRESSION_CONVERTER
#define _EXPRESSION_CONVERTER

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include "Token.h"
#include "ExpressionTokenizer.h"

#define NODE_ARRAY_SIZE 256

namespace FuncParser
{

class ExpressionConverter
{
public:
    Node* nodes;
    Node* root;
    int   nb_node_used;

    std::vector<int> token_array;
    std::vector<int> var_array;
    std::vector<float> value_array;

public:
    ExpressionConverter( void );
    // {
    //        nodes = new Node[NODE_ARRAY_SIZE];
    //    }

    ~ExpressionConverter()
        {
            delete [] nodes;
        }

    // setExpression()とconvertExpression()を繰り返して使用できる
    void setExpToken( ExpToken* exp_token )
        {
            token_array.clear();
            var_array.clear();
            value_array.clear();

            nb_node_used = 0;
            root = this->create_node(); //nodes[0]へのポインタ

            //nodes[0].exp[]に数式のトークン配列を入力
            for( int i = 0; i< TOKEN_ARRAY_SIZE; i++ )
            {
                root->m_exp[i] = exp_token[i];
            }
        }

    void convertExpToken( void )
        {
            if (this->parse_expression( root ) < 0) {
                fprintf(stderr, "parser error\n");
                return;
            }

            //std::cout<<"reverse polish notation: ";
            this->traverse_tree_postorder( root );
            //std::cout<<std::endl;

            token_array.push_back( static_cast<int>( END ) );
            var_array.push_back( 0 );
            value_array.push_back( 0.0 );

            //std::cout<<"infix notation: ";
            //this->traverse_tree_inorder( root );
            //std::cout<<std::endl;
        }

    void convertExpToken( ExpToken* exp_token )
        {
            this->setExpToken( exp_token );
            this->convertExpToken();
        }

protected:
    //配列nodesを二分木として扱う
    Node* create_node()
        {
            if (nb_node_used == NODE_ARRAY_SIZE)
                return NULL;
            else
                return &nodes[nb_node_used++];
        }


    //最外の小括弧()を取り除く
    int remove_bracket( Node* node )
        {
            int end_pos = node->exp_end_pos();
            int nest = 1;

            //最外の小括弧()の有無を判定。無ければ脱出
            if(  !( (node->m_exp[0].m_token == LEFT) && 
                    (node->m_exp[ end_pos-1 ].m_token == RIGHT) )  )
            {
                return 0;
            }

            //右括弧(と左括弧)の数が同じかどうかを判定
            for (int i = 1; i < end_pos-1; i++) {
                if (node->m_exp[i].m_token == LEFT)
                    nest++;
                else if (node->m_exp[i].m_token == RIGHT)
                    nest--;
                //式が(...)+(...)のようなとき、nestが0になる
                if (nest == 0)
                return 0;
            }

            if (nest != 1) {
                fprintf(stderr, "unbalanced bracket\n");
                return -1;
            }

            //最外の小括弧()を取り除く
            for ( int i = 0; i < end_pos - 2; i++) {
                node->m_exp[i] = node->m_exp[i + 1];
            }
            node->m_exp[end_pos-2].m_token = END;

            //二重、三重括弧の場合の、最外の括弧を取り除く
            if ( node->m_exp[0].m_token == LEFT )
                remove_bracket( node );

            return 0;
        }

    //最も優先度の低い演算子の位置を取得
    int get_pos_operator( Node* node )
        {
            int pos_op = -1;
            int nest = 0;//括弧カウント
            int priority=4;
            int priority_lowest = 5;//最低の優先度を保持

            if (!node)
                return -1;

            for (int i = 0; node->m_exp[i].m_token != END; i++)
            {
                switch ( node->m_exp[i].m_token )
                {
                case COMMA: priority = -1; break;
                case PLUS: priority = 0; break;
                case MINUS: priority = 0; break;
                case MINUSF: priority = 1; break;
                case MULTI: priority = 2; break;
                case DIVIDE: priority = 2; break;
                case POW: priority = 3; break;
                case SQRT: priority = 4; break;
                case CBRT: priority = 4; break;
                case LOG:  priority = 4; break;
                case LOG10:priority = 4; break;
                case EXP:  priority = 4; break;
                case ABS:  priority = 4; break;
                case FLOOR:priority = 4; break;
                case CEIL: priority = 4; break;
                case SIN:  priority = 4; break;
                case COS:  priority = 4; break;
                case TAN:  priority = 4; break;
                case ASIN: priority = 4; break;
                case ACOS: priority = 4; break;
                case ATAN: priority = 4; break;
                case SINH: priority = 4; break;
                case COSH: priority = 4; break;
                case TANH: priority = 4; break;
                case ASINH:priority = 4; break;
                case ACOSH:priority = 4; break;
                case ATANH:priority = 4; break;
                case HEAVI:priority = 4; break;
                case RECTFUNC: priority = 4; break;
                case SIGMOID:priority = 4; break;
                case GAUSS:priority = 4; break;
                case RIGHT: nest++; continue;
                case LEFT: nest--; continue;
                default: priority = 5; break;//上記に該当しない=変数
                }
                //括弧の中は優先度が高いので、priority_lowestの更新を飛ばす
                if ( nest == 0 ) {
                    if( priority <= priority_lowest) {
                        priority_lowest = priority;
                        pos_op = i;
                    }
                }
            }
            //括弧の中を除いて、演算子が存在しない場合、posi_op=-1は更新されない
            return pos_op;
        }

    int parse_expression( Node* node ) //最初の一回は、nodes[0]へのポインタ
        {
            //show_node( node );

            //エラー処理。意味のない最外の括弧を取り除く。
            if (remove_bracket(node) < 0)
                return -1;

            int pos_op;
            int pos_end;

            if (!node)
                return -1;

            //nodeの中の優先度の低い演算子(+や-)の位置を取得
            pos_op = get_pos_operator( node );

            //括弧の中を除いて、演算子が存在しない場合、処理を終了
            if (pos_op == -1) {
                node->m_left  = NULL;
                node->m_right = NULL;

                return 0;
            }

            pos_end = node->exp_end_pos();

            //node->leftにノードを追加（ノード配列の要素へのポインタを作成）
            node->m_left  = create_node();

            if (!node->m_left) {
                fprintf(stderr, "expression too long\n");
                return -1;
            }

            //node->left->exp に node->exp を先頭から pos_op 文字コピー
            for( int i = 0; i < pos_op; i++ )
            {
                node->m_left->m_exp[i] = node->m_exp[i];
            }
            node->m_left->m_exp[ pos_op ].m_token = END;

            //node->left->expの最外の括弧を取り除く
            if (remove_bracket(node->m_left) < 0)
                return -1;

            //node->left->expに対する分解処理を繰り返す
            if (parse_expression(node->m_left) < 0)
                return -1;

            //node->rightにノードを追加（ノード配列の要素へのポインタを作成）
            node->m_right = create_node();

            if (!node->m_right) {
                fprintf(stderr, "expression too long\n");
                return -1;
            }

            //node->right->exp に node->exp をpos_op+1ずらした先頭から pos_end-pos_op 文字コピー
            int i0 = 0;
            for( int i = pos_op + 1; i <= pos_end; i++,i0++ )
            {
                node->m_right->m_exp[i0]  = node->m_exp[i];
            }

            //node->right->expの最外の括弧を取り除く
            if (remove_bracket(node->m_right) < 0)
                return -1;

            //node->right->expに対する分解処理を繰り返す
            if (parse_expression(node->m_right) < 0)
                return -1;

            //演算子を文字列の先頭に入力
            node->m_exp[0] = node->m_exp[pos_op];
            node->m_exp[1].m_token = END;

            return 0;
        }

    void traverse_tree_postorder(Node* node)
        {
            if( node->m_left->m_exp[0].m_token != END )
                traverse_tree_postorder(node->m_left);
            if( node->m_right->m_exp[0].m_token != END )
                traverse_tree_postorder(node->m_right);

            //show( node->m_exp[0] );
            put_token( node->m_exp[0] );
        }

    void traverse_tree_inorder(Node* node)
        {
            if( node->m_left->m_exp[0].m_token != END && node->m_right->m_exp[0].m_token != END )
                printf("(");

            if( node->m_left->m_exp[0].m_token != END )
                traverse_tree_inorder(node->m_left);

            show( node->m_exp[0] );

            if( node->m_right->m_exp[0].m_token != END )
                traverse_tree_inorder(node->m_right);

            if( node->m_left->m_exp[0].m_token != END && node->m_right->m_exp[0].m_token != END )
                printf(")");
        }

    void traverse_tree_preorder(Node* node)
        {
            //show( node->m_exp[0] );

            if( node->m_left->m_exp[0].m_token != END )
                traverse_tree_preorder(node->m_left);

            if( node->m_right->m_exp[0].m_token != END )
                traverse_tree_preorder(node->m_right);
        }

    void put_token( ExpToken exp_token )
        {
            token_array.push_back( static_cast<int>( exp_token.m_token ) );
            var_array.push_back( static_cast<int>( exp_token.m_var ) );
            value_array.push_back( exp_token.m_value );
        }

    void show( ExpToken exp_token )
        {
            Token token = exp_token.m_token;

            switch ( token )
            {
            case PLUS: std::cout<<"+ "; break;
            case MINUS: std::cout<<"- "; break;
            case MINUSF: std::cout<<"-f "; break;
            case MULTI: std::cout<<"* "; break;
            case DIVIDE: std::cout<<"/ "; break;
            case POW: std::cout<<"^"; break;
            case SQRT: std::cout<<"sqrt "; break;
            case CBRT: std::cout<<"cbrt "; break;
            case LOG:  std::cout<<"log "; break;
            case LOG10:std::cout<<"log10 "; break;
            case EXP:  std::cout<<"exp "; break;
            case ABS:  std::cout<<"abs "; break;
            case FLOOR:std::cout<<"floor "; break;
            case CEIL: std::cout<<"ceil "; break;
            case SIN:  std::cout<<"sin "; break;
            case COS:  std::cout<<"cos "; break;
            case TAN:  std::cout<<"tan "; break;
            case ASIN: std::cout<<"asin "; break;
            case ACOS: std::cout<<"acos "; break;
            case ATAN: std::cout<<"atan "; break;
            case SINH: std::cout<<"sinh "; break;
            case COSH: std::cout<<"cosh "; break;
            case TANH: std::cout<<"tanh "; break;
            case ASINH:std::cout<<"asinh "; break;
            case ACOSH:std::cout<<"acosh "; break;
            case ATANH:std::cout<<"atanh "; break;
            case HEAVI:std::cout<<"heavi "; break;
            case RECTFUNC: std::cout<<"rect "; break;
            case SIGMOID:std::cout<<"sigmoid "; break;
            case GAUSS:std::cout<<"gauss "; break;
            case VALUE: std::cout<<exp_token.m_value<<" "; break;
            case VARIABLE: std::cout<<"q"<<exp_token.m_var<<" ";break;
            case LEFT:std::cout<<"(";break;
            case RIGHT:std::cout<<")";break;
            case COMMA:std::cout<<",";break;
            case END: std::cout<<" end ";break;
            default: std::cout<<" other "; break;
            }
        }


    void show_node( Node* node )
        {
            std::cout<<"show_node: ";
            for (int i = 0; node->m_exp[i].m_token != END; i++)
            {
                show( node->m_exp[i] );
            }
            std::cout<<"\n";
        }
};

} // end of namespace FuncParser

#endif

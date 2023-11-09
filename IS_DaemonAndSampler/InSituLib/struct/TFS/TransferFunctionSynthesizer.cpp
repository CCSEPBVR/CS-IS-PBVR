#include "TransferFunctionSynthesizer.h"
#include <fstream>
#include <limits>

TransferFunctionSynthesizer::TransferFunctionSynthesizer():
    m_scalars(TF_COUNT)
{
    m_var_value = new float*[128];
    for(int i=0; i<128; i++) m_var_value[i] = new float[SIMDW];

    EquationToken opa_func = {
        {VARIABLE, VARIABLE, PLUS, END}, // A1+A2 -> A1 A2 +
        {A1, A2},// A1, A2
        {} //nothing
    };

    m_opa_func = opa_func;

    EquationToken col_func = {
        {VARIABLE, VARIABLE, PLUS, END},// C1+C2 -> C1 C2 +
        {C1, C2},// C1, C2
        {}//nothing
    };

    m_col_func = col_func;

    EquationToken opa_var_1 = {
        {VARIABLE, VALUE, POW, VARIABLE, VALUE, POW, PLUS, SQRT, END}, //sqrt(q1^2+q2^2)->q1 2 ^ q2 2 ^ + sqrt
        {Q1,NOTHING,NOTHING,Q2},//q1,,,q2,,,
        {0,2,0,0,2}//,2,,2,
    };

    EquationToken opa_var_2 = {
        {VARIABLE, VARIABLE, MINUS, END}, //q1-q2 -> q1 q2 -
        {Q1, Q2},
        {}
    };

    m_opa_var.push_back( opa_var_1 );
    m_opa_var.push_back( opa_var_2 );

    EquationToken col_var_1 = {
        {VARIABLE, VALUE, POW, VARIABLE, VALUE, POW, PLUS, SQRT, END}, //sqrt(q1^2+q2^2)->q1 2 ^ q2 2 ^ + sqrt
        {Q1,NOTHING,NOTHING,Q2},//q1,,,q2,,,
        {0,2,0,0,2}//,2,,2,
    };

    EquationToken col_var_2 = {
        {VARIABLE, VARIABLE, MINUS, END}, //q1-q2 -> q1 q2 -
        {Q1, Q2},
        {}
    };

    m_col_var.push_back( col_var_1 );
    m_col_var.push_back( col_var_2 );
}

TransferFunctionSynthesizer::TransferFunctionSynthesizer( TransferFunctionSynthesizer& tfs ):
    m_scalars(TF_COUNT)
{
    m_var_value = new float*[128];
    for(int i=0; i<128; i++) m_var_value[i] = new float[SIMDW];

    this->setOpacityFunction( tfs.opacityFunction() );
    this->setColorFunction( tfs.colorFunction() );
    this->setOpacityVariable( tfs.opacityVariable() );
    this->setColorVariable( tfs.colorVariable() );
}

EquationToken TransferFunctionSynthesizer::opacityFunction( void )
{
    return m_opa_func;
}

EquationToken TransferFunctionSynthesizer::colorFunction( void )
{
    return m_col_func;
}

std::vector<EquationToken> TransferFunctionSynthesizer::opacityVariable( void )
{
    return m_opa_var;
}

std::vector<EquationToken> TransferFunctionSynthesizer::colorVariable( void )
{
    return m_col_var;
}

void TransferFunctionSynthesizer::setOpacityFunction( EquationToken opa_func )
{
    m_opa_func = opa_func;
}

void TransferFunctionSynthesizer::setColorFunction( EquationToken col_func )
{
    m_col_func = col_func;
}

void TransferFunctionSynthesizer::setOpacityVariable( std::vector<EquationToken> opa_var )
{
    m_opa_var = opa_var;
}

void TransferFunctionSynthesizer::setColorVariable( std::vector<EquationToken> col_var )
{
    m_col_var = col_var;
}

TransferFunctionSynthesizer::~TransferFunctionSynthesizer()
{
    for(int i=0; i<128; i++) delete[] m_var_value[i];
    delete[] m_var_value;
}

//kawamura
void TransferFunctionSynthesizer::SynthesizedOpacityScalars(
    std::vector< TFS::TrilinearInterpolator* > interp ,
    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
    float scalars[][SIMDW] )//resulting scalars[TF_COUNT][SIMDW]
{
    for( int I=0; I<SIMDW; I++)
    {
        m_var_value[X][I] = x_g[I];
        m_var_value[Y][I] = y_g[I];
        m_var_value[Z][I] = z_g[I];
    }

    const size_t nvar = interp.size();

    //各変数のループ
    //id of Q1=4, Q2=8,,,,, Qn=4*n
    //id of dxQ1=5, dyQ1=6, dzQ1=7,
    for( size_t j= 0; j < nvar; j++ )
    {
        float scalar[SIMDW];
        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];

        interp[j]->attachPoint( x_l, y_l, z_l );
        interp[j]->scalar( scalar );
        interp[j]->gradient( grad_x, grad_y, grad_z );

        for( int I=0; I<SIMDW; I++ )
        {
            m_var_value[4*(j+1)  ][I] = scalar[I];
            m_var_value[4*(j+1)+1][I] = grad_x[I];
            m_var_value[4*(j+1)+2][I] = grad_y[I];
            m_var_value[4*(j+1)+3][I] = grad_z[I];
        }
    }

    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value );
        //m_rpn.eval( &scalars[i][0] );
        m_rpn.setVariableValueArray( m_var_value );
        m_rpn.evalArray( &scalars[i][0], SIMDW );

    }
}

//kawamura
void TransferFunctionSynthesizer::SynthesizedColorScalars(
    std::vector< TFS::TrilinearInterpolator* > interp ,
    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
    float scalars[][SIMDW] )//resulting scalars[TF_COUNT][SIMDW]
{
    for( int I=0; I<SIMDW; I++)
    {
        m_var_value[X][I] = x_g[I];
        m_var_value[Y][I] = y_g[I];
        m_var_value[Z][I] = z_g[I];
    }

    const size_t nvar = interp.size();

    //各変数のループ
    //id of Q1=4, Q2=8,,,,, Qn=4*n
    //id of dxQ1=5, dyQ1=6, dzQ1=7,
    for( size_t j= 0; j < nvar; j++ )
    {
        float scalar[SIMDW];
        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];

        interp[j]->attachPoint( x_l, y_l, z_l );
        interp[j]->scalar( scalar );
        interp[j]->gradient( grad_x, grad_y, grad_z );

        for( int I=0; I<SIMDW; I++ )
        {
            m_var_value[4*(j+1)  ][I] = scalar[I];
            m_var_value[4*(j+1)+1][I] = grad_x[I];
            m_var_value[4*(j+1)+2][I] = grad_y[I];
            m_var_value[4*(j+1)+3][I] = grad_z[I];
        }
    }


    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value );
        //m_rpn.eval( &scalars[i][0] );
        m_rpn.setVariableValueArray( m_var_value );
        m_rpn.evalArray( &scalars[i][0], SIMDW );
    }
}

//kawamura
void TransferFunctionSynthesizer::CalculateOpacity(
    std::vector< TFS::TrilinearInterpolator* > interp,
    std::vector<pbvr::TransferFunction>& tf,
    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
    float* opacity )//resulting opacity[SIMDW]
{
    for( int I=0; I<SIMDW; I++)
    {
        m_var_value[X][I] = x_g[I];
        m_var_value[Y][I] = y_g[I];
        m_var_value[Z][I] = z_g[I];
    }

    const size_t nvar = interp.size();

    //各変数のループ
    //id of Q1=4, Q2=8,,,,, Qn=4*n
    for( size_t j= 0; j < nvar; j++ )
    {
        float scalar[SIMDW];
        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];

        interp[j]->attachPoint( x_l, y_l, z_l );
        interp[j]->scalar( scalar );
        interp[j]->gradient( grad_x, grad_y, grad_z );

        for( int I=0; I<SIMDW; I++ )
        {
            m_var_value[4*(j+1)  ][I] = scalar[I];
            m_var_value[4*(j+1)+1][I] = grad_x[I];
            m_var_value[4*(j+1)+2][I] = grad_y[I];
            m_var_value[4*(j+1)+3][I] = grad_z[I];
        }
    }

    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value );
        m_rpn.setVariableValueArray( m_var_value );

        //calc. m_opa_var
        float scalars[SIMDW];
        //m_rpn.eval( &scalars[0] );
        m_rpn.evalArray( &scalars[0], SIMDW );

        //set opacity A1,A2,,,Ai. start 116 in VarName(Token.h)
        for( int I=0; I<SIMDW; I++ )
        {
            m_var_value[ VAR_OFFSET_A+i ][I] = tf[i].opacityMap().at( scalars[I] );
            //std::cout<<m_var_value[ VAR_OFFSET_A+i ] <<",";
        }
    }
    //std::cout<<std::endl;

    //set opacity function eq. ex) A1*A2+A3
    m_rpn.setExpToken( &(m_opa_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_opa_func.var_name[0]) );
    m_rpn.setNumber( &(m_opa_func.val_array[0]) );
    //m_rpn.setVariableValue( m_var_value );
    m_rpn.setVariableValueArray( m_var_value );

    //calc. opacity function
    //m_rpn.eval( opacity );
    m_rpn.evalArray( opacity, SIMDW );

    for( int I=0; I<SIMDW; I++ )
    {
        kvs::Math::Clamp<float>( opacity[I], 0.0, 1.0 );
    }
}

//kawamura
void TransferFunctionSynthesizer::CalculateColor(
    std::vector< TFS::TrilinearInterpolator* > interp,
    std::vector<pbvr::TransferFunction>& tf,
    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
    kvs::UInt8* Red, kvs::UInt8* Green, kvs::UInt8* Blue )//resulting colors[SIMDW]
{
    for( int I=0; I<SIMDW; I++)
    {
        m_var_value[X][I] = x_g[I];
        m_var_value[Y][I] = y_g[I];
        m_var_value[Z][I] = z_g[I];
    }

    const size_t nvar = interp.size();

    //各変数のループ
    //id of Q1=4, Q2=8,,,,, Qn=4*n
    for( size_t j= 0; j < nvar; j++ )
    {
        float scalar[SIMDW];
        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];

        interp[j]->attachPoint( x_l, y_l, z_l );
        interp[j]->scalar( scalar );
        interp[j]->gradient( grad_x, grad_y, grad_z );

        for( int I=0; I<SIMDW; I++ )
        {
            m_var_value[4*(j+1)  ][I] = scalar[I];
            m_var_value[4*(j+1)+1][I] = grad_x[I];
            m_var_value[4*(j+1)+2][I] = grad_y[I];
            m_var_value[4*(j+1)+3][I] = grad_z[I];
        }
    }

    kvs::RGBColor colors[m_col_var.size()][SIMDW]; //result of t_func.colorMap().at( m_scalars[i] );

    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value );
        m_rpn.setVariableValueArray( m_var_value );

        //calc. m_col_var
        float scalars[SIMDW];
        //m_rpn.eval( &scalars[0] );
        m_rpn.evalArray( &scalars[0], SIMDW );
        //if(i==0) std::cout<<"local_coord="<<local_coord<<", ";
        //if(i==0) std::cout<<"m_var_value="<<m_var_value[4]<<", ";
        //if(i==0) std::cout<<"TFS:col:scalars="<<m_scalars[i]<<std::endl;

        for( int I=0; I<SIMDW; I++ )
        {
            colors[i][I] = tf[i].colorMap().at( scalars[I] );
        }
    }

    //std::cout<<"TFS:RGB="<<(int)colors[0].r()<<","<<(int)colors[0].g()<<","<<(int)colors[0].b()<<std::endl;

    //set color function eq. ex) C1*C2+C3
    m_rpn.setExpToken( &(m_col_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_col_func.var_name[0]) );
    m_rpn.setNumber( &(m_col_func.val_array[0]) );

    //RED
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        for( int I=0; I<SIMDW; I++ )
        {
            float col = (float)colors[i][I].r() / 255.0;
            //if(i==0)std::cout<<"colR="<<col<<std::endl;
            m_var_value[ VAR_OFFSET_C+i ][I] = col;
        }
    }
    //m_rpn.setVariableValue( m_var_value );
    m_rpn.setVariableValueArray( m_var_value );
    float red[SIMDW];
    //m_rpn.eval( &red[0]  );
    m_rpn.evalArray( &red[0], SIMDW );

    for( int I=0; I<SIMDW; I++ )
    {
        red[I] = red[I]<0 ? 0 : red[I]>1.0 ? 1 : red[I];
    }

    //GREEN
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        for( int I=0; I<SIMDW; I++ )
        {
            float col = (float)colors[i][I].g() / 255.0;
            //if(i==0)std::cout<<"colG="<<col<<std::endl;
            m_var_value[ VAR_OFFSET_C+i ][I] = col;
        }
    }
    //m_rpn.setVariableValue( m_var_value );
    m_rpn.setVariableValueArray( m_var_value );
    float green[SIMDW];
    //m_rpn.eval( &green[0] );
    m_rpn.evalArray( &green[0], SIMDW );

    for( int I=0; I<SIMDW; I++ )
    {
        green[I] = green[I]<0 ? 0 : green[I]>1.0 ? 1 : green[I];
    }


    //BLUE
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        for( int I=0; I<SIMDW; I++ )
        {
            float col = (float)colors[i][I].b() / 255.0;
            //if(i==0)std::cout<<"colB="<<col<<std::endl;
            m_var_value[ VAR_OFFSET_C+i ][I] = col;
        }
    }
    //m_rpn.setVariableValue( m_var_value );
    m_rpn.setVariableValueArray( m_var_value );
    float blue[SIMDW];
    //m_rpn.eval( &blue[0] );
    m_rpn.evalArray( &blue[0], SIMDW );

    for( int I=0; I<SIMDW; I++ )
    {
        blue[I] = blue[I]<0 ? 0 : blue[I]>1.0 ? 1 : blue[I];
    }
     
    for( int I=0; I<SIMDW; I++ )
    {
        Red[I] = (kvs::UInt8)(red[I]*255);
        Green[I] = (kvs::UInt8)(green[I]*255);
        Blue[I] = (kvs::UInt8)(blue[I]*255);
    }
}

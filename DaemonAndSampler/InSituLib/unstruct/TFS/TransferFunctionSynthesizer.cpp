#include "TransferFunctionSynthesizer.h"
#include <fstream>
#include <limits>

TransferFunctionSynthesizer::TransferFunctionSynthesizer():
    m_scalars(TF_COUNT)
{
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
}

//kawamura
std::vector<float> TransferFunctionSynthesizer::SynthesizedOpacityScalars(
    std::vector< pbvr::HexahedralCell<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord )
{
    m_var_value[X] = global_coord.x();
    m_var_value[Y] = global_coord.y();
    m_var_value[Z] = global_coord.z();

    m_scalars.clear();

    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );

        size_t nvar = interp.size();

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        for( size_t j= 0; j < nvar; j++ )
        {
            interp[j]->setLocalPoint( local_coord );
            const kvs::Vector3f grad = interp[j]->gradient();
            m_var_value[4*(j+1)  ] = interp[j]->scalar();
            m_var_value[4*(j+1)+1] = grad.x();
            m_var_value[4*(j+1)+2] = grad.y();
            m_var_value[4*(j+1)+3] = grad.z();
        }

        m_rpn.setVariableValue( &m_var_value[0] );

        //m_scalars[i] = m_rpn.eval();
        m_scalars.push_back( m_rpn.eval() );
    }

    return  m_scalars;
}

// Fj add
void TransferFunctionSynthesizer::SynthesizedOpacityScalarsArray(
    std::vector< pbvr::HexahedralCell<Type>* > interp ,
    const int loop_cnt,
    const kvs::Vector3f *local_coord,
    const kvs::Vector3f *global_coord,
    std::vector<float> *o_scalars_array )
{
    //配列を追加
    float scalar_array[interp.size()][loop_cnt];

    float grad_array_x[interp.size()][loop_cnt];
    float grad_array_y[interp.size()][loop_cnt];
    float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];
    float global_coord_x[loop_cnt];
    float global_coord_y[loop_cnt];
    float global_coord_z[loop_cnt];
    float eval_result[m_opa_var.size()][loop_cnt];

    for (int i = 0; i < loop_cnt; i++ )
    {
        //local_coord_x[i] = local_coord[i].x();
        //local_coord_y[i] = local_coord[i].y();
        //local_coord_z[i] = local_coord[i].z();
        global_coord_x[i] = global_coord[i].x();
        global_coord_y[i] = global_coord[i].y();
        global_coord_z[i] = global_coord[i].z();
    }

    size_t nvar = interp.size();

    for (size_t j = 0; j < nvar; j++ )
    {
        interp[j]->setLocalPointArray( loop_cnt,
                                       local_coord );
        interp[j]->CalcScalarGrad( loop_cnt,
                                   scalar_array[j],
                                   grad_array_x[j],
                                   grad_array_y[j],
                                   grad_array_z[j] );
    }

    m_var_value_array[X] = global_coord_x;
    m_var_value_array[Y] = global_coord_y;
    m_var_value_array[Z] = global_coord_z;

    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        for( size_t j= 0; j < nvar; j++ )
        {
            m_var_value_array[4*(j+1)  ] = &scalar_array[j][0];
            m_var_value_array[4*(j+1)+1] = &grad_array_x[j][0];
            m_var_value_array[4*(j+1)+2] = &grad_array_y[j][0];
            m_var_value_array[4*(j+1)+3] = &grad_array_z[j][0];
        }

        m_rpn.setVariableValueArray( m_var_value_array );

        m_rpn.evalArray(&eval_result[i][0], loop_cnt);
    }

    for( size_t i=0; i<loop_cnt; i++)
    {
        for( size_t j=0; j<m_opa_var.size(); j++)
        {
            o_scalars_array[i][j] = eval_result[j][i];
        }
    }

}

//kawamura
std::vector<float> TransferFunctionSynthesizer::SynthesizedColorScalars(
    std::vector< pbvr::HexahedralCell<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord )
{
    m_var_value[X] = global_coord.x();
    m_var_value[Y] = global_coord.y();
    m_var_value[Z] = global_coord.z();

    m_scalars.clear();

    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );

        size_t nvar = interp.size();

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        //id of dxQ1=5, dyQ1=6, dzQ1=7,
        for( size_t j= 0; j < nvar; j++ )
        {
            interp[j]->setLocalPoint( local_coord );
            const kvs::Vector3f grad = interp[j]->gradient();
            m_var_value[4*(j+1)  ] = interp[j]->scalar();
            m_var_value[4*(j+1)+1] = grad.x();
            m_var_value[4*(j+1)+2] = grad.y();
            m_var_value[4*(j+1)+3] = grad.z();
        }

        m_rpn.setVariableValue( &m_var_value[0] );

        //m_scalars[i] = m_rpn.eval();
        m_scalars.push_back( m_rpn.eval() );
    }

    return m_scalars;
}

// Fj add
void TransferFunctionSynthesizer::SynthesizedColorScalarsArray(
    std::vector< pbvr::HexahedralCell<Type>* > interp,
    const int loop_cnt,
    const kvs::Vector3f *local_coord,
    const kvs::Vector3f *global_coord,
    std::vector<float> *c_scalars_array )
{

    //配列を追加
    float scalar_array[interp.size()][loop_cnt];

    float grad_array_x[interp.size()][loop_cnt];
    float grad_array_y[interp.size()][loop_cnt];
    float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];
    float global_coord_x[loop_cnt];
    float global_coord_y[loop_cnt];
    float global_coord_z[loop_cnt];
    float eval_result[m_col_var.size()][loop_cnt];

    for ( int i = 0; i < loop_cnt; i++ )
    {
        //local_coord_x[i] = local_coord[i].x();
        //local_coord_y[i] = local_coord[i].y();
        //local_coord_z[i] = local_coord[i].z();
        global_coord_x[i] = global_coord[i].x();
        global_coord_y[i] = global_coord[i].y();
        global_coord_z[i] = global_coord[i].z();
    }

    size_t nvar = interp.size();

    for (size_t j = 0; j < nvar; j++ )
    {
        interp[j]->setLocalPointArray( loop_cnt,
                                       local_coord );
        interp[j]->CalcScalarGrad( loop_cnt,
                                   scalar_array[j],
                                   grad_array_x[j],
                                   grad_array_y[j],
                                   grad_array_z[j] );
    }

    m_var_value_array[X] = global_coord_x;
    m_var_value_array[Y] = global_coord_y;
    m_var_value_array[Z] = global_coord_z;

    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        //id of dxQ1=5, dyQ1=6, dzQ1=7,
        for( size_t j= 0; j < nvar; j++ )
        {
            m_var_value_array[4*(j+1)  ] = &scalar_array[j][0];
            m_var_value_array[4*(j+1)+1] = &grad_array_x[j][0];
            m_var_value_array[4*(j+1)+2] = &grad_array_y[j][0];
            m_var_value_array[4*(j+1)+3] = &grad_array_z[j][0];
        }
     
        m_rpn.setVariableValueArray( m_var_value_array );

        m_rpn.evalArray(&eval_result[i][0], loop_cnt);
    }

    for( size_t i=0; i<loop_cnt; i++)
    {
        for( size_t j=0; j<m_col_var.size(); j++)
        {
            c_scalars_array[i][j] = eval_result[j][i];
        }
    }

}

//kawamura
float TransferFunctionSynthesizer::CalculateOpacity(
    std::vector< pbvr::HexahedralCell<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord,
    std::vector<pbvr::TransferFunction>& tf)
{
    m_var_value[X] = global_coord.x();
    m_var_value[Y] = global_coord.y();
    m_var_value[Z] = global_coord.z();

    m_scalars.clear();

    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );

        size_t nvar = interp.size();

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        for( size_t j= 0; j < nvar; j++ )
        {
            interp[j]->setLocalPoint( local_coord );
            const kvs::Vector3f grad = interp[j]->gradient();
            m_var_value[4*(j+1)  ] = interp[j]->scalar();
            m_var_value[4*(j+1)+1] = grad.x();
            m_var_value[4*(j+1)+2] = grad.y();
            m_var_value[4*(j+1)+3] = grad.z();
        }

        m_rpn.setVariableValue( &m_var_value[0] );

        //calc. m_opa_var
        //m_scalars[i] = m_rpn.eval();
        m_scalars.push_back( m_rpn.eval() );

        //set opacity A1,A2,,,Ai. start 116 in VarName(Token.h)
        m_var_value[ VAR_OFFSET_A+i ] = tf[i].opacityMap().at( m_scalars[i] );
        //std::cout<<m_var_value[ VAR_OFFSET_A+i ] <<",";
    }
    //std::cout<<std::endl;

    //set opacity function eq. ex) A1*A2+A3
    m_rpn.setExpToken( &(m_opa_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_opa_func.var_name[0]) );
    m_rpn.setNumber( &(m_opa_func.val_array[0]) );
    m_rpn.setVariableValue( &m_var_value[0] );

    //calc. opacity function
    float opacity = m_rpn.eval();
    return kvs::Math::Clamp<float>( opacity, 0.0, 1.0 );
}

// Fj add

void TransferFunctionSynthesizer::CalculateOpacityArray(
    std::vector< pbvr::HexahedralCell<Type>* > interp , 
    const int loop_cnt,
    const kvs::Vector3f *local_coord, 
    const kvs::Vector3f *global_coord,
    std::vector<pbvr::TransferFunction>& tf,
    float *opacity_array)
{

    //配列を追加
    float scalar_array[interp.size()][loop_cnt];

    float grad_array_x[interp.size()][loop_cnt];
    float grad_array_y[interp.size()][loop_cnt];
    float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];

    float global_coord_x[loop_cnt];
    float global_coord_y[loop_cnt];
    float global_coord_z[loop_cnt];

    float eval_result[loop_cnt];
    float opacity_map_array[m_opa_var.size()][loop_cnt];

    for (int i = 0; i < loop_cnt; i++)
    {
        //local_coord_x[i] = local_coord[i].x();
        //local_coord_y[i] = local_coord[i].y();
        //local_coord_z[i] = local_coord[i].z();
        global_coord_x[i] = global_coord[i].x();
        global_coord_y[i] = global_coord[i].y();
        global_coord_z[i] = global_coord[i].z();
    }

    size_t nvar = interp.size();

    //bindCell, setLocalPoint, gradient, scalar をまとめてこの関数内部でSIMD化
    for( size_t j= 0; j < nvar; j++ )
    {
        interp[j]->setLocalPointArray( loop_cnt,
                                       local_coord );
        interp[j]->CalcScalarGrad( loop_cnt,
                                   scalar_array[j],
                                   grad_array_x[j],
                                   grad_array_y[j],
                                   grad_array_z[j] );
    }

    m_var_value_array[X] = global_coord_x;
    m_var_value_array[Y] = global_coord_y;
    m_var_value_array[Z] = global_coord_z;


    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );

        size_t nvar = interp.size();

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        for( size_t j= 0; j < nvar; j++ )
        {
            m_var_value_array[4*(j+1)  ] = &scalar_array[j][0];
            m_var_value_array[4*(j+1)+1] = &grad_array_x[j][0];
            m_var_value_array[4*(j+1)+2] = &grad_array_y[j][0];
            m_var_value_array[4*(j+1)+3] = &grad_array_z[j][0];
        }

        m_rpn.setVariableValueArray( m_var_value_array );

        //calc. m_opa_var
        m_rpn.evalArray(eval_result, loop_cnt);

        //set opacity A1,A2,,,Ai. start 116 in VarName(Token.h)
        //m_var_value[ VAR_OFFSET_A+i ] = tf[i].opacityMap().at( m_scalars[i] );
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            opacity_map_array[i][jx] = tf[i].opacityMap().at( eval_result[jx] );
        }
        m_var_value_array[ VAR_OFFSET_A+i ] = &opacity_map_array[i][0];

    }
    //std::cout<<std::endl;

    //set opacity function eq. ex) A1*A2+A3
    m_rpn.setExpToken( &(m_opa_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_opa_func.var_name[0]) );
    m_rpn.setNumber( &(m_opa_func.val_array[0]) );
    // 20190118
    //m_rpn.setVariableValue( &m_var_value[0] );
    m_rpn.setVariableValueArray( m_var_value_array );

    //calc. opacity function
    m_rpn.evalArray(eval_result, loop_cnt);

    for( int jx=0; jx<loop_cnt; jx++ )
    {
        opacity_array[jx] = kvs::Math::Clamp<float>( eval_result[jx], 0.0, 1.0 );
    }

}


//kawamura
kvs::RGBColor TransferFunctionSynthesizer::CalculateColor(
    std::vector< pbvr::HexahedralCell<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord,
    std::vector<pbvr::TransferFunction>& tf )
{
    kvs::RGBColor colors[10]; //result of t_func.colorMap().at( m_scalars[i] );

    m_var_value[X] = global_coord.x();
    m_var_value[Y] = global_coord.y();
    m_var_value[Z] = global_coord.z();

    m_scalars.clear();
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );

        size_t nvar = interp.size();

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        for( size_t j= 0; j < nvar; j++ )
        {
            interp[j]->setLocalPoint( local_coord );
            const kvs::Vector3f grad = interp[j]->gradient();
            m_var_value[4*(j+1)  ] = interp[j]->scalar();
            m_var_value[4*(j+1)+1] = grad.x();
            m_var_value[4*(j+1)+2] = grad.y();
            m_var_value[4*(j+1)+3] = grad.z();
        }

        m_rpn.setVariableValue( &m_var_value[0] );

        //calc. m_col_var
        //m_scalars[i] = m_rpn.eval();
        m_scalars.push_back( m_rpn.eval() );

        //if(i==0) std::cout<<"local_coord="<<local_coord<<", ";
        //if(i==0) std::cout<<"m_var_value="<<m_var_value[4]<<", ";
        //if(i==0) std::cout<<"TFS:col:scalars="<<m_scalars[i]<<std::endl;

        colors[i] = tf[i].colorMap().at( m_scalars[i] );
    }

    //std::cout<<"TFS:RGB="<<(int)colors[0].r()<<","<<(int)colors[0].g()<<","<<(int)colors[0].b()<<std::endl;

    //set color function eq. ex) C1*C2+C3
    m_rpn.setExpToken( &(m_col_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_col_func.var_name[0]) );
    m_rpn.setNumber( &(m_col_func.val_array[0]) );

    //RED
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        float col = (float)colors[i].r() / 255.0;
        //if(i==0)std::cout<<"colR="<<col<<std::endl;
        m_var_value[ VAR_OFFSET_C+i ] = col;
    }
    m_rpn.setVariableValue( &m_var_value[0] );
    float red = kvs::Math::Clamp<float>( m_rpn.eval(), 0.0, 1.0);

    //GREEN
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        float col = (float)colors[i].g() / 255.0;
        //if(i==0)std::cout<<"colG="<<col<<std::endl;
        m_var_value[ VAR_OFFSET_C+i ] = col;
    }
    m_rpn.setVariableValue( &m_var_value[0] );
    float green = kvs::Math::Clamp<float>( m_rpn.eval(), 0.0, 1.0);

    //BLUE
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        float col = (float)colors[i].b() / 255.0;
        //if(i==0)std::cout<<"colB="<<col<<std::endl;
        m_var_value[ VAR_OFFSET_C+i ] = col;
    }
    m_rpn.setVariableValue( &m_var_value[0] );
    float blue = kvs::Math::Clamp<float>( m_rpn.eval(), 0.0, 1.0);

    //std::cout<<"rgb="<<red<<","<<green<<","<<blue<<std::endl;

    return kvs::RGBColor(
        (kvs::UInt8)(red * 255),
        (kvs::UInt8)(green * 255),
        (kvs::UInt8)(blue * 255) );
}

void TransferFunctionSynthesizer::CalculateColorArray(
    std::vector< pbvr::HexahedralCell<Type>* > interp,
    const int loop_cnt,
    const kvs::Vector3f *local_coord,
    const kvs::Vector3f *global_coord,
    std::vector<pbvr::TransferFunction>& tf,
    kvs::RGBColor *color_seq)
{

    //配列を追加
    float scalar_array[interp.size()][loop_cnt];

    float grad_array_x[interp.size()][loop_cnt];
    float grad_array_y[interp.size()][loop_cnt];
    float grad_array_z[interp.size()][loop_cnt];

    float local_coord_x[loop_cnt];
    float local_coord_y[loop_cnt];
    float local_coord_z[loop_cnt];

    float global_coord_x[loop_cnt];
    float global_coord_y[loop_cnt];
    float global_coord_z[loop_cnt];

    float eval_result[loop_cnt];

    for (int jx = 0; jx < loop_cnt; jx++)
    {
        local_coord_x[jx] = local_coord[jx].x();
        local_coord_y[jx] = local_coord[jx].y();
        local_coord_z[jx] = local_coord[jx].z();
        global_coord_x[jx] = global_coord[jx].x();
        global_coord_y[jx] = global_coord[jx].y();
        global_coord_z[jx] = global_coord[jx].z();
    }

    size_t nvar = interp.size();

    for( size_t j= 0; j < nvar; j++ )
    {
        interp[j]->setLocalPointArray( loop_cnt,
                                       local_coord );
        interp[j]->CalcScalarGrad( loop_cnt,
                                   scalar_array[j],
                                   grad_array_x[j],
                                   grad_array_y[j],
                                   grad_array_z[j] );
    }

    kvs::RGBColor colors[10][loop_cnt]; //result of t_func.colorMap().at( m_scalars[i] );

    m_var_value_array[X] = global_coord_x;
    m_var_value_array[Y] = global_coord_y;
    m_var_value_array[Z] = global_coord_z;

    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        for( size_t j= 0; j < nvar; j++ )
        {
            m_var_value_array[4*(j+1)  ] = &scalar_array[j][0];
            m_var_value_array[4*(j+1)+1] = &grad_array_x[j][0];
            m_var_value_array[4*(j+1)+2] = &grad_array_y[j][0];
            m_var_value_array[4*(j+1)+3] = &grad_array_z[j][0];
        }

       m_rpn.setVariableValueArray( m_var_value_array );
       m_rpn.evalArray( eval_result, loop_cnt );

        for( int jx=0; jx<loop_cnt; jx++){
            colors[i][jx] = tf[i].colorMap().at( eval_result[jx] );
        }
    }

    m_rpn.setExpToken( &(m_col_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_col_func.var_name[0]) );
    m_rpn.setNumber( &(m_col_func.val_array[0]) );

    float color_array[m_col_var.size()][loop_cnt];
    float red_array[loop_cnt];
    float green_array[loop_cnt];
    float blue_array[loop_cnt];

    //RED
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        for( int jx=0; jx<loop_cnt; jx++ ){
            color_array[i][jx] = (float)colors[i][jx].r() / 255.0;
        }
        m_var_value_array[ VAR_OFFSET_C+i ] = &color_array[i][0];
    }
    m_rpn.setVariableValueArray( m_var_value_array );
    m_rpn.evalArray( red_array, loop_cnt );

    //GREEN
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        for( int jx=0; jx<loop_cnt; jx++ ){
            color_array[i][jx] = (float)colors[i][jx].g() / 255.0;
        }
        m_var_value_array[ VAR_OFFSET_C+i ] = &color_array[i][0];
    }
    m_rpn.setVariableValueArray( m_var_value_array );
    m_rpn.evalArray( green_array, loop_cnt );

    //BLUE
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        for( int jx=0; jx<loop_cnt; jx++ ){
            color_array[i][jx] = (float)colors[i][jx].b() / 255.0;
        }
        m_var_value_array[ VAR_OFFSET_C+i ] = &color_array[i][0];
    }
    m_rpn.setVariableValueArray( m_var_value_array );
    m_rpn.evalArray( blue_array, loop_cnt );

    //結果を配列に格納
    for( int jx=0; jx<loop_cnt; jx++ ){
        color_seq[jx].set(
            (kvs::UInt8)(red_array[jx] * 255),
            (kvs::UInt8)(green_array[jx] * 255),
            (kvs::UInt8)(blue_array[jx] * 255) );
    }

}

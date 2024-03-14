#include "TransferFunctionSynthesizer.h"
#include <fstream>
#include <limits>

#if (defined(KVS_PLATFORM_LINUX) || defined(KVS_PLATFORM_MACOSX))                                                                                                                   
#include <execinfo.h>
#endif


TransferFunctionSynthesizer::TransferFunctionSynthesizer():
    m_scalars(TF_COUNT)
{
    m_variable_range.clear();
    m_var_value_struct = new float*[128];
    for(int i=0; i<128; i++) m_var_value_struct[i] = new float[SIMDW];

//    EquationToken opa_func = {
//        {VARIABLE, VARIABLE, PLUS, END}, // A1+A2 -> A1 A2 +
//        {A1, A2},// A1, A2
//        {} //nothing
//    };
//
//    m_opa_func = opa_func;
//
//    EquationToken col_func = {
//        {VARIABLE, VARIABLE, PLUS, END},// C1+C2 -> C1 C2 +
//        {C1, C2},// C1, C2
//        {}//nothing
//    };
//
//    m_col_func = col_func;

    EquationToken opa_func = {
        {VARIABLE, END}, // A1+A2 -> A1 A2 +
        {A1},// A1, A2
        {} //nothing
    };

    m_opa_func = opa_func;

    EquationToken col_func = {
        {VARIABLE, END},// C1
        {C1},// C1
        {}//nothing
    };

    m_col_func = col_func;




    EquationToken opa_var_1 = {
        {VARIABLE, VALUE, POW, VARIABLE, VALUE, POW, PLUS, SQRT, END}, //sqrt(q1^2+q2^2)->q1 2 ^ q2 2 ^ + sqrt
        {Q1,NOTHING,NOTHING,Q2},//q1,,,q2,,,
        {0,2,0,0,2}//,2,,2,
    };

    EquationToken opa_var_2 = {
        {VARIABLE, VARIABLE, PLUS, END}, //q1+q2 -> q1 q2 +
        {Q1, Q1},
        {}
    };

    //m_opa_var.push_back( opa_var_1 );
    m_opa_var.push_back( opa_var_2 );
    //m_opa_var.push_back( opa_var_2 );

    EquationToken col_var_1 = {
        {VARIABLE, VALUE, POW, VARIABLE, VALUE, POW, PLUS, SQRT, END}, //sqrt(q1^2+q2^2)->q1 2 ^ q2 2 ^ + sqrt
        {Q1,NOTHING,NOTHING,Q2},//q1,,,q2,,,
        {0,2,0,0,2}//,2,,2,
    };

    EquationToken col_var_2 = {
        {VARIABLE, VARIABLE, PLUS, END}, //q1+q2 -> q1 q2 +
        {Q1, Q1},
        {}
    };

    //m_col_var.push_back( col_var_1 );
    m_col_var.push_back( col_var_2 );
    //m_col_var.push_back( col_var_2 );
}

TransferFunctionSynthesizer::TransferFunctionSynthesizer( TransferFunctionSynthesizer& tfs ):
    m_scalars(TF_COUNT)
{
    m_var_value_struct = new float*[128];
    for(int i=0; i<128; i++) m_var_value_struct[i] = new float[SIMDW];


    this->setOpacityFunction( tfs.opacityFunction() );
    this->setColorFunction( tfs.colorFunction() );
    this->setOpacityVariable( tfs.opacityVariable() );
    this->setColorVariable( tfs.colorVariable() );
   
    m_variable_range = tfs.m_variable_range; 
}

// function 4 debug add by shimomura 2022/12/28 
void  TransferFunctionSynthesizer::setStabToken()
{

//    // stab token
//    //  only q1
//    EquationToken opa_func = {
//        {VARIABLE, END}, // A1+A2 -> A1 A2 +
//        {A1},// A1, A2
//        {} //nothing
//    };
//
//    m_opa_func = opa_func;
//
//    EquationToken col_func = {
//        {VARIABLE, END},// C1
//        {C1},// C1
//        {}//nothing
//    };
//
//    m_col_func = col_func;
//
//    EquationToken opa_var_1 = {
//        {VARIABLE, END}, //q1-q2 -> q1 q2 -
//        {Q1},
//        {}
//    };
//
//    EquationToken col_var_1 = {
//        {VARIABLE, END}, //q1-q2 -> q1 q2 -
//        {Q1},
//        {}
//    };


    EquationToken opa_func = {
        {VARIABLE, END}, // A1+A2 -> A1 A2 +
        {A1},// A1, A2
        {} //nothing
    };

    m_opa_func = opa_func;

    EquationToken col_func = {
        {VARIABLE, END},// C1
        {C1},// C1
        {}//nothing
    };

    m_col_func = col_func;

    EquationToken opa_var_1 = {
        //{VARIABLE, VARIABLE, PLUS, END}, //q1+q1 -> q1 q1 +
        //{Q1,Q1},
        {VARIABLE, END}, //q1+q1 -> q1 q1 +
        {Q1},
        {}
    };

    EquationToken col_var_1 = {
        //{VARIABLE,VARIABLE, PLUS, END}, //q1+q1 -> q1 q1 +
        //{Q1, Q1},
        {VARIABLE,END}, //q1+q1 -> q1 q1 +
        {Q1 },
        {}
    };

    std::vector<EquationToken> opa_var;//ex) q1+q2
    std::vector<EquationToken> col_var;//ex) q3*q4
    opa_var.push_back( opa_var_1 );
    col_var.push_back( col_var_1 );

    m_opa_var = opa_var;
    m_col_var = col_var;

//            std::stringstream debug10; 
//            debug10 << "exp_token = {" ;
//            for (int i=0; i<NUMVAR; i++) debug10 << m_opa_var[0].exp_token[i] << ",";
//            debug10 << "}" << std::endl;
//            debug10 << "var_name = {" ;
//            for (int i=0; i<NUMVAR; i++) debug10 << m_opa_var[0].var_name[i] << ",";
//            debug10 << "}" << std::endl;
//            debug10 << "val_array = {" ;
//            for (int i=0; i<NUMVAR; i++) debug10 << m_opa_var[0].val_array[i] << ",";
//            debug10 << "}" << std::endl;
//            std::cout <<debug10.str() << std::endl;  


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

void TransferFunctionSynthesizer::setMaxOpacity( const float opacity )
{
    m_max_opacity = opacity;
}

void TransferFunctionSynthesizer::setMaxDensity( const float density )
{
    m_max_density = density;
}

void TransferFunctionSynthesizer::setSamplingVolumeInverse( const float sampling_volume_inverse  ) 
{
    m_sampling_volume_inverse = sampling_volume_inverse;
}

float TransferFunctionSynthesizer::getSamplingVolumeInverse() const
{
    return m_sampling_volume_inverse;
}

float TransferFunctionSynthesizer::getMaxOpacity() const
{
    return m_max_opacity;
}

float TransferFunctionSynthesizer::getMaxDensity() const
{
    return m_max_density;
}

void TransferFunctionSynthesizer::setParticleDensity(float particle_density ) 
{
    m_particle_density = particle_density;
}

void TransferFunctionSynthesizer::setParticleDataSizeLimit(float particle_data_size_limit ) 
{
    m_particle_data_size_limit = particle_data_size_limit;
}

std::vector<float> TransferFunctionSynthesizer::scalars() 
{
    return m_scalars;
}

float TransferFunctionSynthesizer::getParticleDensity()
{
    return m_particle_density;
}

float TransferFunctionSynthesizer::getParticleDataSizeLimit()
{
    return m_particle_data_size_limit;
}

VariableRange& TransferFunctionSynthesizer::variableRange()
{
    return m_variable_range;
}

const VariableRange& TransferFunctionSynthesizer::variableRange() const
{
    return m_variable_range;
}                                                                                                                                                                  

void TransferFunctionSynthesizer::mergeVarRange( const TransferFunctionSynthesizer& tfs )
{
    m_variable_range.merge( tfs.m_variable_range );
}

TransferFunctionSynthesizer::~TransferFunctionSynthesizer()
{
    for(int i=0; i<128; i++) delete[] m_var_value_struct[i];
    delete[] m_var_value_struct;
}

void TransferFunctionSynthesizer::create()
{
    //m_variable_range.clear();
    //return;
}

//---------structured----------------
//kawamura
void TransferFunctionSynthesizer::SynthesizedOpacityScalars(
    std::vector< TFS::TrilinearInterpolator* > interp ,
    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
    float scalars[][SIMDW] )//resulting scalars[TF_COUNT][SIMDW]
{
    for( int I=0; I<SIMDW; I++)
    {
        m_var_value_struct[X][I] = x_g[I];
        m_var_value_struct[Y][I] = y_g[I];
        m_var_value_struct[Z][I] = z_g[I];
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
            m_var_value_struct[4*(j+1)  ][I] = scalar[I];
            m_var_value_struct[4*(j+1)+1][I] = grad_x[I];
            m_var_value_struct[4*(j+1)+2][I] = grad_y[I];
            m_var_value_struct[4*(j+1)+3][I] = grad_z[I];
        }
    }

    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value_struct );
        //m_rpn.eval( &scalars[i][0] );
        m_rpn.setVariableValueArray( m_var_value_struct );
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
        m_var_value_struct[X][I] = x_g[I];
        m_var_value_struct[Y][I] = y_g[I];
        m_var_value_struct[Z][I] = z_g[I];
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
            m_var_value_struct[4*(j+1)  ][I] = scalar[I];
            m_var_value_struct[4*(j+1)+1][I] = grad_x[I];
            m_var_value_struct[4*(j+1)+2][I] = grad_y[I];
            m_var_value_struct[4*(j+1)+3][I] = grad_z[I];
        }
    }


    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_col_var.size(); i++ )
    {
        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value_struct );
        //m_rpn.eval( &scalars[i][0] );
        m_rpn.setVariableValueArray( m_var_value_struct );
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
        m_var_value_struct[X][I] = x_g[I];
        m_var_value_struct[Y][I] = y_g[I];
        m_var_value_struct[Z][I] = z_g[I];
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
            m_var_value_struct[4*(j+1)  ][I] = scalar[I];
            m_var_value_struct[4*(j+1)+1][I] = grad_x[I];
            m_var_value_struct[4*(j+1)+2][I] = grad_y[I];
            m_var_value_struct[4*(j+1)+3][I] = grad_z[I];
        }
    }

    //ボリュームデータ合成式のループ
    for( size_t i = 0; i < m_opa_var.size(); i++ )
    {
        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );
        //m_rpn.setVariableValue( m_var_value_struct );
        m_rpn.setVariableValueArray( m_var_value_struct );

        //calc. m_opa_var
        float scalars[SIMDW];
        //m_rpn.eval( &scalars[0] );
        m_rpn.evalArray( &scalars[0], SIMDW );

        //set opacity A1,A2,,,Ai. start 116 in VarName(Token.h)
        for( int I=0; I<SIMDW; I++ )
        {
            m_var_value_struct[ VAR_OFFSET_A+i ][I] = tf[i].opacityMap().at( scalars[I] );
            //std::cout<<m_var_value_struct[ VAR_OFFSET_A+i ] <<",";
        }
    }
    //std::cout<<std::endl;

    //set opacity function eq. ex) A1*A2+A3
    m_rpn.setExpToken( &(m_opa_func.exp_token[0]) );
    m_rpn.setVariableName( &(m_opa_func.var_name[0]) );
    m_rpn.setNumber( &(m_opa_func.val_array[0]) );
    //m_rpn.setVariableValue( m_var_value_struct );
    m_rpn.setVariableValueArray( m_var_value_struct );

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
        m_var_value_struct[X][I] = x_g[I];
        m_var_value_struct[Y][I] = y_g[I];
        m_var_value_struct[Z][I] = z_g[I];
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
            m_var_value_struct[4*(j+1)  ][I] = scalar[I];
            m_var_value_struct[4*(j+1)+1][I] = grad_x[I];
            m_var_value_struct[4*(j+1)+2][I] = grad_y[I];
            m_var_value_struct[4*(j+1)+3][I] = grad_z[I];
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
        //m_rpn.setVariableValue( m_var_value_struct );
        m_rpn.setVariableValueArray( m_var_value_struct );

        //calc. m_col_var
        float scalars[SIMDW];
        //m_rpn.eval( &scalars[0] );
        m_rpn.evalArray( &scalars[0], SIMDW );
        //if(i==0) std::cout<<"local_coord="<<local_coord<<", ";
        //if(i==0) std::cout<<"m_var_value_struct="<<m_var_value_struct[4]<<", ";
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
            m_var_value_struct[ VAR_OFFSET_C+i ][I] = col;
        }
    }
    //m_rpn.setVariableValue( m_var_value_struct );
    m_rpn.setVariableValueArray( m_var_value_struct );
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
            m_var_value_struct[ VAR_OFFSET_C+i ][I] = col;
        }
    }
    //m_rpn.setVariableValue( m_var_value_struct );
    m_rpn.setVariableValueArray( m_var_value_struct );
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
            m_var_value_struct[ VAR_OFFSET_C+i ][I] = col;
        }
    }
    //m_rpn.setVariableValue( m_var_value_struct );
    m_rpn.setVariableValueArray( m_var_value_struct );
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

//------------unstructured------
//kawamura
std::vector<float> TransferFunctionSynthesizer::SynthesizedOpacityScalars(
    std::vector< pbvr::CellBase<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord )
    //std::vector< pbvr::HexahedralCell<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord )
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

////kawamura
//void TransferFunctionSynthesizer::SynthesizedOpacityScalars(
//    std::vector< TFS::TrilinearInterpolator* > interp ,
//    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//    float scalars[][SIMDW] )//resulting scalars[TF_COUNT][SIMDW]
//{
//    for( int I=0; I<SIMDW; I++)
//    {
//        m_var_value[X][I] = x_g[I];
//        m_var_value[Y][I] = y_g[I];
//        m_var_value[Z][I] = z_g[I];
//    }
//
//    const size_t nvar = interp.size();
//
//    //各変数のループ
//    //id of Q1=4, Q2=8,,,,, Qn=4*n
//    //id of dxQ1=5, dyQ1=6, dzQ1=7,
//    for( size_t j= 0; j < nvar; j++ )
//    {
//        float scalar[SIMDW];
//        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];
//
//        interp[j]->attachPoint( x_l, y_l, z_l );
//        interp[j]->scalar( scalar );
//        interp[j]->gradient( grad_x, grad_y, grad_z );
//
//        for( int I=0; I<SIMDW; I++ )
//        {
//            m_var_value[4*(j+1)  ][I] = scalar[I];
//            m_var_value[4*(j+1)+1][I] = grad_x[I];
//            m_var_value[4*(j+1)+2][I] = grad_y[I];
//            m_var_value[4*(j+1)+3][I] = grad_z[I];
//        }
//    }
//
//    //ボリュームデータ合成式のループ
//    for( size_t i = 0; i < m_opa_var.size(); i++ )
//    {
//        m_rpn.setExpToken( &(m_opa_var[i].exp_token[0]) );
//        m_rpn.setVariableName( &(m_opa_var[i].var_name[0]) );
//        m_rpn.setNumber( &(m_opa_var[i].val_array[0]) );
//        //m_rpn.setVariableValue( m_var_value );
//        //m_rpn.eval( &scalars[i][0] );
//        m_rpn.setVariableValueArray( m_var_value );
//        m_rpn.evalArray( &scalars[i][0], SIMDW );
//    }
//}

// Fj add
void TransferFunctionSynthesizer::SynthesizedOpacityScalarsArray(
//    std::vector< pbvr::HexahedralCell<Type>* > interp ,
    std::vector< pbvr::CellBase<Type>* > interp ,
    const int loop_cnt,
    const kvs::Vector3f *local_coord,
    const kvs::Vector3f *global_coord,
    std::vector<float> *o_scalars_array )
{
    //配列を追加
    //float scalar_array[interp.size()][loop_cnt];

    //float grad_array_x[interp.size()][loop_cnt];   
    //float grad_array_y[interp.size()][loop_cnt];    
    //float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];

    //float global_coord_x[loop_cnt];
    //float global_coord_y[loop_cnt];    
    //float global_coord_z[loop_cnt];

    //float eval_result[m_opa_var.size()][loop_cnt];

    float** scalar_array = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        scalar_array[i] = new float[loop_cnt];
    }

    float** grad_array_x = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_x[i] = new float[loop_cnt];
    }

    float** grad_array_y = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_y[i] = new float[loop_cnt];
    }

    float** grad_array_z = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_z[i] = new float[loop_cnt];
    }

    float* global_coord_x;
    global_coord_x = new float[loop_cnt];

    float* global_coord_y;
    global_coord_y = new float[loop_cnt];

    float* global_coord_z;
    global_coord_z = new float[loop_cnt];

    float** eval_result = new float* [m_opa_var.size()];
    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        eval_result[i] = new float[loop_cnt];
    }

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
        interp[j]->CalcAveragedScalarGrad( loop_cnt,
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

        //std::cout << "NUMVAR = " << NUMVAR << std::endl;
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

    //配列を削除
    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] scalar_array[i];
    }
    delete[] scalar_array;
    scalar_array = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_x[i];
    }
    delete[] grad_array_x;
    grad_array_x = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_y[i];
    }
    delete[] grad_array_y;
    grad_array_y = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_z[i];
    }
    delete[] grad_array_z;
    grad_array_z = nullptr;

    delete[] global_coord_x;
    delete[] global_coord_y;
    delete[] global_coord_z;

    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        delete[] eval_result[i];
    }
    delete[] eval_result;
    eval_result = nullptr;
}


////kawamura
//void TransferFunctionSynthesizer::SynthesizedColorScalars(
//    std::vector< TFS::TrilinearInterpolator* > interp ,
//    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//    float scalars[][SIMDW] )//resulting scalars[TF_COUNT][SIMDW]
//{
//    for( int I=0; I<SIMDW; I++)
//    {
//        m_var_value[X][I] = x_g[I];
//        m_var_value[Y][I] = y_g[I];
//        m_var_value[Z][I] = z_g[I];
//    }
//
//    const size_t nvar = interp.size();
//
//    //各変数のループ
//    //id of Q1=4, Q2=8,,,,, Qn=4*n
//    //id of dxQ1=5, dyQ1=6, dzQ1=7,
//    for( size_t j= 0; j < nvar; j++ )
//    {
//        float scalar[SIMDW];
//        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];
//
//        interp[j]->attachPoint( x_l, y_l, z_l );
//        interp[j]->scalar( scalar );
//        interp[j]->gradient( grad_x, grad_y, grad_z );
//
//        for( int I=0; I<SIMDW; I++ )
//        {
//            m_var_value[4*(j+1)  ][I] = scalar[I];
//            m_var_value[4*(j+1)+1][I] = grad_x[I];
//            m_var_value[4*(j+1)+2][I] = grad_y[I];
//            m_var_value[4*(j+1)+3][I] = grad_z[I];
//        }
//    }
//
//
//    //ボリュームデータ合成式のループ
//    for( size_t i = 0; i < m_col_var.size(); i++ )
//    {
//        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
//        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
//        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );
//        //m_rpn.setVariableValue( m_var_value );
//        //m_rpn.eval( &scalars[i][0] );
//        m_rpn.setVariableValueArray( m_var_value );
//        m_rpn.evalArray( &scalars[i][0], SIMDW );
//    }
//}

// Fj add
void TransferFunctionSynthesizer::SynthesizedColorScalarsArray(
//    std::vector< pbvr::HexahedralCell<Type>* > interp,
    std::vector< pbvr::CellBase<Type>* > interp ,
    const int loop_cnt,
    const kvs::Vector3f *local_coord,
    const kvs::Vector3f *global_coord,
    std::vector<float> *c_scalars_array )
{

    //配列を追加
    //float scalar_array[interp.size()][loop_cnt];

    //float grad_array_x[interp.size()][loop_cnt];
    //float grad_array_y[interp.size()][loop_cnt];
    //float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];

    //float global_coord_x[loop_cnt];
    //float global_coord_y[loop_cnt];
    //float global_coord_z[loop_cnt];

    //float eval_result[m_opa_var.size()][loop_cnt];

    float** scalar_array = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        scalar_array[i] = new float[loop_cnt];
    }

    float** grad_array_x = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_x[i] = new float[loop_cnt];
    }

    float** grad_array_y = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_y[i] = new float[loop_cnt];
    }

    float** grad_array_z = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_z[i] = new float[loop_cnt];
    }

    float* global_coord_x;
    global_coord_x = new float[loop_cnt];

    float* global_coord_y;
    global_coord_y = new float[loop_cnt];

    float* global_coord_z;
    global_coord_z = new float[loop_cnt];

    float** eval_result = new float* [m_col_var.size()];
    for (int i = 0; i < m_col_var.size(); ++i)
    {
        eval_result[i] = new float[loop_cnt];
    }

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
        //interp[j]->CalcScalarGrad( loop_cnt,
        interp[j]->CalcAveragedScalarGrad( loop_cnt,
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

    //配列を削除
    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] scalar_array[i];
    }
    delete[] scalar_array;
    scalar_array = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_x[i];
    }
    delete[] grad_array_x;
    grad_array_x = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_y[i];
    }
    delete[] grad_array_y;
    grad_array_y = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_z[i];
    }
    delete[] grad_array_z;
    grad_array_z = nullptr;

    delete[] global_coord_x;
    delete[] global_coord_y;
    delete[] global_coord_z;

    for (int i = 0; i < m_col_var.size(); ++i)
    {
        delete[] eval_result[i];
    }
    delete[] eval_result;
    eval_result = nullptr;
}

//kawamura
float TransferFunctionSynthesizer::CalculateOpacity(
    std::vector< pbvr::CellBase<Type>* > interp , kvs::Vector3f local_coord, kvs::Vector3f global_coord,
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

void TransferFunctionSynthesizer::CalculateCoordArray(
    //std::vector< pbvr::HexahedralCell<Type>* > interp , 
    std::vector< pbvr::CellBase<Type>* > interp , 
    const int loop_cnt,
    const kvs::Vector3f *local_coord, 
    const kvs::Vector3f *global_coord,
    std::vector<pbvr::TransferFunction>& tf,
    const pbvr::CoordSynthesizerTokens cst,
//    float *new_coord_array)
    kvs::Vector3f *new_coord_array)
{

    //配列を追加
    //float scalar_array[interp.size()][loop_cnt];

    //float grad_array_x[interp.size()][loop_cnt];
    //float grad_array_y[interp.size()][loop_cnt];
    //float grad_array_z[interp.size()][loop_cnt];

    //float global_coord_x[loop_cnt];
    //float global_coord_y[loop_cnt];
    //float global_coord_z[loop_cnt];

    //float eval_result[loop_cnt];

    //float opacity_map_array[m_opa_var.size()][loop_cnt];

    float** scalar_array = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        scalar_array[i] = new float[loop_cnt];
    }

    float** grad_array_x = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_x[i] = new float[loop_cnt];
    }

    float** grad_array_y = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_y[i] = new float[loop_cnt];
    }

    float** grad_array_z = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_z[i] = new float[loop_cnt];
    }

    float* global_coord_x;
    global_coord_x = new float[loop_cnt];

    float* global_coord_y;
    global_coord_y = new float[loop_cnt];

    float* global_coord_z;
    global_coord_z = new float[loop_cnt];

    float* eval_result;
    eval_result = new float[loop_cnt];

    float** opacity_map_array = new float* [m_opa_var.size()];
    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        opacity_map_array[i] = new float[loop_cnt];
    }

    EquationToken x_synthesis; 
    EquationToken y_synthesis; 
    EquationToken z_synthesis; 
            
    for(int i = 0 ; i<128; i++ )
    {
        x_synthesis.exp_token[i] = cst.m_x_coord_synthesizer_token.exp_token[i];
        x_synthesis.var_name[i]  = cst.m_x_coord_synthesizer_token.var_name[i] ;
        x_synthesis.val_array[i] = cst.m_x_coord_synthesizer_token.val_array[i];
        y_synthesis.exp_token[i] = cst.m_y_coord_synthesizer_token.exp_token[i];
        y_synthesis.var_name[i]  = cst.m_y_coord_synthesizer_token.var_name[i] ;
        y_synthesis.val_array[i] = cst.m_y_coord_synthesizer_token.val_array[i];
        z_synthesis.exp_token[i] = cst.m_z_coord_synthesizer_token.exp_token[i];
        z_synthesis.var_name[i]  = cst.m_z_coord_synthesizer_token.var_name[i] ;
        z_synthesis.val_array[i] = cst.m_z_coord_synthesizer_token.val_array[i];
    }

    for (int i = 0; i < loop_cnt; i++)
    {
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
        interp[j]->CalcAveragedScalarGrad( loop_cnt,
                                   scalar_array[j],
                                   grad_array_x[j],
                                   grad_array_y[j],
                                   grad_array_z[j] );
    }

    m_var_value_array[X] = global_coord_x;
    m_var_value_array[Y] = global_coord_y;
    m_var_value_array[Z] = global_coord_z;

    if(cst.x_token_empty)
    { 
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            new_coord_array[jx][0]   = global_coord_x[jx]; 
        }
    }
    else{
//        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken    ( &(x_synthesis.exp_token[0]) );
        m_rpn.setVariableName( &(x_synthesis.var_name[0]) );
        m_rpn.setNumber      ( &(x_synthesis.val_array[0]) );

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
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            new_coord_array[jx][0]   = eval_result[jx]; 
        }
    }

    if(cst.y_token_empty)
    { 
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            new_coord_array[jx][1]   = global_coord_y[jx]; 
        }
    }
    else
    {

//        //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken    ( &(y_synthesis.exp_token[0]) );
        m_rpn.setVariableName( &(y_synthesis.var_name[0]) );
        m_rpn.setNumber      ( &(y_synthesis.val_array[0]) );

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
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            new_coord_array[jx][1]   = eval_result[jx]; 
        }
    }

    if(cst.z_token_empty)
    { 
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            new_coord_array[jx][2]   = global_coord_z[jx]; 
        }
    }
    else
    {

       //set variable eq. ex) Q1+Q2/Q3
        m_rpn.setExpToken    ( &(z_synthesis.exp_token[0]) );
        m_rpn.setVariableName( &(z_synthesis.var_name[0]) );
        m_rpn.setNumber      ( &(z_synthesis.val_array[0]) );


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
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            new_coord_array[jx][2]   = eval_result[jx]; 
        }
    }

    //配列の削除
    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] scalar_array[i];
    }
    delete[] scalar_array;
    scalar_array = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_x[i];
    }
    delete[] grad_array_x;
    grad_array_x = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_y[i];
    }
    delete[] grad_array_y;
    grad_array_y = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_z[i];
    }
    delete[] grad_array_z;
    grad_array_z = nullptr;

    delete[] global_coord_x;
    delete[] global_coord_y;
    delete[] global_coord_z;

    delete[] eval_result;

    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        delete[] opacity_map_array[i];
    }
    delete[] opacity_map_array;
    opacity_map_array = nullptr;
}

void TransferFunctionSynthesizer::CalculateOpacityArrayAverage(
    //std::vector< pbvr::HexahedralCell<Type>* > interp , 
    std::vector< pbvr::CellBase<Type>* > interp , 
    const int loop_cnt,
    const kvs::Vector3f *local_coord, 
    const kvs::Vector3f *global_coord,
    std::vector<pbvr::TransferFunction>& tf,
    float *opacity_array)
{

    //配列を追加
    //float scalar_array[interp.size()][loop_cnt];

    //float grad_array_x[interp.size()][loop_cnt];
    //float grad_array_y[interp.size()][loop_cnt];
    //float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];

    //float global_coord_x[loop_cnt];
    //float global_coord_y[loop_cnt];
    //float global_coord_z[loop_cnt];

    //float eval_result[loop_cnt];

    //float opacity_map_array[m_opa_var.size()][loop_cnt];

    float** scalar_array = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        scalar_array[i] = new float[loop_cnt];
    }

    float** grad_array_x = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_x[i] = new float[loop_cnt];
    }

    float** grad_array_y = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_y[i] = new float[loop_cnt];
    }

    float** grad_array_z = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_z[i] = new float[loop_cnt];
    }

    float* global_coord_x;
    global_coord_x = new float[loop_cnt];

    float* global_coord_y;
    global_coord_y = new float[loop_cnt];

    float* global_coord_z;
    global_coord_z = new float[loop_cnt];

    float* eval_result;
    eval_result = new float[loop_cnt];

    float** opacity_map_array = new float* [m_opa_var.size()];
    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        opacity_map_array[i] = new float[loop_cnt];
    }

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
        interp[j]->CalcAveragedScalarGrad( loop_cnt,
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

        //std::stringstream debug_value;
        //for (int j=0; j<loop_cnt; j += 1) {
        //debug_value << "(x,y,z) = (" << global_coord_x[j] << ", " << global_coord_y[j]<< ", " << global_coord_z[j]<< std::endl; 
        //debug_value << "(scalar_array,grad_array) = " << scalar_array[0][j] << ", " << grad_array_x[0][j] << ", " << grad_array_y[0][j] << ", "<< grad_array_z[0][j] << std::endl;
        //}   
        //std::cout << debug_value.str() << std::endl;

        //calc. m_opa_var
        m_rpn.evalArray(eval_result, loop_cnt);
    
        // debug stab 2023/01/10 add by shimomura 
        //tf[i].setRange(1.0, 3.0);
        //tf[i].setRange(0.0, 1.0);
            
        //set opacity A1,A2,,,Ai. start 116 in VarName(Token.h)
        //m_var_value[ VAR_OFFSET_A+i ] = tf[i].opacityMap().at( m_scalars[i] );
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            opacity_map_array[i][jx] = tf[i].opacityMap().at( eval_result[jx] );
        }
        m_var_value_array[ VAR_OFFSET_A+i ] = &opacity_map_array[i][0];

    }

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

    //配列を削除
    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] scalar_array[i];
    }
    delete[] scalar_array;
    scalar_array = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_x[i];
    }
    delete[] grad_array_x;
    grad_array_x = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_y[i];
    }
    delete[] grad_array_y;
    grad_array_y = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_z[i];
    }
    delete[] grad_array_z;
    grad_array_z = nullptr;

    delete[] global_coord_x;
    delete[] global_coord_y;
    delete[] global_coord_z;

    delete[] eval_result;

    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        delete[] opacity_map_array[i];
    }
    delete[] opacity_map_array;
    opacity_map_array = nullptr;
}


void TransferFunctionSynthesizer::CalculateOpacityArray(
    //std::vector< pbvr::HexahedralCell<Type>* > interp , 
    std::vector< pbvr::CellBase<Type>* > interp , 
    const int loop_cnt,
    const kvs::Vector3f *local_coord, 
    const kvs::Vector3f *global_coord,
    std::vector<pbvr::TransferFunction>& tf,
    float *opacity_array)
{

    //配列を追加
    //float scalar_array[interp.size()][loop_cnt];

    //float grad_array_x[interp.size()][loop_cnt];
    //float grad_array_y[interp.size()][loop_cnt];
    //float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];

    //float global_coord_x[loop_cnt];
    //float global_coord_y[loop_cnt];
    //float global_coord_z[loop_cnt];

    //float eval_result[loop_cnt];

    //float opacity_map_array[m_opa_var.size()][loop_cnt];

    float** scalar_array = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        scalar_array[i] = new float[loop_cnt];
    }

    float** grad_array_x = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_x[i] = new float[loop_cnt];
    }

    float** grad_array_y = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_y[i] = new float[loop_cnt];
    }

    float** grad_array_z = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_z[i] = new float[loop_cnt];
    }

    float* global_coord_x;
    global_coord_x = new float[loop_cnt];

    float* global_coord_y;
    global_coord_y = new float[loop_cnt];

    float* global_coord_z;
    global_coord_z = new float[loop_cnt];

    float* eval_result;
    eval_result = new float[loop_cnt];

    float** opacity_map_array = new float* [m_opa_var.size()];
    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        opacity_map_array[i] = new float[loop_cnt];
    }

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

        //std::stringstream debug_value;
        //for (int j=0; j<loop_cnt; j += 1) {
        //debug_value << "(x,y,z) = (" << global_coord_x[j] << ", " << global_coord_y[j]<< ", " << global_coord_z[j]<< std::endl; 
        //debug_value << "(scalar_array,grad_array) = " << scalar_array[0][j] << ", " << grad_array_x[0][j] << ", " << grad_array_y[0][j] << ", "<< grad_array_z[0][j] << std::endl;
        //}   
        //std::cout << debug_value.str() << std::endl;

        //calc. m_opa_var
        m_rpn.evalArray(eval_result, loop_cnt);
    
        // debug stab 2023/01/10 add by shimomura 
        //tf[i].setRange(1.0, 3.0);
        //tf[i].setRange(0.0, 1.0);
            
        //set opacity A1,A2,,,Ai. start 116 in VarName(Token.h)
        //m_var_value[ VAR_OFFSET_A+i ] = tf[i].opacityMap().at( m_scalars[i] );
        for( int jx=0; jx<loop_cnt; jx++ )
        {
            opacity_map_array[i][jx] = tf[i].opacityMap().at( eval_result[jx] );
        }
        m_var_value_array[ VAR_OFFSET_A+i ] = &opacity_map_array[i][0];

    }

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

    //配列の削除
    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] scalar_array[i];
    }
    delete[] scalar_array;
    scalar_array = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_x[i];
    }
    delete[] grad_array_x;
    grad_array_x = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_y[i];
    }
    delete[] grad_array_y;
    grad_array_y = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_z[i];
    }
    delete[] grad_array_z;
    grad_array_z = nullptr;

    delete[] global_coord_x;
    delete[] global_coord_y;
    delete[] global_coord_z;

    delete[] eval_result;

    for (int i = 0; i < m_opa_var.size(); ++i)
    {
        delete[] opacity_map_array[i];
    }
    delete[] opacity_map_array;
    opacity_map_array = nullptr;
}

////kawamura
//void TransferFunctionSynthesizer::CalculateColor(
//    std::vector< TFS::TrilinearInterpolator* > interp,
//    std::vector<pbvr::TransferFunction>& tf,
//    float* x_l, float* y_l, float* z_l, //local coord[SIMDW]
//    float* x_g, float* y_g, float* z_g, //global coord[SIMDW]
//    kvs::UInt8* Red, kvs::UInt8* Green, kvs::UInt8* Blue )//resulting colors[SIMDW]
//{
//    for( int I=0; I<SIMDW; I++)
//    {
//        m_var_value[X][I] = x_g[I];
//        m_var_value[Y][I] = y_g[I];
//        m_var_value[Z][I] = z_g[I];
//    }
//
//    const size_t nvar = interp.size();
//
//    //各変数のループ
//    //id of Q1=4, Q2=8,,,,, Qn=4*n
//    for( size_t j= 0; j < nvar; j++ )
//    {
//        float scalar[SIMDW];
//        float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];
//
//        interp[j]->attachPoint( x_l, y_l, z_l );
//        interp[j]->scalar( scalar );
//        interp[j]->gradient( grad_x, grad_y, grad_z );
//
//        for( int I=0; I<SIMDW; I++ )
//        {
//            m_var_value[4*(j+1)  ][I] = scalar[I];
//            m_var_value[4*(j+1)+1][I] = grad_x[I];
//            m_var_value[4*(j+1)+2][I] = grad_y[I];
//            m_var_value[4*(j+1)+3][I] = grad_z[I];
//        }
//    }
//
//    kvs::RGBColor colors[m_col_var.size()][SIMDW]; //result of t_func.colorMap().at( m_scalars[i] );
//
//    //ボリュームデータ合成式のループ
//    for( size_t i = 0; i < m_col_var.size(); i++ )
//    {
//        //set variable eq. ex) Q1+Q2/Q3
//        m_rpn.setExpToken( &(m_col_var[i].exp_token[0]) );
//        m_rpn.setVariableName( &(m_col_var[i].var_name[0]) );
//        m_rpn.setNumber( &(m_col_var[i].val_array[0]) );
//        //m_rpn.setVariableValue( m_var_value );
//        m_rpn.setVariableValueArray( m_var_value );
//
//        //calc. m_col_var
//        float scalars[SIMDW];
//        //m_rpn.eval( &scalars[0] );
//        m_rpn.evalArray( &scalars[0], SIMDW );
//        //if(i==0) std::cout<<"local_coord="<<local_coord<<", ";
//        //if(i==0) std::cout<<"m_var_value="<<m_var_value[4]<<", ";
//        //if(i==0) std::cout<<"TFS:col:scalars="<<m_scalars[i]<<std::endl;
//
//        for( int I=0; I<SIMDW; I++ )
//        {
//            colors[i][I] = tf[i].colorMap().at( scalars[I] );
//        }
//    }
//
//    //std::cout<<"TFS:RGB="<<(int)colors[0].r()<<","<<(int)colors[0].g()<<","<<(int)colors[0].b()<<std::endl;
//
//    //set color function eq. ex) C1*C2+C3
//    m_rpn.setExpToken( &(m_col_func.exp_token[0]) );
//    m_rpn.setVariableName( &(m_col_func.var_name[0]) );
//    m_rpn.setNumber( &(m_col_func.val_array[0]) );
//
//    //RED
//    for( size_t i = 0; i < m_col_var.size(); i++ )
//    {
//        for( int I=0; I<SIMDW; I++ )
//        {
//            float col = (float)colors[i][I].r() / 255.0;
//            //if(i==0)std::cout<<"colR="<<col<<std::endl;
//            m_var_value[ VAR_OFFSET_C+i ][I] = col;
//        }
//    }
//    //m_rpn.setVariableValue( m_var_value );
//    m_rpn.setVariableValueArray( m_var_value );
//    float red[SIMDW];
//    //m_rpn.eval( &red[0]  );
//    m_rpn.evalArray( &red[0], SIMDW );
//
//    for( int I=0; I<SIMDW; I++ )
//    {
//        red[I] = red[I]<0 ? 0 : red[I]>1.0 ? 1 : red[I];
//    }
//
//    //GREEN
//    for( size_t i = 0; i < m_col_var.size(); i++ )
//    {
//        for( int I=0; I<SIMDW; I++ )
//        {
//            float col = (float)colors[i][I].g() / 255.0;
//            //if(i==0)std::cout<<"colG="<<col<<std::endl;
//            m_var_value[ VAR_OFFSET_C+i ][I] = col;
//        }
//    }
//    //m_rpn.setVariableValue( m_var_value );
//    m_rpn.setVariableValueArray( m_var_value );
//    float green[SIMDW];
//    //m_rpn.eval( &green[0] );
//    m_rpn.evalArray( &green[0], SIMDW );
//
//    for( int I=0; I<SIMDW; I++ )
//    {
//        green[I] = green[I]<0 ? 0 : green[I]>1.0 ? 1 : green[I];
//    }
//
//
//    //BLUE
//    for( size_t i = 0; i < m_col_var.size(); i++ )
//    {
//        for( int I=0; I<SIMDW; I++ )
//        {
//            float col = (float)colors[i][I].b() / 255.0;
//            //if(i==0)std::cout<<"colB="<<col<<std::endl;
//            m_var_value[ VAR_OFFSET_C+i ][I] = col;
//        }
//    }
//    //m_rpn.setVariableValue( m_var_value );
//    m_rpn.setVariableValueArray( m_var_value );
//    float blue[SIMDW];
//    //m_rpn.eval( &blue[0] );
//    m_rpn.evalArray( &blue[0], SIMDW );
//
//    for( int I=0; I<SIMDW; I++ )
//    {
//        blue[I] = blue[I]<0 ? 0 : blue[I]>1.0 ? 1 : blue[I];
//    }
//     
//    for( int I=0; I<SIMDW; I++ )
//    {
//        Red[I] = (kvs::UInt8)(red[I]*255);
//        Green[I] = (kvs::UInt8)(green[I]*255);
//        Blue[I] = (kvs::UInt8)(blue[I]*255);
//    }
//}

void TransferFunctionSynthesizer::CalculateColorArray(
//    std::vector< pbvr::HexahedralCell<Type>* > interp,
    std::vector< pbvr::CellBase<Type>* > interp , 
    const int loop_cnt,
    const kvs::Vector3f *local_coord,
    const kvs::Vector3f *global_coord,
    std::vector<pbvr::TransferFunction>& tf,
    kvs::RGBColor *color_seq)
{

    //配列を追加
    //float scalar_array[interp.size()][loop_cnt];

    //float grad_array_x[interp.size()][loop_cnt];
    //float grad_array_y[interp.size()][loop_cnt];
    //float grad_array_z[interp.size()][loop_cnt];

    //float local_coord_x[loop_cnt];
    //float local_coord_y[loop_cnt];
    //float local_coord_z[loop_cnt];

    //float global_coord_x[loop_cnt];
    //float global_coord_y[loop_cnt];
    //float global_coord_z[loop_cnt];

    //float eval_result[loop_cnt];

    float** scalar_array = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        scalar_array[i] = new float[loop_cnt];
    }

    float** grad_array_x = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_x[i] = new float[loop_cnt];
    }

    float** grad_array_y = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_y[i] = new float[loop_cnt];
    }

    float** grad_array_z = new float* [interp.size()];
    for (int i = 0; i < interp.size(); ++i)
    {
        grad_array_z[i] = new float[loop_cnt];
    }

    float* local_coord_x;
    local_coord_x = new float[loop_cnt];

    float* local_coord_y;
    local_coord_y = new float[loop_cnt];

    float* local_coord_z;
    local_coord_z = new float[loop_cnt];

    float* global_coord_x;
    global_coord_x = new float[loop_cnt];

    float* global_coord_y;
    global_coord_y = new float[loop_cnt];

    float* global_coord_z;
    global_coord_z = new float[loop_cnt];

    float* eval_result;
    eval_result = new float[loop_cnt];

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

    //kvs::RGBColor colors[10][loop_cnt]; //result of t_func.colorMap().at( m_scalars[i] );
    kvs::RGBColor** colors = new kvs::RGBColor* [10];
    for (int i = 0; i < 10; ++i)
    {
        colors[i] = new kvs::RGBColor[loop_cnt];
    }

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

    //float color_array[m_col_var.size()][loop_cnt];

    //float red_array[loop_cnt];
    //float green_array[loop_cnt];
    //float blue_array[loop_cnt];

    float** color_array = new float* [m_col_var.size()];
    for (int i = 0; i < m_col_var.size(); ++i)
    {
        color_array[i] = new float[loop_cnt];
    }

    float* red_array;
    red_array = new float[loop_cnt];

    float* green_array;
    green_array = new float[loop_cnt];

    float* blue_array;
    blue_array = new float[loop_cnt];

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

    //配列を削除
    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] scalar_array[i];
    }
    delete[] scalar_array;
    scalar_array = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_x[i];
    }
    delete[] grad_array_x;
    grad_array_x = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_y[i];
    }
    delete[] grad_array_y;
    grad_array_y = nullptr;

    for (int i = 0; i < interp.size(); ++i)
    {
        delete[] grad_array_z[i];
    }
    delete[] grad_array_z;
    grad_array_z = nullptr;

    delete[] global_coord_x;
    delete[] global_coord_y;
    delete[] global_coord_z;

    delete[] eval_result;

    for (int i = 0; i < 10; ++i)
    {
        delete[] colors[i];
    }
    delete[] colors;
    colors = nullptr;

    for (int i = 0; i < m_col_var.size(); ++i)
    {
        delete[] color_array[i];
    }
    delete[] color_array;
    color_array = nullptr;

    delete[] red_array;
    delete[] green_array;
    delete[] blue_array;
}

void TransferFunctionSynthesizer::AssertValid( const float& v, const char* file, const int line )
{
    if ( std::isnan( v ) || std::isinf( v ) )
    {
        std::stringstream ss;
        ss << "Assertion failed: NaN/Inf detected: "
           << file << ":" << line << std::endl;
#if (defined(KVS_PLATFORM_LINUX) || defined(KVS_PLATFORM_MACOSX))
        void* stk[128];
        int sz = backtrace( stk, 128 );
        char** sstr = backtrace_symbols( stk, sz );
        for ( int i = 0; i < sz; i++ )
        {
            ss << sstr[i] << std::endl;
        }
        std::free( sstr );
#endif
        throw NumericException( ss.str() );
    }
}



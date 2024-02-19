#include "RangeEstimater.h"

#if 0
VariableRange RangeEstimater::Estimation( const size_t step,
                                          const FilterInformationFile& fi,
                                          const TransferFunctionSynthesizer& tfs )
{
    VariableRange range;
    TransferFunctionSynthesizer s_min( tfs );
    TransferFunctionSynthesizer s_max( tfs );
    const QuantityMap& qmap = tfs.quantityMap();
    const TransferFunctionMap& fmap_c = tfs.colorTransferFunctionMap();
    const TransferFunctionMap& fmap_o = tfs.opacityTransferFunctionMap();
    const TransfuncQuantityBindMap& bmap_c = tfs.colorTransferFunctionBindMap();
    const TransfuncQuantityBindMap& bmap_o = tfs.opacityTransferFunctionBindMap();
    const FunctionMap& fmin = s_min.functionMap();
    const FunctionMap& fmax = s_max.functionMap();

    s_min.setValue( "X", fi.m_min_object_coord.x() );
    s_min.setValue( "Y", fi.m_min_object_coord.y() );
    s_min.setValue( "Z", fi.m_min_object_coord.z() );
    s_max.setValue( "X", fi.m_max_object_coord.x() );
    s_max.setValue( "Y", fi.m_max_object_coord.y() );
    s_max.setValue( "Z", fi.m_max_object_coord.z() );

    for ( QuantityMap::const_iterator q = qmap.begin(); q != qmap.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        s_min.setValue( nm, fi.m_ingredient_step[step].m_ingredient[qi].m_min );
        s_max.setValue( nm, fi.m_ingredient_step[step].m_ingredient[qi].m_max );
    }

    for ( FunctionMap::const_iterator i = fmin.begin(); i != fmin.end(); i++ )
    {
        const std::string& nm = i->first;
        const FuncParser::Function& f = i->second;
        s_min.setValue( nm, f.eval() );
    }

    for ( FunctionMap::const_iterator i = fmax.begin(); i != fmax.end(); i++ )
    {
        const std::string& nm = i->first;
        const FuncParser::Function& f = i->second;
        s_max.setValue( nm, f.eval() );
    }

    range.clear();
    for ( TransferFunctionMap::const_iterator f = fmap_c.begin(); f != fmap_c.end(); f++ )
    {
        const std::string& fnm = f->first;
        const std::string& qnm = bmap_c.find( fnm )->second;
        const std::string tag = fnm + "_var_c";

        range.setValue( tag, s_min.getValue( qnm ) );
        range.setValue( tag, s_max.getValue( qnm ) );
    }

    for ( TransferFunctionMap::const_iterator f = fmap_o.begin(); f != fmap_o.end(); f++ )
    {
        const std::string& fnm = f->first;
        const std::string& qnm = bmap_o.find( fnm )->second;
        const std::string tag = fnm + "_var_o";

        range.setValue( tag, s_min.getValue( qnm ) );
        range.setValue( tag, s_max.getValue( qnm ) );
    }

    return range;
}
#endif

VariableRange RangeEstimater::EstimationList(const size_t step,
                                             const FilterInformationList& fil,
                                             const TransferFunctionSynthesizer& tfs,
                                              jpv::ParticleTransferClientMessage clntMes )
{
    FuncParser::ReversePolishNotation m_rpn;
    VariableRange range;
    float* m_var_value_array[128];
    int loop_cnt = 2;
    int component_num = fil.m_list[0].m_number_ingredients;
    int tf_num = clntMes.color_var.size(); 

    //float comp_minmax[component_num][loop_cnt]; 
    float** comp_minmax = new float* [component_num];
    for ( int i = 0; i < component_num; i++ )
    {
        comp_minmax[i] = new float[loop_cnt];
    }

    //float eval_result[tf_num][loop_cnt];
    float** eval_result = new float* [tf_num];
    for (int i = 0; i < tf_num; i++)
    {
        eval_result[i] = new float[loop_cnt];
    }


    for (int i =0; i <component_num; i++)
    {
        comp_minmax[i][0] = fil.m_total_ingredient[i].m_min;
        comp_minmax[i][1] = fil.m_total_ingredient[i].m_max;
    }

    //----------------------------------- color 
    for (int i =0; i <tf_num; i++)
    {
        m_rpn.setExpToken( &(clntMes.color_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(clntMes.color_var[i].var_name[0]) );
        m_rpn.setNumber( &(clntMes.color_var[i].value_array[0]) );

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        //id of dxQ1=5, dyQ1=6, dzQ1=7,
        for( size_t i= 0; i <component_num ; i++ )
        {
            m_var_value_array[4*(i+1)  ] = &comp_minmax[i][0];
            m_var_value_array[4*(i+1)+1] = 0;
            m_var_value_array[4*(i+1)+2] = 0;
            m_var_value_array[4*(i+1)+3] = 0;
        }
     
        m_rpn.setVariableValueArray( m_var_value_array );
        m_rpn.evalArray(&eval_result[i][0], loop_cnt );
    }

    for ( int tf = 0; tf < tf_num; tf++ )
    {
        std::stringstream ss; 
        ss << (tf + 1); 
        const std::string idxbuf = ss.str();
        range.setValue( "t" + idxbuf + "_var_c", eval_result[tf][0]);
        range.setValue( "t" + idxbuf + "_var_c", eval_result[tf][1]);
//        std::cout << "opa_min_eval_result[tf][0] = " << eval_result[tf][0] <<std::endl;
//        std::cout << "opa_min_eval_result[tf][1] = " << eval_result[tf][1] <<std::endl;
    }   


    //----------------------------------- opacity
 
    for (int i =0; i <tf_num; i++)
    {
        m_rpn.setExpToken( &(clntMes.opacity_var[i].exp_token[0]) );
        m_rpn.setVariableName( &(clntMes.opacity_var[i].var_name[0]) );
        m_rpn.setNumber( &(clntMes.opacity_var[i].value_array[0]) );

        //id of Q1=4, Q2=8,,,,, Qn=4*n
        //id of dxQ1=5, dyQ1=6, dzQ1=7,
        for( size_t i= 0; i <component_num ; i++ )
        {
            m_var_value_array[4*(i+1)  ] = &comp_minmax[i][0];
            m_var_value_array[4*(i+1)+1] = 0;
            m_var_value_array[4*(i+1)+2] = 0;
            m_var_value_array[4*(i+1)+3] = 0;
        }
     
        m_rpn.setVariableValueArray( m_var_value_array );
        m_rpn.evalArray(&eval_result[i][0], loop_cnt );
    }

    for ( int tf = 0; tf < tf_num; tf++ )
    {
        std::stringstream ss; 
        ss << (tf + 1); 
        const std::string idxbuf = ss.str();
        range.setValue( "t" + idxbuf + "_var_o", eval_result[tf][0]);
        range.setValue( "t" + idxbuf + "_var_o", eval_result[tf][1]);
    }

    for (int i = 0; i < component_num; i++ )
    {
        delete[] comp_minmax[i];
    }
    delete[] comp_minmax;

    for (int i = 0; i < tf_num; i++)
    {
        delete[] eval_result[i];
    }
    delete[] eval_result;

    return range;
}

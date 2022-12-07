#include "RangeEstimater.h"

VariableRange RangeEstimater::Estimation( const size_t step,
                                          const FilterInformationFile *fi,
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

    s_min.setValue( "X", fi->m_min_object_coord.x() );
    s_min.setValue( "Y", fi->m_min_object_coord.y() );
    s_min.setValue( "Z", fi->m_min_object_coord.z() );
    s_max.setValue( "X", fi->m_max_object_coord.x() );
    s_max.setValue( "Y", fi->m_max_object_coord.y() );
    s_max.setValue( "Z", fi->m_max_object_coord.z() );

    for ( QuantityMap::const_iterator q = qmap.begin(); q != qmap.end(); q++ )
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        s_min.setValue( nm, fi->m_ingredient_step[step].m_ingredient[qi].m_min );
        s_max.setValue( nm, fi->m_ingredient_step[step].m_ingredient[qi].m_max );
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
        const std::string& qnm = bmap_c.find( fnm )->second;
        const std::string tag = fnm + "_var_o";

        range.setValue( tag, s_min.getValue( qnm ) );
        range.setValue( tag, s_max.getValue( qnm ) );
    }

    return range;
}

VariableRange RangeEstimater::EstimationList(const size_t step,
                                             const FilterInformationList* fil,
                                             const TransferFunctionSynthesizer& tfs)
{
    VariableRange range;
    TransferFunctionSynthesizer s_min(tfs);
    TransferFunctionSynthesizer s_max(tfs);
    const QuantityMap& qmap = tfs.quantityMap();
    const TransferFunctionMap& fmap_c = tfs.colorTransferFunctionMap();
    const TransferFunctionMap& fmap_o = tfs.opacityTransferFunctionMap();
    const TransfuncQuantityBindMap& bmap_c = tfs.colorTransferFunctionBindMap();
    const TransfuncQuantityBindMap& bmap_o = tfs.opacityTransferFunctionBindMap();
    const FunctionMap& fmin = s_min.functionMap();
    const FunctionMap& fmax = s_max.functionMap();

    s_min.setValue("X", fil->m_total_min_object_coord.x());
    s_min.setValue("Y", fil->m_total_min_object_coord.y());
    s_min.setValue("Z", fil->m_total_min_object_coord.z());
    s_max.setValue("X", fil->m_total_max_object_coord.x());
    s_max.setValue("Y", fil->m_total_max_object_coord.y());
    s_max.setValue("Z", fil->m_total_max_object_coord.z());

    for (QuantityMap::const_iterator q = qmap.begin(); q != qmap.end(); q++)
    {
        const std::string& nm = q->first;
        const size_t qi = q->second;
        s_min.setValue(nm, fil->m_total_ingredient[qi].m_min);
        s_max.setValue(nm, fil->m_total_ingredient[qi].m_max);
    }

    for (FunctionMap::const_iterator i = fmin.begin(); i != fmin.end(); i++)
    {
        const std::string& nm = i->first;
        const FuncParser::Function& f = i->second;
        s_min.setValue(nm, f.eval());
    }

    for (FunctionMap::const_iterator i = fmax.begin(); i != fmax.end(); i++)
    {
        const std::string& nm = i->first;
        const FuncParser::Function& f = i->second;
        s_max.setValue(nm, f.eval());
    }

    range.clear();
    for (TransferFunctionMap::const_iterator f = fmap_c.begin(); f != fmap_c.end(); f++)
    {
        const std::string& fnm = f->first;
        // add by @hira at 2016/12/01 ; key ckeck
        if (bmap_c.find(fnm) == bmap_c.end()) continue;
        const std::string& qnm = bmap_c.find(fnm)->second;
        const std::string tag = fnm + "_var_c";

        range.setValue(tag, s_min.getValue(qnm));
        range.setValue(tag, s_max.getValue(qnm));
    }

    for (TransferFunctionMap::const_iterator f = fmap_o.begin(); f != fmap_o.end(); f++)
    {
        const std::string& fnm = f->first;
        // add by @hira at 2016/12/01 ; key ckeck
        if (bmap_o.find(fnm) == bmap_o.end()) continue;
        // modify by @hira at 2016/12/01
        // const std::string& qnm = bmap_c.find(fnm)->second;
        const std::string& qnm = bmap_o.find(fnm)->second;
        const std::string tag = fnm + "_var_o";

        range.setValue(tag, s_min.getValue(qnm));
        range.setValue(tag, s_max.getValue(qnm));
    }

    return range;
}

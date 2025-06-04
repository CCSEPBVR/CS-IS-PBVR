#include <vismodule/PointObjectCreator> 


PointObjectCreator::PointObjectCreator() : m_volume(NULL), m_mpi_rank(0), m_mvp(NULL) 
{}

//~PointObjectCreator::PointObjectCreator()
//{
//}

vismodule::PointObject* PointObjectCreator::run( const Argument& param, const vismodule::Camera& camera, const int timeStep,  const int st )
{
    m_generator.setFinlterInfo( m_mvp );
    m_generator.setCoordSynthTS( st );

    struct stat s;
    if ( stat( param.m_input_data.c_str(), &s ) )
    {
        std::cout << "Error. read failed:" << param.m_input_data << std::endl;
        exit( 1 );
    }
    m_generator.createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step );

    vismodule::PointObject* po = m_generator.getPointObject();
    return po;
}

vismodule::PointObject* PointObjectCreator::run( const Argument& param, const vismodule::Camera& camera, const int timeStep, const int st, const int vl)
{
    m_generator.setFinlterInfo( m_mvp );
    m_generator.setCoordSynthTS( st );
    m_generator.createFromFile( param, camera, param.m_subpixel_level, param.m_sampling_step, st, vl );
    vismodule::PointObject* po = m_generator.getPointObject();
    return po;
}

void PointObjectCreator::setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss )
{
    m_xcSynthStr = xss;
    m_ycSynthStr = yss;
    m_zcSynthStr = zss;

    vismodule::CoordSynthesizerStrings css( 0, xss, yss, zss );
    m_generator.setCoordSynthStrs( css );
}

//2023 shimomura
//void setCoordSynthTkn( const EquationToken & xst, const EquationToken & yst, const EquationToken & zst )
void PointObjectCreator::setCoordSynthTkn( const jpv::ParticleTransferClientMessage::EquationToken & xst,
        const jpv::ParticleTransferClientMessage::EquationToken & yst,
        const jpv::ParticleTransferClientMessage::EquationToken & zst )
{
    //        m_xcSynthStr = xss;
    //        m_ycSynthStr = yss;
    //        m_zcSynthStr = zss;

    vismodule::EquationToken xst_tmp;
    vismodule::EquationToken yst_tmp;
    vismodule::EquationToken zst_tmp;

    for(int i=0; i<128; i++ )
    {
        xst_tmp.exp_token[i] = xst.exp_token[i];
        xst_tmp.var_name[i]  = xst.var_name[i] ;
        xst_tmp.val_array[i] = xst.value_array[i];
        yst_tmp.exp_token[i] = yst.exp_token[i];
        yst_tmp.var_name[i]  = yst.var_name[i] ;
        yst_tmp.val_array[i] = yst.value_array[i];
        zst_tmp.exp_token[i] = zst.exp_token[i];
        zst_tmp.var_name[i]  = zst.var_name[i] ;
        zst_tmp.val_array[i] = zst.value_array[i];
    }

    vismodule::CoordSynthesizerTokens cst(xst_tmp, yst_tmp, zst_tmp );
    m_generator.setCoordSynthTkns( cst );
}





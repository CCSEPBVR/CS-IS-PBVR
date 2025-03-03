#include "POLObjectGenerator.h"
//#include <sys/time.h>
#include "TransferFunction.h"
#include "UnstructuredVolumeObject.h"
#include "UnstructuredVolumeImporter.h"
//#include "CellByCellLayeredSampling.h"
#include <kvs/Camera>
#if 0 //TEST_DELETE
#include <kvs/TestVolume>
#include <kvs/FrontSTRFileReader>
#endif
#include <kvs/AVSUcd>
#include "common.h"
#include <kvs/ValueArray>
#include <kvs/File>

#include "FileChecker.h"
#include "StructuredVolumeObject.h"
#include "StructuredVolumeImporter.h"

#include "Argument.h"

#include "timer_simple.h"

using namespace pbvr;

//void POLObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera )
void POLObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide )
{
//FJ_TIMER_KAWAMURA
    PBVR_TIMER_STA( 260 );
//FJ_TIMER_KAWAMURA

    // add by shimomura 2023/0407
    pbvr::VolumeObjectBase* volume = nullptr;
    if ( kvsview::FileChecker::ImportableStructuredVolume( param.m_input_data ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new pbvr::StructuredVolumeImporter( param.m_input_data ); 
//        kvsMessageError("structured data does not apply." );

    } 
    else if ( kvsview::FileChecker::ImportableUnstructuredVolume( param.m_input_data))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new pbvr::UnstructuredVolumeImporter( param.m_input_data );  
    }
    else 
    {
        kvsMessageError("%s is not volume data.", param.m_input_data.c_str());
    }

//FJ_TIMER_KAWAMURA
    PBVR_TIMER_END( 260 );
//FJ_TIMER_KAWAMURA

    volume->updateMinMaxValues();
    //volume->setMinMaxValues( m_fi->m_min_value, m_fi->m_max_value );
//    volume->setMinMaxObjectCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );
//    volume->setMinMaxExternalCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    pbvr::VolumeObjectBase::VolumeType voltype = volume->volumeType();

    if(voltype ==  pbvr::VolumeObjectBase::VolumeType::Unstructured)
    {
        const pbvr::UnstructuredVolumeObject* uvo_p = static_cast<const pbvr::UnstructuredVolumeObject*>( volume );
        kvs::Vec3 start_point( clntMes.m_start_point[0], clntMes.m_start_point[1], clntMes.m_start_point[2] );
        kvs::Vec3 end_point( clntMes.m_end_point[0], clntMes.m_end_point[1], clntMes.m_end_point[2] );
        kvs::UnstructuredVolumeObject* vo_p = new kvs::UnstructuredVolumeObject();
        switch(volume -> cellType())
        {
            case pbvr::VolumeObjectBase::Tetrahedra:
                {
                    vo_p -> setCellType(kvs::VolumeObjectBase::Tetrahedra); 
                    break;
                }
            case pbvr::VolumeObjectBase::QuadraticTetrahedra:
                {
                    vo_p -> setCellType(kvs::VolumeObjectBase::QuadraticTetrahedra); 
                    break;
                }
            case pbvr::VolumeObjectBase::Hexahedra:
                {
                    vo_p -> setCellType(kvs::VolumeObjectBase::Hexahedra); 
                    break;
                }
            case pbvr::VolumeObjectBase::QuadraticHexahedra:
                {
                    vo_p -> setCellType(kvs::VolumeObjectBase::QuadraticHexahedra); 
                    break;
                }
            case pbvr::VolumeObjectBase::Prism:
                {
                    vo_p -> setCellType(kvs::VolumeObjectBase::Prism); 
                    break;
                }
            case pbvr::VolumeObjectBase::Pyramid:
                {
                    vo_p -> setCellType(kvs::VolumeObjectBase::Pyramid); 
                    break;
                }
            default:
                {
                    kvsMessageError( "Unsupported cell type." );
                    return;
                }
        }

        vo_p -> setNNodes( uvo_p->nnodes()); 
        vo_p -> setNCells( uvo_p->ncells()); 
        vo_p -> setCoords( uvo_p->coords()); 
        vo_p -> setVeclen( uvo_p->veclen()); 
        vo_p -> setValues( uvo_p->values()); 
        vo_p -> setConnections( uvo_p->connections()); 

        int plot_variable =  std::atoi(clntMes.m_plot_variable.substr(1).c_str()) -1;

        try
        {
            PlotOverLine plot_over_line(vo_p, clntMes.m_sampling_size, start_point, end_point, plot_variable);

            m_object->setValuesOnLine(plot_over_line.values()); 
            m_object->setXAxis(plot_over_line.xAxis()); 
            m_object->setMask(plot_over_line.mask()); 
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            //        m_object = NULL;
            delete volume;
            throw e;
        }
        delete vo_p;
    }
    else if(voltype ==  pbvr::VolumeObjectBase::VolumeType::Structured)
    {
#if 1
        const pbvr::StructuredVolumeObject* object = static_cast<const pbvr::StructuredVolumeObject*>( volume );
        kvs::Vec3 start_point( clntMes.m_start_point[0], clntMes.m_start_point[1], clntMes.m_start_point[2] );
        kvs::Vec3 end_point( clntMes.m_end_point[0], clntMes.m_end_point[1], clntMes.m_end_point[2] );
//        kvs::StructuredVolumeObject* vo_p = new kvs::StructuredVolumeObject();
//        vo_p -> setNNodes( vo_p->nnodes()); 
//        vo_p -> setNCells( vo_p->ncells()); 
//        vo_p -> setCoords( vo_p->coords()); 
//        vo_p -> setVeclen( vo_p->veclen()); 
//        vo_p -> setResolution( vo_p->resolution()); 
//        vo_p -> setGridType( vo_p->gridType()); 
//        vo_p -> setValues( vo_p->values()); 
//        vo_p -> setConnections( uvo_p->connections()); 

        int plot_variable =  std::atoi(clntMes.m_plot_variable.substr(1).c_str()) -1;

        try
        {
            PlotOverLine plot_over_line(object, clntMes.m_sampling_size, start_point, end_point, plot_variable);

            m_object->setValuesOnLine(plot_over_line.values()); 
            m_object->setXAxis(plot_over_line.xAxis()); 
            m_object->setMask(plot_over_line.mask()); 
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            //        m_object = NULL;
            delete volume;
            throw e;
        }
#endif
    }

    delete volume;
//    delete uvo_p;
}

#if 0
void POLObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera,const jpv::ParticleTransferClientMessage& clntMes,const int number_of_divide, const int st, const int vl )
{
    PBVR_TIMER_STA( 260 );
//    delete m_object;
    pbvr::UnstructuredVolumeObject* volume;
    volume = new pbvr::UnstructuredVolumeImporter( param.m_input_data );

    kvs::File ifpx( m_fi->m_file_path );
    std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();

    volume = new pbvr::UnstructuredVolumeImporter( path_base, m_fi->m_file_type, st, vl );

    PBVR_TIMER_END( 260 );

    volume->setMinMaxValues( m_fi->m_min_value, m_fi->m_max_value );
    volume->setMinMaxObjectCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    try
    {
        //m_object = sampling( param, camera, volume, subpixel_level, sampling_step );
        sampling( volume ,clntMes, number_of_divide);
    }
    catch ( const std::runtime_error& e )
    {
#ifdef _DEBUG		// debug by @hira
        printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
//        m_object = NULL;
        delete volume;
        throw e;
    }

    delete volume;
}
#endif

std::string POLObjectGenerator::getErrorMessage( const size_t maxMemory ) const
{
    std::string errorMessage( "" );
    //const size_t totalMemory = sizeof( float ) * m_object.values_on_line().size() + sizeof( float ) * m_object.values_on_line().size() + sizeof( bool ) * m_object.values_on_line().size();
    const size_t totalMemory = sizeof( float ) * m_object->values_on_line().size() + sizeof( float ) * m_object->values_on_line().size() + sizeof( bool ) * m_object->values_on_line().size();
    if ( totalMemory > maxMemory )
    {
        char ms[512];
        sprintf( ms, "Memory Error: TotalMemory = %zd, MaxMemory = %zd\n", totalMemory, maxMemory );
        errorMessage += ms;
    }
    return errorMessage;
}

const kvs::ValueArray<kvs::Real32>& POLObjectGenerator::coords( void ) const
{
    return( m_coords );
}

const kvs::ValueArray<kvs::UInt8>& POLObjectGenerator::colors( void ) const
{
    return( m_colors );
}

const kvs::ValueArray<kvs::Real32>& POLObjectGenerator::directions( void ) const
{
    return( m_directions );
}

const kvs::ValueArray<kvs::Real32>& POLObjectGenerator::sizes( void ) const
{
    return( m_sizes );
}

void POLObjectGenerator::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

void POLObjectGenerator::setColors( const kvs::ValueArray<kvs::UInt8>& colors )
{
    m_colors = colors;
}

void POLObjectGenerator::setDirections( const kvs::ValueArray<kvs::Real32>& directions )
{
    m_directions = directions;
}

void POLObjectGenerator::setSizes( const kvs::ValueArray<kvs::Real32>& sizes )
{
    m_sizes = sizes;
}

void POLObjectGenerator::clear()
{
    m_sizes.deallocate();
    m_directions.deallocate();
    m_coords.deallocate();
    m_colors.deallocate();
}



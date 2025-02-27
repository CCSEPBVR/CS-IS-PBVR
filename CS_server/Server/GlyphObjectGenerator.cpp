#include "GlyphObjectGenerator.h"
//#include <sys/time.h>
#include "TransferFunction.h"
#include "UnstructuredVolumeObject.h"
#include "UnstructuredVolumeImporter.h"
#include "CellByCellUniformSampling.h"
#include "CellByCellRejectionSampling.h"
#include "CellByCellMetropolisSampling.h"
//#include "CellByCellLayeredSampling.h"
#include <kvs/Camera>
#include "CellByCellUniformSampling.h"
#include "CellByCellRejectionSampling.h"
#include "CellByCellMetropolisSampling.h"
#include "CellByCellHistogram.h"
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

//void GlyphObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera )
void GlyphObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide )
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

   try
    {
        sampling( volume , clntMes, number_of_divide);
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

void GlyphObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera,const jpv::ParticleTransferClientMessage& clntMes,const int number_of_divide, const int st, const int vl )
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

std::string GlyphObjectGenerator::getErrorMessage( const size_t maxMemory ) const
{
    std::string errorMessage( "" );
//    const size_t totalMemory = sizeof( float ) * m_object->sizes().size() + sizeof( float ) * m_object->sizes().size()*3 + sizeof( char ) * m_object->sizes().size()*3+ sizeof( float ) * m_object->sizes().size()*3 ;
    const size_t totalMemory = sizeof( float ) * m_object.sizes().size() + sizeof( float ) * m_object.sizes().size()*3 + sizeof( char ) * m_object.sizes().size()*3+ sizeof( float ) * m_object.sizes().size()*3 ;
    if ( totalMemory > maxMemory )
    {
        char ms[512];
        sprintf( ms, "Memory Error: TotalMemory = %zd, MaxMemory = %zd\n", totalMemory, maxMemory );
        errorMessage += ms;
    }
    return errorMessage;
}

//void GlyphObjectGenerator::sampling( pbvr::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes )
void GlyphObjectGenerator::sampling( pbvr::VolumeObjectBase* volume,const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide )
{
#ifndef CPU_VER
    int rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
#endif

    std::cout << "Glyph Generating start " << std::endl;

    pbvr::VolumeObjectBase::VolumeType voltype = volume->volumeType();


    Type** values;
    kvs::AnyValueArray valueArray; 
    std::vector<float> coordinates; 
    int ncoords;
    std::vector<unsigned int> connections ;
    int ncells; 
    int nnodes;
    int nvariables;
    pbvr::VolumeObjectBase::CellType celltype;

    if(voltype ==  pbvr::VolumeObjectBase::VolumeType::Unstructured)
    {
        const pbvr::UnstructuredVolumeObject* uvo_p = static_cast<const pbvr::UnstructuredVolumeObject*>( volume );
       
        valueArray = volume->values(); 
        coordinates.assign( (float * )volume->coords().begin(),(float * )volume->coords().end()); 
        ncoords =  volume->nnodes();
        connections.assign((unsigned int*)uvo_p->connections().begin(), (unsigned int*)uvo_p->connections().end());
        ncells = uvo_p->ncells();
        nnodes = volume->nnodes();
        celltype = uvo_p->cellType();

        nvariables = volume->veclen();
        values = new Type * [nvariables];

        for ( int j = 0; j < nvariables; j++ )
        {
            values[j] = new float[nnodes];
            for ( int i = 0; i < nnodes; i++ )
            {
                int  it = j * nnodes  + i;
                values[j][i] = valueArray.at<Type>(it);  
            }
        } 
        GlyphGenerator glyph_generator( clntMes, number_of_divide, values, nvariables,
                coordinates.data(), ncoords, connections.data(), ncells, celltype);
        glyph_generator.getGlyphData(&m_object);
        
        for (int i = 0; i < nvariables; i++)
        {
            delete[] values[i];
        }
        delete[] values;


    }
    else if(voltype ==  pbvr::VolumeObjectBase::VolumeType::Structured)
    {
        const pbvr::StructuredVolumeObject* vo_p = static_cast<const pbvr::StructuredVolumeObject*>( volume );
            GlyphGenerator glyph_generator( clntMes, number_of_divide, *vo_p);
            glyph_generator.getGlyphData(&m_object);
    }

}

const kvs::ValueArray<kvs::Real32>& GlyphObjectGenerator::coords( void ) const
{
    return( m_coords );
}

const kvs::ValueArray<kvs::UInt8>& GlyphObjectGenerator::colors( void ) const
{
    return( m_colors );
}

const kvs::ValueArray<kvs::Real32>& GlyphObjectGenerator::directions( void ) const
{
    return( m_directions );
}

const kvs::ValueArray<kvs::Real32>& GlyphObjectGenerator::sizes( void ) const
{
    return( m_sizes );
}

void GlyphObjectGenerator::setCoords( const kvs::ValueArray<kvs::Real32>& coords )
{
    m_coords = coords;
}

void GlyphObjectGenerator::setColors( const kvs::ValueArray<kvs::UInt8>& colors )
{
    m_colors = colors;
}

void GlyphObjectGenerator::setDirections( const kvs::ValueArray<kvs::Real32>& directions )
{
    m_directions = directions;
}

void GlyphObjectGenerator::setSizes( const kvs::ValueArray<kvs::Real32>& sizes )
{
    m_sizes = sizes;
}

void GlyphObjectGenerator::clear()
{
    m_sizes.deallocate();
    m_directions.deallocate();
    m_coords.deallocate();
    m_colors.deallocate();
}



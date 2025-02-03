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

void GlyphObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera )
{
//FJ_TIMER_KAWAMURA
    PBVR_TIMER_STA( 260 );
//FJ_TIMER_KAWAMURA

    delete m_object;

    // add by shimomura 2023/0407
    pbvr::VolumeObjectBase* volume = nullptr;
    if ( kvsview::FileChecker::ImportableStructuredVolume( param.m_input_data ))
    {
        std::cout << "Structured !" <<std::endl;
        //volume = new pbvr::StructuredVolumeImporter( param.m_input_data ); 
        kvsMessageError("structured data does not apply." );

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

    //pbvr::UnstructuredVolumeObject* volume;
    //volume = new pbvr::UnstructuredVolumeImporter( param.m_input_data );
    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
        volume->setCoordSynthesizerTokens( m_coord_synthesizer_tokens );
    }

//FJ_TIMER_KAWAMURA
    PBVR_TIMER_END( 260 );
//FJ_TIMER_KAWAMURA
    
    // change by shimomura 20240730
    volume->updateMinMaxValues();
    //volume->setMinMaxValues( m_fi->m_min_value, m_fi->m_max_value );
    volume->setMinMaxObjectCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_fi->m_min_object_coord, m_fi->m_max_object_coord );

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

   try
    {
        //m_object = sampling( param, camera, volume, subpixel_level, sampling_step );
        m_object = sampling( volume );
    }
    catch ( const std::runtime_error& e )
    {
#ifdef _DEBUG		// debug by @hira
        printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
        m_object = NULL;
        delete volume;
        throw e;
    }

    delete volume;
}

void GlyphObjectGenerator::createFromFile( const Argument& param, const kvs::Camera& camera, const int st, const int vl )
{
    PBVR_TIMER_STA( 260 );
    delete m_object;
    pbvr::UnstructuredVolumeObject* volume;
    volume = new pbvr::UnstructuredVolumeImporter( param.m_input_data );

    kvs::File ifpx( m_fi->m_file_path );
    std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();

    volume = new pbvr::UnstructuredVolumeImporter( path_base, m_fi->m_file_type, st, vl );
    if ( volume )
    {
        volume->setCoordSynthesizerStrings( m_coord_synthesizer_strings );
        volume->setCoordSynthesizerTokens( m_coord_synthesizer_tokens );
    }

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
        m_object = sampling( volume );
    }
    catch ( const std::runtime_error& e )
    {
#ifdef _DEBUG		// debug by @hira
        printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
        m_object = NULL;
        delete volume;
        throw e;
    }

    delete volume;
}

std::string GlyphObjectGenerator::getErrorMessage( const size_t maxMemory ) const
{
    std::string errorMessage( "" );
    const size_t totalMemory = sizeof( float ) * m_object->sizes().size() + sizeof( float ) * m_object->sizes().size()*3 + sizeof( char ) * m_object->sizes().size()*3+ sizeof( float ) * m_object->sizes().size()*3 ;
    if ( totalMemory > maxMemory )
    {
        char ms[512];
        sprintf( ms, "Memory Error: TotalMemory = %zd, MaxMemory = %zd\n", totalMemory, maxMemory );
        errorMessage += ms;
    }
    return errorMessage;
}

//pbvr::KVSMLObjectGlyph* GlyphObjectGenerator::sampling( const Argument& param, const kvs::Camera& camera, pbvr::VolumeObjectBase* volume, const size_t subpixel_level, const float sampling_step )
kvs::KVSMLObjectGlyph* GlyphObjectGenerator::sampling( pbvr::VolumeObjectBase* volume )
{
#ifndef CPU_VER
    int rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
#endif

    std::cout << "Glyph Generating start " << std::endl;

    const pbvr::UnstructuredVolumeObject* uvo_p = static_cast<const pbvr::UnstructuredVolumeObject*>( volume );
    kvs::AnyValueArray valueArray = volume->values(); 
    float* coordinates =  (float * )volume->coords().pointer(); 
    int ncoords =  volume->nnodes();
    unsigned int* connections =  (unsigned int*)uvo_p->connections().pointer();
    int ncells = uvo_p->ncells();
    int nnodes = volume->nnodes();
   
    pbvr::VolumeObjectBase::CellType celltype = uvo_p->cellType();

    const int nvariables = volume->veclen();
    //Type*  values[nvariables];
    Type** values;
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
        
    GlyphGenerator glyph_generator( values, nvariables,
            coordinates, ncoords, connections, ncells, celltype);
 
    return glyph_generator.getGlyphData();
}

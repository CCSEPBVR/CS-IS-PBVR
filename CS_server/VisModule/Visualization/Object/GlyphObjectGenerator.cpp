#include "GlyphObjectGenerator.h"
//#include <sys/time.h>
#include <vismodule/TransferFunction>
#include "UnstructuredVolumeObject.h"
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
//#include "CellByCellLayeredSampling.h"
#include <vismodule/Camera>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellHistogram>
#if 0 //TEST_DELETE
#include <vismodule/TestVolume>
#include <vismodule/FrontSTRFileReader>
#endif
#include <vismodule/AVSUcd>
#include <vismodule/ValueArray>
#include <vismodule/File>

#include <vismodule/FileChecker>
#include "StructuredVolumeObject.h"
#include <vismodule/StructuredVolumeImporter>

#include <vismodule/Argument>

#include <vismodule/timer_simple>

using namespace vismodule;

//void GlyphObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera )
void GlyphObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide )
{
//FJ_TIMER_KAWAMURA
    VIS_MODULE_TIMER_STA( 260 );
//FJ_TIMER_KAWAMURA

    // add by shimomura 2023/0407
    vismodule::VolumeObjectBase* volume = nullptr;
    if ( vismoduleview::FileChecker::ImportableStructuredVolume( param.m_input_data ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new vismodule::StructuredVolumeImporter( param.m_input_data ); 
//        visModuleMessageError("structured data does not apply." );
        // change by shimomura 20240730
        int id = param.m_subvolume_id;
        //volume->updateMinMaxValues();
        //volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
//        volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[id], m_mvp->m_max_subvolume_coord[id] );
//        volume->setMinMaxExternalCoords( m_mvp->m_min_subvolume_coord[id], m_mvp->m_max_subvolume_coord[id] );

    } 
    else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( param.m_input_data))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );  
        
        volume->updateMinMaxValues();
        //volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
        //volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
        //volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );

    }
    else 
    {
        visModuleMessageError("%s is not volume data.", param.m_input_data.c_str());
    }

//FJ_TIMER_KAWAMURA
    VIS_MODULE_TIMER_END( 260 );
//FJ_TIMER_KAWAMURA

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

void GlyphObjectGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes,const int number_of_divide, const int st, const int vl )
{
    VIS_MODULE_TIMER_STA( 260 );
//    delete m_object;

    size_t found_kvsml = param.m_input_data_base.find(".kvsml");
    size_t found_vtm = param.m_input_data_base.find(".vtm");
    size_t found_vtu = param.m_input_data_base.find(".vtu");
    size_t found_vti = param.m_input_data_base.find(".vti");

    vismodule::VolumeObjectBase* volume = nullptr;

    if ( found_kvsml != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );
    
        vismodule::File ifpx( m_mvp->m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
    
        volume = new vismodule::UnstructuredVolumeImporter( path_base, m_mvp->m_file_type, st, vl );
    }
#ifdef EXTEND_FILE_FORMAT
    else if ( found_vtm != std::string::npos )
    {
        // structured
        if( m_mvp->m_file_type == 3 )
        {
            volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
        }
        // unstructured
        if( m_mvp->m_file_type == 4 )
        {
            volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
        }
    }
    else if ( found_vtu != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
    }
    else if ( found_vti != std::string::npos )
    {
        volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
    }
#endif

    VIS_MODULE_TIMER_END( 260 );

    volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
    volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
    volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );

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

//void GlyphObjectGenerator::sampling( vismodule::VolumeObjectBase* volume, const jpv::ParticleTransferClientMessage& clntMes )
void GlyphObjectGenerator::sampling( vismodule::VolumeObjectBase* volume,const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide )
{
#ifndef CPU_VER
    int rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
#endif

    std::cout << "Glyph Generating start " << std::endl;

    vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();


    Type** values;
    vismodule::AnyValueArray valueArray; 
    std::vector<float> coordinates; 
    int ncoords;
    std::vector<unsigned int> connections ;
    int ncells; 
    int nnodes;
    int nvariables;
    vismodule::VolumeObjectBase::CellType celltype;

    if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Unstructured)
    {
        const vismodule::UnstructuredVolumeObject* uvo_p = static_cast<const vismodule::UnstructuredVolumeObject*>( volume );
       
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
    else if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Structured)
    {
        const vismodule::StructuredVolumeObject* vo_p = static_cast<const vismodule::StructuredVolumeObject*>( volume );
            GlyphGenerator glyph_generator( clntMes, number_of_divide, *vo_p);
            glyph_generator.getGlyphData(&m_object);
    }

}

const vismodule::ValueArray<vismodule::Real32>& GlyphObjectGenerator::coords( void ) const
{
    return( m_coords );
}

const vismodule::ValueArray<vismodule::UInt8>& GlyphObjectGenerator::colors( void ) const
{
    return( m_colors );
}

const vismodule::ValueArray<vismodule::Real32>& GlyphObjectGenerator::directions( void ) const
{
    return( m_directions );
}

const vismodule::ValueArray<vismodule::Real32>& GlyphObjectGenerator::sizes( void ) const
{
    return( m_sizes );
}

void GlyphObjectGenerator::setCoords( const vismodule::ValueArray<vismodule::Real32>& coords )
{
    m_coords = coords;
}

void GlyphObjectGenerator::setColors( const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    m_colors = colors;
}

void GlyphObjectGenerator::setDirections( const vismodule::ValueArray<vismodule::Real32>& directions )
{
    m_directions = directions;
}

void GlyphObjectGenerator::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
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



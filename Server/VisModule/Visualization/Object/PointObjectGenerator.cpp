#include <vismodule/PointObjectGenerator>
//#include <sys/time.h>
#include <vismodule/TransferFunction>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/UnstructuredVolumeImporter>
//#include "CellByCellLayeredSampling.h"
#include <vismodule/Camera>
#include <vismodule/CellByCellUniformSampling>
#include <vismodule/CellByCellRejectionSampling>
#include <vismodule/CellByCellMetropolisSampling>
#include <vismodule/CellByCellHistogram>
#include <vismodule/CellByCellMinMax>
#include <vismodule/CellByCellParticleGenerator>
#if 0 //TEST_DELETE
#include <vismodule/TestVolume>
#include <vismodule/FrontSTRFileReader>
#endif
#include <vismodule/AVSUcd>
#include <vismodule/timer_simple>
#include <vismodule/ValueArray>
#include <vismodule/File>

#include <vismodule/FileChecker>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/StructuredVolumeImporter>

using namespace vismodule;

vismodule::PointObject* PointObjectGenerator::GenerateParticleStruct(
    const ParticleProperty& particle_property,
    const domain_parameters_struct& dom,
    Type** values,
    const int nvariables,
    ServerMode server_mode
)
{
    vismodule::CoordSynthesizerStrings* css;

    if ( server_mode == ServerMode::CS )
    {
        std::string xss = particle_property.m_x_synthesis;
        std::string yss = particle_property.m_y_synthesis;
        std::string zss = particle_property.m_z_synthesis;
        css = new vismodule::CoordSynthesizerStrings( 0, xss, yss, zss );
    }
    else // jpv::ServerMode::IS
    {
        css = NULL;
    }

    // NamedTransferFunctionをTransferFunctionにupcast
    std::vector<vismodule::TransferFunction> transfunc_array;
    for ( const auto& named_tf : particle_property.m_transfunc_array )
    {
        transfunc_array.push_back( static_cast<const vismodule::TransferFunction&>(named_tf) );
    }

#ifdef CPU_SAMPLING_TIME
    std::cout << std::endl << "CPU - ";
#else
    std::cout << std::endl << "GPU - ";
#endif

    switch ( particle_property.m_sampling_method )
    {
    case 'u':
        std::cout << "Uniform sampling" << std::endl;
        return new vismodule::CellByCellUniformSampling(
            dom,
            values,
            nvariables,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer, 
            css
        );
    case 'r':
        std::cout << "Rejection sampling" << std::endl;
        return new vismodule::CellByCellRejectionSampling(
            dom,
            values,
            nvariables,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer, 
            css
        );
    case 'm':
        std::cout << "Metropolis sampling" << std::endl;
        return new vismodule::CellByCellMetropolisSampling(
            dom,
            values,
            nvariables,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );
    case 'h':
        std::cout << "Histogram " << std::endl;
        return new vismodule::CellByCellHistogram(
            dom,
            values,
            nvariables,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );
    case 'x':
        std::cout << "MinMax " << std::endl;
        return new vismodule::CellByCellMinMax(
            dom,
            values,
            nvariables,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );


    default:
        std::cout << "Unknown sampling method:" << particle_property.m_sampling_method << std::endl;
        exit( 1 );
        return 0;
    }
}

vismodule::PointObject* PointObjectGenerator::GenerateParticleUnstruct(
    const ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    ServerMode server_mode
)
{
    vismodule::CoordSynthesizerStrings* css;

    if ( server_mode == ServerMode::CS )
    {
        std::string xss = particle_property.m_x_synthesis;
        std::string yss = particle_property.m_y_synthesis;
        std::string zss = particle_property.m_z_synthesis;
        css = new vismodule::CoordSynthesizerStrings( 0, xss, yss, zss );
    }
    else // jpv::ServerMode::IS
    {
        css = NULL;
    }

    // NamedTransferFunctionをTransferFunctionにupcast
    std::vector<vismodule::TransferFunction> transfunc_array;
    for ( const auto& named_tf : particle_property.m_transfunc_array )
    {
        transfunc_array.push_back( static_cast<const vismodule::TransferFunction&>(named_tf) );
    }

#ifdef CPU_SAMPLING_TIME
    std::cout << std::endl << "CPU - ";
#else
    std::cout << std::endl << "GPU - ";
#endif

    switch ( particle_property.m_sampling_method )
    {
    case 'u':
        std::cout << "Uniform sampling" << std::endl;
        return new vismodule::CellByCellUniformSampling(
            dom,
            values,
            nvariables,
            coordinates,
            ncoords,
            connections,
            ncells,
            celltype,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );
    case 'r':
        std::cout << "Rejection sampling" << std::endl;
        return new vismodule::CellByCellRejectionSampling(
            dom,
            values,
            nvariables,
            coordinates,
            ncoords,
            connections,
            ncells,
            celltype,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );
    case 'm':
        std::cout << "Metropolis sampling" << std::endl;
        return new vismodule::CellByCellMetropolisSampling(
            dom,
            values,
            nvariables,
            coordinates,
            ncoords,
            connections,
            ncells,
            celltype,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );
    case 'h':
        std::cout << "Histogram " << std::endl;
        return new vismodule::CellByCellHistogram( 
            dom,
            values,
            nvariables,
            coordinates,
            ncoords,
            connections,
            ncells,
            celltype,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );
    case 'x':
        std::cout << "MinMax " << std::endl;
        return new vismodule::CellByCellMinMax( 
            dom,
            values,
            nvariables,
            coordinates,
            ncoords,
            connections,
            ncells,
            celltype,
            transfunc_array[0],
            transfunc_array,
            particle_property.m_transfunc_synthesizer,
            css
        );

    default:
        std::cout << "Unknown sampling method:" << particle_property.m_sampling_method << std::endl;
        exit( 1 );
        return 0;
    }
}

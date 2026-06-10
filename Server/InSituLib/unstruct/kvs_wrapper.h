#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include <vismodule/VolumeObjectBase>
#include <vismodule/NameListFile>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/ParticleProperty>

#ifdef EXTEND_FILE_FORMAT
#include <vtkUnstructuredGrid.h>
#endif

#ifdef DOUBLE_SCHEME
    typedef double Type;
#else
    typedef float Type;
#endif

typedef unsigned char Byte;

#ifdef __cplusplus
extern "C" {
#endif

void OutputCoordMinMaxFile(
    const domain_parameters_unstruct& dom,
    const std::string& coordMinMaxFilePath
);

bool SetParticleParameter( 
    const domain_parameters_unstruct& dom,
    const std::string& tfFilePath,
    const std::string& tfFilePath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables,
    bool& object_generation_enabled
);

void generate_particles(
    int time_step,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltypes
);

#ifdef EXTEND_FILE_FORMAT
void generate_particles_vtk(
    int time_step,
    vtkUnstructuredGrid* ucd
);

void SetDomain(
    vtkUnstructuredGrid* ucd,
    domain_parameters_unstruct* dom
);
#endif

#ifdef __cplusplus
}
#endif

#endif

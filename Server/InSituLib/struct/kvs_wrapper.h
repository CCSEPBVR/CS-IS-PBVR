#ifndef KVS_WRAPPER_H_INCLUDED
#define KVS_WRAPPER_H_INCLUDED

#include <vismodule/Argument>
#include <vismodule/NameListFile>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/ParticleProperty>

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
    const domain_parameters_struct& dom,
    const std::string& coordMinMaxFilePath
);

void generate_particles(
    int time_step,
    domain_parameters_struct dom,
    Type** volume_data,
    int nvariables
);

bool ensemble_generate_particles(
    int time_step,
    const int num_ensemble,
    domain_parameters_struct dom,
    Type** volume_data,
    int nvariables
);

bool SetParticleParameter( 
    const domain_parameters_struct& dom,
    const std::string& tfJsonPath,
    const std::string& tfJsonPath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables,
    bool& object_generation_enabled
);

#ifdef __cplusplus
}
#endif

#endif

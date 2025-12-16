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

bool generate_particles(
    int time_step,
    domain_parameters_struct dom,
    Type** volume_data,
    int nvariables
);

bool SetParticleParameter( 
    const domain_parameters_struct& dom,
    const std::string& tfFilePath,
    const std::string& tfFilePath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    NameListFile& nameListFile
);

#ifdef __cplusplus
}
#endif

#endif

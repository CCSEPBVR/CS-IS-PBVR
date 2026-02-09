#ifndef VIS_MODULE_INITIAL_STEP_H_INCLDE
#define VIS_MODULE_INITIAL_STEP_H_INCLDE

#include <vismodule/ParticleProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/AnyValueArray>
#include <vismodule/VolumeObjectBase>
#include <vismodule/PointObject>
#include <vismodule/TransferFunctionSynthesizer>

#include <kvs/PointObject>

bool SetDefaultParticleParameterCS(
    const std::string& transfer_function_file_name,
    const MultiVolumePropertyList& mvpl,
    ParticleProperty& particle_property
);

void InitialStepCS(
    std::string& file_path,
    const int time_step, 
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl
);

void SetDefaultParticleParameterIS(
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl
);

void InitialStepIS(
    const int time_step,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl
);

#endif

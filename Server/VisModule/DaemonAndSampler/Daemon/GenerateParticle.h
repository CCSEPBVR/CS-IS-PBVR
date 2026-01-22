#ifndef VIS_MODULE_GENERATE_STEP_H_INCLDE
#define VIS_MODULE_GENERATE_STEP_H_INCLDE

#include <vismodule/ParticleProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/AnyValueArray>
#include <vismodule/VolumeObjectBase>
#include <vismodule/PointObject>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/CellByCellParticleGenerator>

#include <kvs/PointObject>

void GenerateParticleCS(
    std::string& file_path,
    const int time_step, 
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    std::unique_ptr<kvs::PointObject>& point_object
);

void GenerateParticleIS(
    const int time_step,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    std::unique_ptr<kvs::PointObject>& point_object
);

void generate_volume(
    const std::string& file_path,
    const MultiVolumeProperty& mvp,
    vismodule::VolumeObjectBase*& volume
);

void generate_volume(
    const std::string& file_path,
    const MultiVolumeProperty& mvp,
    const int time_step,
    const int sub_volume_id,
    vismodule::VolumeObjectBase*& volume
);

void store_volume_in_variables_array_common(
    vismodule::VolumeObjectBase* volume,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
);

void store_volume_in_variables_array_struct(
    const vismodule::VolumeObjectBase* volume,
    domain_parameters_struct& dom,
    std::array<int, 3>& resolution,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
);

void store_volume_in_variables_array_unstruct(
    const vismodule::VolumeObjectBase* volume,
    domain_parameters_unstruct& dom,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    std::unique_ptr<float[]>& coordinates,
    int& ncoords,
    std::unique_ptr<unsigned int[]>& connections,
    int& ncells,
    vismodule::VolumeObjectBase::CellType& celltype
);

template <typename T>
void copy_values(
    vismodule::AnyValueArray& valueArray, std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int nvariables,
    int nnodes
);

void MakeHistgram(
    const vismodule::PointObject* point_object,
    const int tf_number,
    vismodule::UInt64* c_bins,
    vismodule::UInt64* o_bins
);

void MakeParticleMinMax(
    const TransferFunctionSynthesizer* transfer_function_synthesizer,
    const int tf_number,
    float* max_array,
    float* min_array
);
  
#endif

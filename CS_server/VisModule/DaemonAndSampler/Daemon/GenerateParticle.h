#ifndef VIS_MODULE_GENERATE_STEP_H_INCLDE
#define VIS_MODULE_GENERATE_STEP_H_INCLDE

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/Connect>
#include <vismodule/CS_PointObjectGenerator>

#include <vismodule/Argument>

#include <vismodule/AVSField>
#include <vismodule/Timer>
#include <vismodule/KVSMLObjectPointWriter>
#include <vismodule/JobDispatcher>
#ifndef CPU_VER
#include <vismodule/JobCollector>
#endif
#include <vismodule/MultiVolumeProperty>
#include <vismodule/TransferFunctionProperty>
#ifndef CPU_VER
#include "mpi.h"
#endif

#include <cassert>
#include <signal.h> /* 140319 for client stop by Ctrl+c */
#include <sys/stat.h>
#if (defined(VIS_MODULE_PLATFORM_LINUX) || defined(VIS_MODULE_PLATFORM_MACOSX))
#include <execinfo.h>
#endif
#include <vismodule/File>

#include <vismodule/ExtendedTransferFunction>
#include <vismodule/TransferFunctionSynthesizerCreator>
#include "VariableRange.h"

#include <vismodule/timer_simple>

#include <vismodule/Compiler>
#ifdef VIS_MODULE_COMPILER_VC
#include <direct.h>
#define mkdir( dir, mode ) _mkdir( dir )
#endif

#include <vismodule/FileChecker>
#include <vismodule/UnstructuredVolumeImporter>
#include <vismodule/StructuredVolumeImporter>
#include <vismodule/CellByCellParticleGenerator>

#include <vismodule/GlyphSeedGenerator>

//plot over line
#include <vismodule/PlotOverLineGenerator>

#include <vismodule/Calculate>
#include <vismodule/SignalHandler>
#include <vismodule/Math>
//IS
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>
#include <vismodule/ParameterFileReader>
#include <vismodule/SetDefaultTransferFunction>
#include <DaemonAndSampler/Daemon/Timer.h>



void generate_particle(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    // std::vector<PointObjectCreator>& point_creator_lst, 
    bool &nan_error, 
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    jpv::ServerMode server_mode,
    jpv::InitializeParameter init_param
);

void generate_volume(
    const Argument& param,
    const MultiVolumeProperty& mvp,
    const int time_step,
    vismodule::VolumeObjectBase*& volume
);

void generate_volume(
    const Argument &param,
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
    const vismodule::UInt64* c_bins,
    const vismodule::UInt64* o_bins
);

void MakeParticleMinMax(
    const TransferFunctionSynthesizer* transfer_function_synthesizer,
    const int tf_number,
    float* max_array,
    float* min_array
);

void generate_particle_worker(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    // std::vector<PointObjectCreator>& point_creator_lst,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    TransferFunctionSynthesizerCreator transfunc_creator,
    int& timer_count
);

void generate_particle_IS(
    jpv::ParticleTransferServerMessage& servMes,
    Argument &param,
    MultiVolumePropertyList& mvpl,
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts
);
  
#endif

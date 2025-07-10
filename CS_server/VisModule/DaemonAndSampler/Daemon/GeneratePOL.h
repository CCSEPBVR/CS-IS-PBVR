#ifndef VIS_MODULE_GENERATE_PLOT_OVER_LINE_H_INCLDE
#define VIS_MODULE_GENERATE_PLOT_OVER_LINE_H_INCLDE

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/PointObjectGenerator>

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

#include <vismodule/Calculate>
#include <vismodule/PointObjectCreator>
#include <vismodule/SignalHandler>
#include <vismodule/Math>
#include <vismodule/PlotOverLineGenerator>
//IS
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>
#include <vismodule/ParameterFileReader>
#include <vismodule/SetDefaultTransferFunction>
#include <DaemonAndSampler/Daemon/Timer.h>



void generate_plot_over_line_master(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, 
#ifndef CPU_VER
                         JobCollector& jc,
#endif
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, bool& useAllNodes, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param );

void generate_plot_over_line_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error,  
#ifndef CPU_VER
                         JobCollector& jc,
#endif
                         JobDispatcher& jd, bool& useAllNodes, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param);

void generate_plot_over_line_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, ParticleMonitor& pm, vismodule::Timer& timer,
                         std::string particlePath, std::string tfFilePath, std::string tfFilePath_old,
                         bool& useAllNodes, int& timer_count , const jpv::InitializeParameter init_param );


#endif

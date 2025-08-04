#ifndef VIS_MODULE_INITIAL_STEP_H_INCLDE
#define VIS_MODULE_INITIAL_STEP_H_INCLDE

#include "ParticleTransferServer.h"
#include "ParticleTransferProtocol.h"

#include <vismodule/PointObject>
#include <vismodule/CommandLine>
#include <vismodule/Camera>
#include <vismodule/TransferFunction>
#include <vismodule/Matrix33>
#include <vismodule/RotationMatrix33>

#include <vismodule/timer_simple>

#include <vismodule/CS_PointObjectGenerator>

#include <vismodule/Argument>

#include <vismodule/AVSField>
#include <vismodule/Timer>
#include <vismodule/KVSMLObjectPointWriter>
//#include "KVSMLObjectPointMPIWriter.h"
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
//#include <vismodule/PointObjectCreator>
//#include <vismodule/PointObjectGenerator>
#include <vismodule/CS_PointObjectGenerator>
#include <vismodule/SignalHandler>
#include <vismodule/Math>
//IS
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>
#include <vismodule/ParameterFileReader>
#include <vismodule/SetDefaultTransferFunction>
#include <DaemonAndSampler/Daemon/Timer.h>


void initial_step_master(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         //bool &nan_error, std::vector<PointObjectCreator>& point_creator_lst, 
                         bool &nan_error,
#ifndef CPU_VER
                         JobCollector& jc, 
#endif
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count );

void initial_step_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         //bool &nan_error, std::vector<PointObjectGenerator>& point_creator_lst, 
                         bool &nan_error,
#ifndef CPU_VER
                         JobCollector& jc, 
#endif
                         JobDispatcher& jd, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count );


void initial_step_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         std::string particlePath, std::string glyphFilePath, std::string plotOverLineFilePath, std::string statePath, std::string  historyPath, std::string tfFilePath_old,
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, int& timer_count );


#endif

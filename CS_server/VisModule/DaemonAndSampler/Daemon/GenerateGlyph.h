//void generate_glyph_master() 
#include <vismodule/Argument>
#include "ParticleTransferProtocol.h"
#include <vismodule/MultiVolumeProperty>
#include <vismodule/TransferFunction>
#include <vismodule/JobDispatcher>
#ifndef CPU_VER
#include <vismodule/JobCollector>
#endif
#include <vismodule/PointObjectCreator>
#include "ParticleTransferServer.h"
#include <vismodule/TransferFunctionSynthesizerCreator>
#include <vismodule/Connect>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/Calculate>
//IS
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileWriter>
#include <vismodule/ParameterFileReader>
#include <vismodule/SetDefaultTransferFunction>
#include <DaemonAndSampler/Daemon/Timer.h>


void generate_glyph(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc,
#endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    jpv::ServerMode server_mode
);

void generate_glyph_worker(Argument &param, jpv::ParticleTransferClientMessage& clntMes, MultiVolumePropertyList& mvpl, 
                         bool &nan_error, 
#ifndef CPU_VER
                         JobCollector& jc,
#endif
                         JobDispatcher& jd, TransferFunctionSynthesizerCreator transfunc_creator , int& timer_count , const jpv::InitializeParameter init_param);

void generate_glyph_IS(Argument &param, jpv::ParticleTransferClientMessage& clntMes, jpv::ParticleTransferServerMessage& servMes, MultiVolumePropertyList& mvpl, 
                         JobDispatcher& jd,  jpv::ParticleTransferServer pts, ParticleMonitor& pm, vismodule::Timer& timer,
                         std::string particlePath, std::string GlyphFilePath, std::string GlyphFilePath_old,
                          int& timer_count , const jpv::InitializeParameter init_param );


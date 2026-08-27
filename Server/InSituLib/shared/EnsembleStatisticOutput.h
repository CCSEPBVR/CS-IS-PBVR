#ifndef PBVR_ENSEMBLE_STATISTIC_OUTPUT_H
#define PBVR_ENSEMBLE_STATISTIC_OUTPUT_H

// アンサンブルPBVR 統計量出力の分離ファイル(step3)。
// ave/var/cov 粒子出力・統計 history 出力を kvs_wrapper.cpp から切り出し公開。
// ヘッダ依存は EnsembleStatisticOutput.h -> EnsembleParticleGenerator.h の一方向のみ。

#include <string>
#include <vector>
#include <vismodule/ParticleProperty>
#include <vismodule/MultiVolumeProperty>

#include "EnsembleParticleGenerator.h"   // EnsembleStatisticRange, Byte, CPU_VER/MPI handling

namespace pbvr
{

std::string EnsembleParticleFilePrefix(
    const std::string& particleFilePrefix,
    const std::string& statisticPrefix );

void OutputEnsembleStatisticParticles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string& particleFilePrefix,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals );

void OutputEnsembleStatisticHistory(
    ParticleProperty& particle_property,
    const int tf_number,
    const int nvariables,
    const std::string& historyFilePath,
    vismodule::EnsembleStatisticRange average_range,
    vismodule::EnsembleStatisticRange variance_range,
    vismodule::EnsembleStatisticRange co_variation_range,
    MPI_Comm ensemble_comm = MPI_COMM_WORLD );

} // namespace pbvr

#endif // PBVR_ENSEMBLE_STATISTIC_OUTPUT_H

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

/*  アンサンブルメンバー1つ分の粒子を出力する。
 *
 *  OutputEnsembleStatisticParticles との違いは2点だけである。
 *    ・コミュニケータを「ノード単位」ではなく「メンバーID単位」で作る
 *    ・連番・総数を 1/1 に固定する
 *
 *  メンバーの区別はディレクトリ名（PARTICLE_DIR/ensNNNN/）で行い、ファイル名の
 *  接頭辞は通常版と同じ "t_" にする。これにより読み出し側（ParticleFile）の
 *  名前解析が無改造で使える。particleFilePrefix には
 *  PrepareEnsembleMemberDirectory() の戻り値を渡すこと。
 *
 *  全プロセスがこの関数を呼ぶこと。集団通信を含むため一部だけが呼ぶと停止する。 */
void OutputEnsembleMemberParticles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string& particleFilePrefix,
    const int my_ens_id,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals );

/*  メンバー用の出力先 "<PARTICLE_DIR>/ensNNNN/" を作り、ファイル接頭辞
 *  "<PARTICLE_DIR>/ensNNNN/t_" を返す。
 *  particleFilePrefix には通常版の "<PARTICLE_DIR>/t_" を渡す。
 *  ディレクトリ作成は my_ens_rank==0 のプロセスだけが行う（競合を避けるため）。 */
std::string PrepareEnsembleMemberDirectory(
    const std::string& particleFilePrefix,
    const int my_ens_id,
    const int my_ens_rank );

void OutputEnsembleStatisticHistory(
    ParticleProperty& particle_property,
    const int tf_number,
    const int nvariables,
    const std::string& historyFilePath,
    vismodule::EnsembleStatisticRange average_range,
    vismodule::EnsembleStatisticRange variance_range,
    vismodule::EnsembleStatisticRange co_variation_range,
    MPI_Comm ensemble_comm = MPI_COMM_WORLD,
    /*  アンサンブルメンバー数。>0 のとき history json に num_ensemble として出す。
     *  クライアントが表示対象メンバーの選択肢を作るのに使う。
     *  comm のサイズでは代用できない（ensemble_comm には MPI_COMM_WORLD が渡るため）。 */
    const int num_ensemble = 0 );

} // namespace pbvr

#endif // PBVR_ENSEMBLE_STATISTIC_OUTPUT_H

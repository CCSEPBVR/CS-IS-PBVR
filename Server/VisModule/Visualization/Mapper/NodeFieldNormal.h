/*****************************************************************************/
/**
 *  @file   NodeFieldNormal.h
 *  @brief  方式C（節点F場法）による法線計算。
 *
 *  アンサンブルPBVR（v360 の InSituLib/shared/EnsembleParticleGenerator.cpp）に
 *  実装済みのものを一様サンプリングへ移植した。閾値による自動切り替えは移植していない
 *  （方式は環境変数 PBVR_NORMAL_METHOD で選ぶ）。
 *
 *  現行方式は不透明度を粒子ごとに 7 点（中心＋±eps×3軸）で評価して差分を取る
 *  （座標差分法）。本方式は代わりに
 *
 *    1. 節点で微分量 grad q を復元する（AGS 法。セル数に比例する固定費）
 *    2. 節点で不透明度 O を評価して「節点F場」を作る（節点数に比例する固定費）
 *    3. 粒子では F 場を 1 回補間して勾配を取るだけ
 *
 *  とする。粒子あたりの仕事が「補間1回＋勾配1回」になるので、粒子数がセル数より
 *  十分多いときに有利になる。
 */
/*****************************************************************************/
#ifndef PBVR__VISMODULE__NODE_FIELD_NORMAL_H_INCLUDE
#define PBVR__VISMODULE__NODE_FIELD_NORMAL_H_INCLUDE

#include <vector>
#include <vismodule/Vector3>
#include <vismodule/VolumeObjectBase>
#include <vismodule/CellBase>
#include <vismodule/TransferFunction>
#include "TransferFunctionSynthesizer.h"

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  法線計算の方式。
 *
 *  coorddiff が現行方式（±eps の座標差分）、nodefield が方式C。
 */
/*===========================================================================*/
enum NormalMethod
{
    NormalMethodCoordDiff = 0,
    NormalMethodNodeField = 1   // 既定
};

/*  環境変数 PBVR_NORMAL_METHOD から方式を読む。既定は方式C。
 *      nodefield (既定) / coorddiff
 *
 *  方式C が使えない場合（対応外のセル種・節点局所座標表の検査失敗・節点場の構築失敗）は
 *  呼び出し側が coorddiff へ退避する。 */
NormalMethod ResolveNormalMethod();
const char*  NormalMethodName( const NormalMethod method );

/*===========================================================================*/
/**
 *  @brief  数式が参照している微分量(dq)の一覧。
 *
 *  slot は (変数, 成分) ごとの節点微分量の格納位置。参照されていなければ -1。
 *  var_index は復元に必要な変数の番号を昇順に並べたもの。
 *
 *  成分単位で持つのは散布と格納を減らすため。grad_ary() は 3 成分を同時に作るので
 *  計算そのものは成分単位では省けず、変数単位でしか飛ばせない。
 */
/*===========================================================================*/
struct NodeDqUsage
{
    std::vector<int> slot;        // [v*3+c] -> 格納位置。未参照は -1
    std::vector<int> var_index;   // 復元が要る変数の番号（昇順）
    int nslots;

    NodeDqUsage() : nslots( 0 ) {}
    bool any() const { return nslots > 0; }
};

/*  不透明度の変数式すべてを走査して、参照されている微分量を集める。 */
NodeDqUsage CollectNodeDqUsage(
    const std::vector< ::EquationToken >& exprs, const int nvariables );

/*  セル種に応じた補間器を 1 つ生成する（節点F場用）。対応外のセル種では NULL。 */
CellBase<Type>* CreateCellForCelltype(
    const VolumeObjectBase::CellType& celltype,
    Type* values, float* coordinates, const int ncoords,
    unsigned int* connections, const int ncells );

/*  節点の微分量 grad q を復元する（AGS 法。毎ステップ 1 回）。
 *  node_dq の添字は (slot)*ncoords + node。
 *  cells は 1 スレッドぶんの変数ごとの補間器（nvariables 個）。
 */
bool BuildNodeDqField(
    const VolumeObjectBase::CellType& celltype,
    const std::vector< CellBase<Type>* >& cells,
    const int nvariables,
    const NodeDqUsage& usage,
    const float* coordinates, const int ncoords,
    const unsigned int* connections, const int ncells,
    std::vector<float>& node_dq );

/*  節点局所座標表の自己検査。方式C を選ぶ前に 1 回通す。
 *  表の写し間違いやセル種の非対応をここで捕まえ、失敗したら現行方式へ退避する。
 */
bool VerifyNodeLocalCoords(
    const VolumeObjectBase::CellType& celltype,
    CellBase<Type>* cell,
    const float* coordinates,
    const unsigned int* connections );

/*  節点で不透明度を評価して節点F場を作る（毎ステップ 1 回）。
 *
 *  通常PBVR の不透明度は 2 段合成なので、節点でもその 2 段を通す。
 *      F_i = 変数式 m_opa_var[i] の評価
 *      A_i = tf[i].opacityMap().at( F_i )
 *      O   = 合成式 m_opa_func の評価( A_1..A_n )
 *  法線は -grad(O) なので、節点場は O で作る必要がある（F_i の場では伝達関数の
 *  微分が落ちる）。
 *
 *  node_F は補間器がこの配列を参照し続けるので、呼び出し側は補間器より長く
 *  生存させること。
 */
bool BuildNodeOpacityField(
    TransferFunctionSynthesizer* tfs,
    std::vector<TransferFunction>& tf,
    Type** values, const int nvariables,
    const float* coordinates, const int ncoords,
    const std::vector<float>* node_dq, const NodeDqUsage& usage,
    std::vector<Type>& node_F );

/*===========================================================================*/
/**
 *  @brief  方式C: 節点F場を 1 回補間して F と grad F を得る。
 *
 *  数式の評価も変数ごとの補間も要らない。粒子 1 個あたりの仕事は
 *  「補間 1 回＋勾配 1 回」で、変数の数にも数式の複雑さにも依らない。
 *  grad_ary() が J^-1 を適用するので、勾配は物理座標系で返る。
 */
/*===========================================================================*/
void CalculateScalarAndGradNodeField(
    const int nparticles_count,
    CellBase<Type>* interp_F,
    const Vector3f* local_coord_array,
    const UInt32* cell_index,
    float* scalar_result,
    float* grad_array_x, float* grad_array_y, float* grad_array_z );

} // end of namespace vismodule

#endif // PBVR__VISMODULE__NODE_FIELD_NORMAL_H_INCLUDE

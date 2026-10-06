/*****************************************************************************/
/**
 *  @file   NodeFieldNormal.cpp
 *  @brief  方式C（節点F場法）による法線計算。
 *
 *  移植元: v360 InSituLib/shared/EnsembleParticleGenerator.cpp
 *      collect_dq_usage / create_cell_for_celltype / verify_local_node_coords /
 *      build_node_dq_field / build_node_f_field / calculate_scalar_and_grad_nodefield
 *
 *  移植で変えたところ（理由つき）
 *
 *  1. 節点の局所座標表をこのファイル内に持った。
 *     移植元は CellBase::localNodeCoord() として各セルクラスに表を足しているが、
 *     v362 の CellBase にはこの仮想関数が無い。仮想関数を足すと vtable が変わり
 *     VisModule の 280 個の翻訳単位すべてに影響するので、表はここに閉じ込めた。
 *     写し間違いは VerifyLocalNodeCoords() が実行時に捕まえる。
 *
 *  2. 節点F場は「合成後の不透明度」で作る。
 *     移植元は可視化量が単一の数式だが、通常PBVR の不透明度は
 *     変数式 → 伝達関数テーブル → 合成式 の 2 段である（BuildNodeOpacityField 参照）。
 *
 *  3. スレッド別配列(TLS)による散布は移植しなかった。
 *     移植元で実測した結果、共有配列＋排他制御の方が速かった（1.08〜3.59 倍）ため
 *     既定で無効になっており、v360 側でも既定無効がコミットされている。
 */
/*****************************************************************************/
#include "NodeFieldNormal.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vismodule/Math>
#include <vismodule/HexahedralCell>
#include <vismodule/TetrahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>
#include "../../../FunctionParser/Token.h"

#if _OPENMP
#include <omp.h>
#endif

#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 128
#endif

namespace
{

/*===========================================================================*/
/**
 *  @brief  節点 i の局所座標。セル種ごとの定数表。
 *
 *  移植元では CellBase::localNodeCoord() として各セルクラスが持っている。
 *  表の値はそこから写した（各セルクラスの interpolationFunctions() から導いたもの）。
 *  写し間違いは VerifyLocalNodeCoords() が検出する。
 *
 *  四角錐だけ注意が要る。PyramidalCell は形状関数の内部で x = 2x/(1-z) と潰すため、
 *  格納座標での底面は ±1 ではなく ±0.5 になる。
 */
/*===========================================================================*/
bool local_node_coord(
    const vismodule::VolumeObjectBase::CellType& celltype,
    const int i, vismodule::Vector3f* coord )
{
    static const float hexa[8][3] =
    {
        { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 },
        { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 }
    };
    static const float tetra[4][3] =
    {
        { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }
    };
    static const float prism[6][3] =
    {
        { 0, 0, 1 }, { 1, 0, 1 }, { 0, 1, 1 },
        { 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }
    };
    static const float pyramid[5][3] =
    {
        {  0.0f,  0.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f }, {  0.5f, -0.5f, 0.0f },
        {  0.5f,  0.5f, 0.0f }, { -0.5f,  0.5f, 0.0f }
    };
    static const float qtetra[10][3] =
    {
        { 0, 0, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 },
        { 0.5f, 0, 0 }, { 0, 0, 0.5f }, { 0, 0.5f, 0 },
        { 0.5f, 0, 0.5f }, { 0, 0.5f, 0.5f }, { 0.5f, 0.5f, 0 }
    };
    static const float qhexa[20][3] =
    {
        { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 },
        { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 },
        { 0.5f, 0, 1 }, { 1, 0.5f, 1 }, { 0.5f, 1, 1 }, { 0, 0.5f, 1 },
        { 0.5f, 0, 0 }, { 1, 0.5f, 0 }, { 0.5f, 1, 0 }, { 0, 0.5f, 0 },
        { 0, 0, 0.5f }, { 1, 0, 0.5f }, { 1, 1, 0.5f }, { 0, 1, 0.5f }
    };

    if ( coord == NULL || i < 0 ) return false;

    const float ( *table )[3] = NULL;
    int n = 0;
    switch ( celltype )
    {
    case vismodule::VolumeObjectBase::Hexahedra:           table = hexa;    n = 8;  break;
    case vismodule::VolumeObjectBase::Tetrahedra:          table = tetra;   n = 4;  break;
    case vismodule::VolumeObjectBase::Prism:               table = prism;   n = 6;  break;
    case vismodule::VolumeObjectBase::Pyramid:             table = pyramid; n = 5;  break;
    case vismodule::VolumeObjectBase::QuadraticTetrahedra: table = qtetra;  n = 10; break;
    case vismodule::VolumeObjectBase::QuadraticHexahedra:  table = qhexa;   n = 20; break;
    default: return false;
    }
    if ( i >= n ) return false;
    *coord = vismodule::Vector3f( table[i][0], table[i][1], table[i][2] );
    return true;
}

/*===========================================================================*/
/**
 *  @brief  節点の局所座標表の自己検査。
 *
 *  節点 i の局所座標で局所→物理変換をすると、その節点の座標そのものが出るはずである
 *  （形状関数が N_i(xi_j) = delta_ij を満たすため）。表の写し間違いはここで捕まる。
 *  代表としてセル 0 で全節点を確認する。許容誤差はセルの代表長に対する相対で見る。
 */
/*===========================================================================*/
bool verify_local_node_coords(
    const vismodule::VolumeObjectBase::CellType& celltype,
    vismodule::CellBase<Type>* cell,
    const float* coordinates,
    const unsigned int* connections,
    const int nnodes )
{
    if ( cell == NULL || coordinates == NULL || connections == NULL ) return false;
    if ( nnodes <= 0 || nnodes > SIMD_BLK_SIZE ) return false;

    vismodule::UInt32 cid[SIMD_BLK_SIZE];
    vismodule::Vector3f lc[SIMD_BLK_SIZE];
    vismodule::Vector3f gc[SIMD_BLK_SIZE];
    for ( int i = 0; i < nnodes; ++i )
    {
        cid[i] = 0;
        if ( !local_node_coord( celltype, i, &lc[i] ) )
        {
            std::cerr << "[方式C] 節点局所座標の表がセル種に対応していない (node "
                      << i << ")" << std::endl;
            return false;
        }
    }
    cell->bindCellArray( nnodes, cid );
    cell->setLocalPointArray( nnodes, lc );
    cell->transformLocalToGlobalArray( nnodes, lc, gc );

    // セルの代表長（節点0からの最大距離）。許容誤差の基準にする。
    float extent = 0.0f;
    const unsigned int n0 = connections[0];
    for ( int i = 1; i < nnodes; ++i )
    {
        const unsigned int ni = connections[i];
        const float dx = coordinates[3 * ni]     - coordinates[3 * n0];
        const float dy = coordinates[3 * ni + 1] - coordinates[3 * n0 + 1];
        const float dz = coordinates[3 * ni + 2] - coordinates[3 * n0 + 2];
        const float d = std::sqrt( dx * dx + dy * dy + dz * dz );
        if ( d > extent ) extent = d;
    }
    if ( extent <= 0.0f ) extent = 1.0f;
    const float tol = 1.0e-4f * extent;

    bool ok = true;
    for ( int i = 0; i < nnodes; ++i )
    {
        const unsigned int ni = connections[i];
        const float ex = coordinates[3 * ni];
        const float ey = coordinates[3 * ni + 1];
        const float ez = coordinates[3 * ni + 2];
        const float dx = gc[i].x() - ex;
        const float dy = gc[i].y() - ey;
        const float dz = gc[i].z() - ez;
        const float err = std::sqrt( dx * dx + dy * dy + dz * dz );
        if ( !( err <= tol ) )
        {
            ok = false;
            std::cerr << "[方式C] 節点局所座標の自己検査 NG: node " << i
                      << " local(" << lc[i].x() << "," << lc[i].y() << "," << lc[i].z() << ")"
                      << " -> global(" << gc[i].x() << "," << gc[i].y() << "," << gc[i].z() << ")"
                      << " expected(" << ex << "," << ey << "," << ez << ")"
                      << " err=" << err << " tol=" << tol << std::endl;
        }
    }
    return ok;
}

} // end of anonymous namespace


namespace vismodule
{

/*===========================================================================*/
NormalMethod ResolveNormalMethod()
{
    const char* e = std::getenv( "PBVR_NORMAL_METHOD" );
    if ( e == NULL || e[0] == '\0' )              return NormalMethodCoordDiff;
    if ( std::strcmp( e, "coorddiff" ) == 0 )     return NormalMethodCoordDiff;
    if ( std::strcmp( e, "nodefield" ) == 0 )     return NormalMethodNodeField;
    std::cerr << "PBVR_NORMAL_METHOD: unknown value '" << e
              << "' (expected coorddiff|nodefield). Using coorddiff." << std::endl;
    return NormalMethodCoordDiff;
}

const char* NormalMethodName( const NormalMethod method )
{
    switch ( method )
    {
    case NormalMethodNodeField: return "節点F場法(nodefield)";
    default:                    return "座標差分法(coorddiff)";
    }
}

/*===========================================================================*/
/**
 *  @brief  数式のトークンを走査して、参照されている微分量を集める。
 *
 *  トークンは Q1=4, DQ1X=5, DQ1Y=6, DQ1Z=7, Q2=8, ... と並ぶので、
 *  Q1 からの差を 4 で割れば変数番号、余りが 1/2/3 なら x/y/z 成分になる。
 *  余り 0 は q 本体で、微分量ではない。
 *
 *  不透明度は伝達関数ごとに変数式を持つので、全部を走査して和をとる。
 */
/*===========================================================================*/
NodeDqUsage CollectNodeDqUsage(
    const std::vector< ::EquationToken >& exprs, const int nvariables )
{
    NodeDqUsage u;
    if ( nvariables <= 0 ) return u;
    u.slot.assign( static_cast<size_t>( nvariables ) * 3, -1 );

    std::vector<bool> used( static_cast<size_t>( nvariables ) * 3, false );
    for ( size_t k = 0; k < exprs.size(); ++k )
    {
        for ( int i = 0; i < 128 && exprs[k].exp_token[i] != END; ++i )
        {
            if ( exprs[k].exp_token[i] != VARIABLE ) continue;
            const int name = exprs[k].var_name[i];
            if ( name < Q1 || name > Q23 ) continue;
            const int d = name - Q1;
            const int c = d % 4;
            if ( c == 0 ) continue;                  // q 本体
            const int v = d / 4;
            if ( v < 0 || v >= nvariables ) continue;
            used[ static_cast<size_t>( v ) * 3 + ( c - 1 ) ] = true;
        }
    }

    for ( int v = 0; v < nvariables; ++v )
    {
        bool any_component = false;
        for ( int c = 0; c < 3; ++c )
        {
            if ( !used[ static_cast<size_t>( v ) * 3 + c ] ) continue;
            u.slot[ static_cast<size_t>( v ) * 3 + c ] = u.nslots++;
            any_component = true;
        }
        if ( any_component ) u.var_index.push_back( v );
    }
    return u;
}

/*===========================================================================*/
CellBase<Type>* CreateCellForCelltype(
    const VolumeObjectBase::CellType& celltype,
    Type* values, float* coordinates, const int ncoords,
    unsigned int* connections, const int ncells )
{
    switch ( celltype )
    {
    case VolumeObjectBase::Tetrahedra:
        return new TetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case VolumeObjectBase::Hexahedra:
        return new HexahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case VolumeObjectBase::QuadraticTetrahedra:
        return new QuadraticTetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case VolumeObjectBase::QuadraticHexahedra:
        return new QuadraticHexahedralCell<Type>( values, coordinates, ncoords, connections, ncells );
    case VolumeObjectBase::Prism:
        return new PrismaticCell<Type>( values, coordinates, ncoords, connections, ncells );
    case VolumeObjectBase::Pyramid:
        return new PyramidalCell<Type>( values, coordinates, ncoords, connections, ncells );
    default:
        return NULL;
    }
}

/*===========================================================================*/
/**
 *  @brief  節点の微分量 grad q を復元する（AGS 法。毎ステップ 1 回）。
 *
 *  各セルで節点位置の grad q を評価し、節点に散布して平均する。節点は複数のセルに
 *  共有され、補間関数がセルごとに独立なので、そこでの微分量はセルごとに違う値になる
 *  （本来「多価」）。この平均で一意な代表値を決める。
 *
 *  混ぜ方は単純平均。grad_ary() が J^-1 を適用した時点で勾配は物理座標系の量に
 *  なっており、セルの大小には依存しないので体積は勾配そのものには効かない。
 *
 *  復元するのは数式が実際に参照している (変数, 成分) だけである。参照の無い変数は
 *  評価そのものを飛ばし、参照の無い成分は散布と格納を飛ばす。
 *
 *  コストはセル数 x 節点数 x 使う変数の数の勾配評価。粒子数には依らない。
 *
 *  制約: MPI 領域境界の節点は自ランクのセルからの寄与しか集まらないため、
 *  そこだけ片側平均になる。境界の合算は未実装（法線＝陰影付け用途のため許容する）。
 */
/*===========================================================================*/
bool BuildNodeDqField(
    const VolumeObjectBase::CellType& celltype,
    const std::vector< CellBase<Type>* >& cells,
    const int nvariables,
    const NodeDqUsage& usage,
    const float* coordinates, const int ncoords,
    const unsigned int* connections, const int ncells,
    std::vector<float>& node_dq )
{
    if ( cells.empty() || nvariables <= 0 || !usage.any() ) return false;
    if ( coordinates == NULL || connections == NULL || ncoords <= 0 || ncells <= 0 ) return false;
    if ( cells[0] == NULL ) return false;

    const int nnodes = static_cast<int>( cells[0]->numberOfNodes() );
    if ( nnodes <= 0 || nnodes > SIMD_BLK_SIZE ) return false;

    const int nused = static_cast<int>( usage.var_index.size() );
    node_dq.assign( static_cast<size_t>( usage.nslots ) * ncoords, 0.0f );
    std::vector<float> weight( static_cast<size_t>( ncoords ), 0.0f );

    // 節点の局所座標。表は VerifyNodeLocalCoords() で検査済みのものを使う。
    std::vector<Vector3f> node_local( nnodes );
    for ( int k = 0; k < nnodes; ++k )
    {
        if ( !local_node_coord( celltype, k, &node_local[k] ) ) return false;
    }

#pragma omp parallel
    {
        std::vector<float> gx( static_cast<size_t>( nused ) * SIMD_BLK_SIZE );
        std::vector<float> gy( static_cast<size_t>( nused ) * SIMD_BLK_SIZE );
        std::vector<float> gz( static_cast<size_t>( nused ) * SIMD_BLK_SIZE );
        UInt32  cid[SIMD_BLK_SIZE];

        // 参照されている変数の補間器だけを使う。スレッドごとに実体が要るので
        // ここで作る（既存のセル生成 switch は変数ごとに作るので流用できない）。
        // 補間器は読み取りしかしないが setLocalPointArray で内部状態を書くため共有不可。
        std::vector< CellBase<Type>* > uc( nused, NULL );
        bool uc_ok = true;
        for ( int j = 0; j < nused; ++j )
        {
            uc[j] = cells[ usage.var_index[j] ];
            if ( uc[j] == NULL ) uc_ok = false;
        }

        if ( uc_ok )
        {
#pragma omp for schedule( static )
            for ( int base = 0; base < ncells; base += SIMD_BLK_SIZE )
            {
                const int m = ( ncells - base > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncells - base;
                for ( int p = 0; p < m; ++p ) cid[p] = static_cast<UInt32>( base + p );

                for ( int j = 0; j < nused; ++j ) uc[j]->bindCellArray( m, cid );

                // 節点ごとに評価する。ブロック内の全セルを同じ局所座標で評価するので、
                // 形状関数は 1 回の評価で足りる（setLocalPointUniformArray）。
                // 復元に要るのは勾配だけなので grad_ary を直接呼び、スカラは出さない。
                for ( int k = 0; k < nnodes; ++k )
                {
                    for ( int j = 0; j < nused; ++j )
                    {
                        uc[j]->setLocalPointUniformArray( m, node_local[k] );
                        uc[j]->grad_ary(
                            &gx[ static_cast<size_t>( j ) * SIMD_BLK_SIZE ],
                            &gy[ static_cast<size_t>( j ) * SIMD_BLK_SIZE ],
                            &gz[ static_cast<size_t>( j ) * SIMD_BLK_SIZE ], m );
                    }

                    for ( int p = 0; p < m; ++p )
                    {
                        const size_t node =
                            connections[ static_cast<size_t>( nnodes ) * ( base + p ) + k ];
#pragma omp atomic
                        weight[node] += 1.0f;
                        for ( int j = 0; j < nused; ++j )
                        {
                            const int v = usage.var_index[j];
                            const float* const a[3] = {
                                &gx[ static_cast<size_t>( j ) * SIMD_BLK_SIZE ],
                                &gy[ static_cast<size_t>( j ) * SIMD_BLK_SIZE ],
                                &gz[ static_cast<size_t>( j ) * SIMD_BLK_SIZE ] };
                            for ( int c = 0; c < 3; ++c )
                            {
                                const int s = usage.slot[ static_cast<size_t>( v ) * 3 + c ];
                                if ( s < 0 ) continue;
                                const float av = std::isfinite( a[c][p] ) ? a[c][p] : 0.0f;
#pragma omp atomic
                                node_dq[ static_cast<size_t>( s ) * ncoords + node ] += av;
                            }
                        }
                    }
                }
            }
        }
    }

    // 寄与したセルの数で割って平均にする。どのセルからも寄与が無かった節点は 0 のまま。
#pragma omp parallel for schedule( static )
    for ( int n = 0; n < ncoords; ++n )
    {
        const float w = weight[n];
        if ( w <= 0.0f ) continue;
        const float inv = 1.0f / w;
        for ( int s = 0; s < usage.nslots; ++s )
        {
            node_dq[ static_cast<size_t>( s ) * ncoords + n ] *= inv;
        }
    }
    return true;
}

/*===========================================================================*/
/**
 *  @brief  節点で不透明度を評価して節点F場を作る（毎ステップ 1 回）。
 *
 *  節点の q はデータそのものなので補間は要らない。X,Y,Z は節点座標を渡す。
 *  微分量(dq)は node_dq から取る。NULL を渡すと 0 として扱うので、
 *  dq を含まない数式では復元を省略できる（その方が安い）。
 *
 *  評価は粒子側の CalculateOpacity と同じ順序で行うので、同じ q に対して
 *  同じ不透明度が出る。節点は互いに独立なのでスレッドで分割できる。
 */
/*===========================================================================*/
bool BuildNodeOpacityField(
    TransferFunctionSynthesizer* tfs,
    std::vector<TransferFunction>& tf,
    Type** values, const int nvariables,
    const float* coordinates, const int ncoords,
    const std::vector<float>* node_dq, const NodeDqUsage& usage,
    std::vector<Type>& node_F )
{
    if ( tfs == NULL || values == NULL || coordinates == NULL ) return false;
    if ( nvariables <= 0 || ncoords <= 0 ) return false;

    ::EquationToken opa_func = tfs->opacityFunction();
    std::vector< ::EquationToken > opa_var = tfs->opacityVariable();
    if ( opa_var.empty() || opa_var.size() > tf.size() ) return false;
    if ( 4 * ( nvariables + 1 ) + 3 >= VAR_OFFSET_C ) return false;   // 添字の衝突を防ぐ

    node_F.assign( static_cast<size_t>( ncoords ), static_cast<Type>( 0 ) );
    const size_t nmap = opa_var.size();

#pragma omp parallel
    {
        // 評価器はスレッドごとに持つ（内部に状態を持つため共有できない）。
        FuncParser::ReversePolishNotation rpn;
        std::vector<float> var_value( NUMVAR, 0.0f );
        std::vector<float> scalars( nmap, 0.0f );

#pragma omp for schedule( static )
        for ( int n = 0; n < ncoords; ++n )
        {
            var_value[X] = coordinates[3 * n];
            var_value[Y] = coordinates[3 * n + 1];
            var_value[Z] = coordinates[3 * n + 2];

            // 節点の q はデータそのもの。dq は復元した節点場から取る。
            for ( int v = 0; v < nvariables; ++v )
            {
                var_value[ 4 * ( v + 1 ) ] = static_cast<float>( values[v][n] );
                for ( int c = 0; c < 3; ++c )
                {
                    float dq = 0.0f;
                    if ( node_dq != NULL )
                    {
                        const int s = usage.slot[ static_cast<size_t>( v ) * 3 + c ];
                        if ( s >= 0 ) dq = ( *node_dq )[ static_cast<size_t>( s ) * ncoords + n ];
                    }
                    var_value[ 4 * ( v + 1 ) + 1 + c ] = dq;
                }
            }

            // 1段目: 伝達関数ごとの変数式 → 不透明度テーブル
            for ( size_t i = 0; i < nmap; ++i )
            {
                rpn.setExpToken( &( opa_var[i].exp_token[0] ) );
                rpn.setVariableName( &( opa_var[i].var_name[0] ) );
                rpn.setNumber( &( opa_var[i].val_array[0] ) );
                rpn.setVariableValue( &var_value[0] );
                const float s = rpn.eval();
                scalars[i] = std::isfinite( s ) ? s : 0.0f;
                var_value[ VAR_OFFSET_A + i ] = tf[i].opacityMap().at( scalars[i] );
            }

            // 2段目: 合成式
            rpn.setExpToken( &( opa_func.exp_token[0] ) );
            rpn.setVariableName( &( opa_func.var_name[0] ) );
            rpn.setNumber( &( opa_func.val_array[0] ) );
            rpn.setVariableValue( &var_value[0] );
            const float o = rpn.eval();
            node_F[n] = static_cast<Type>(
                std::isfinite( o ) ? Math::Clamp<float>( o, 0.0f, 1.0f ) : 0.0f );
        }
    }
    return true;
}

/*===========================================================================*/
void CalculateScalarAndGradNodeField(
    const int nparticles_count,
    CellBase<Type>* interp_F,
    const Vector3f* local_coord_array,
    const UInt32* cell_index,
    float* scalar_result,
    float* grad_array_x, float* grad_array_y, float* grad_array_z )
{
    interp_F->bindCellArray( nparticles_count, cell_index );
    interp_F->setLocalPointArray( nparticles_count, local_coord_array );
    interp_F->scalar_ary( scalar_result, nparticles_count );
    interp_F->grad_ary( grad_array_x, grad_array_y, grad_array_z, nparticles_count );
}

/*  節点局所座標表の自己検査を外から呼べるようにする（呼び出し側が方式C を選ぶ前に通す）。 */
bool VerifyNodeLocalCoords(
    const VolumeObjectBase::CellType& celltype,
    CellBase<Type>* cell,
    const float* coordinates,
    const unsigned int* connections )
{
    if ( cell == NULL ) return false;
    return verify_local_node_coords(
        celltype, cell, coordinates, connections,
        static_cast<int>( cell->numberOfNodes() ) );
}

} // end of namespace vismodule

/*****************************************************************************/
/**
 *  @file   CubicBSplineInterpolator.h
 *  @brief  等間隔構造格子の三次Bスプライン補間
 *
 *  TrilinearInterpolator の差し替えとして使えるよう、メソッド名と引数を揃えてある。
 *  勾配の符号も TrilinearInterpolator に合わせて -grad q を返す（下記「符号の規約」）。
 *
 *  三線形補間との違い:
 *    ・1軸あたり 4 節点（三線形は 2）。3次元で 64 節点（三線形は 8）
 *    ・C2 連続。2階微分まで連続なので、微分量を含む数式の連鎖律に使える
 *    ・節点値をそのまま係数にはできない。制御点を逆算する前処理が要る（buildControlPoints）
 *
 *  精度（正弦波の場、真値基準、実測）:
 *    1波長あたり 15.9 セルで、値の誤差 3.82e-2 -> 2.43e-4、勾配 5.55e-2 -> 4.75e-4
 *  処理時間: 三線形比 2.62 倍（補間部分のみ）
 */
/*****************************************************************************/
#ifndef PBVR__CUBIC_B_SPLINE_INTERPOLATOR_H_INCLUDE
#define PBVR__CUBIC_B_SPLINE_INTERPOLATOR_H_INCLUDE

#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <vismodule/ClassName>
#include <vismodule/Vector3>
#include <vismodule/Assert>

#ifndef SIMDW
#define SIMDW 128
#endif

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  等間隔構造格子の三次Bスプライン補間器。
 *
 *  【符号の規約】
 *      gradient() は TrilinearInterpolator に合わせて **-grad q** を返す。
 *      呼び出し側で符号を反転して使うこと。差し替え時の取り違えを避けるため、
 *      あえて同じ（紛らわしい）規約に揃えてある。
 *
 *  【構築の引数】
 *      TrilinearInterpolator は節点値を受け取るが、こちらは **制御点** を受け取る。
 *      節点値から制御点を作るには buildControlPoints() を先に呼ぶこと。
 *
 *  【添字と重みを持たない理由】
 *      TrilinearInterpolator は m_index[8][SIMDW] などを attachPoint() で作り置きするが、
 *      三次では 8 が 64 になり 1スレッドあたり数百 KB になって L2 を超える。
 *      ここでは座標だけ控え、重みは scalar()/gradient() の中でその場で計算する。
 *      分離可能な形（各軸の重み 4 個）なので、1点あたりの保持は 36 個の float で済む。
 */
/*===========================================================================*/
class CubicBSplineInterpolator
{
    visModuleClassName_without_virtual( vismodule::CubicBSplineInterpolator );

protected:

    const float*         m_coeff;        ///< 制御点。所有しない
    vismodule::Vector3ui m_resolution;   ///< 節点数
    int                  m_line_size;
    int                  m_slice_size;
    float                m_cell_length;

    // attachPoint() が控える座標（格子単位）
    float m_px[SIMDW];
    float m_py[SIMDW];
    float m_pz[SIMDW];

public:

    CubicBSplineInterpolator( const float* coeff, const vismodule::Vector3ui resolution ):
        m_coeff( coeff ),
        m_resolution( resolution ),
        m_line_size( static_cast<int>( resolution.x() ) ),
        m_slice_size( static_cast<int>( resolution.x() * resolution.y() ) ),
        m_cell_length( 1.0f )
    {
    }

    void setCellLength( const float cell_length ) { m_cell_length = cell_length; }

    void attachPoint( const float* p_x, const float* p_y, const float* p_z )
    {
        #pragma ivdep
        for ( int I = 0; I < SIMDW; I++ )
        {
            m_px[I] = p_x[I];
            m_py[I] = p_y[I];
            m_pz[I] = p_z[I];
        }
    }

    void scalar( float* values ) const;
    void gradient( float* g_x, float* g_y, float* g_z ) const;

    /*  2階微分（ヘッセ行列）。対称なので独立な6成分を返す。
     *
     *  【符号の規約】gradient() と同じく **-H** を返す。呼び出し側は gradient() と
     *  まったく同じ扱い（符号を反転して使う）でよい。この class の微分は
     *  すべて符号が反転している、と覚えること。
     *
     *  【単位】1/cell_length^2 が掛かる。setCellLength() に物理的な格子間隔を
     *  渡してあれば物理座標系で返る。gradient() が 1/cell_length なのに対して
     *  ここは二乗なので、格子単位のまま通すと誤差が二乗で効く。 */
    void hessian( float* h_xx, float* h_yy, float* h_zz,
                  float* h_xy, float* h_xz, float* h_yz ) const;

    /*  節点値から制御点を逆算する。x, y, z 軸に順に1回ずつ再帰フィルタを通す。
     *  境界は鏡像（q[-1] = q[1]）。単精度で計算する（速度優先。§資料 参照）。
     *  値が更新されるたびに呼ぶこと。
     *  入力の型は呼び出し側に合わせる（DOUBLE_SCHEME で Type が double になりうる）。 */
    template < typename T >
    static void buildControlPoints( const T* value,
                                    const vismodule::Vector3ui resolution,
                                    std::vector<float>& coeff );

    /*  実装の自己検査。重みの規格化と前処理の正しさを確かめる。
     *  起動時に一度呼び、false なら三線形へ退避すること。 */
    static bool selfTest( bool verbose = false );

protected:

    /*  1次元の重み。w=値, d=1階微分（局所座標 t について）。
     *  三次Bスプライン基底 B(x) を t で評価したもの。
     *      Sum w = 1 、 Sum d = 0 が恒等的に成り立つ（selfTest で確認） */
    static inline void weights( const float t, float w[4], float d[4] )
    {
        const float t2 = t * t, t3 = t2 * t, u = 1.0f - t;
        w[0] = u * u * u * ( 1.0f / 6.0f );
        w[1] = ( 3.0f * t3 - 6.0f * t2 + 4.0f ) * ( 1.0f / 6.0f );
        w[2] = ( -3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f ) * ( 1.0f / 6.0f );
        w[3] = t3 * ( 1.0f / 6.0f );
        d[0] = -0.5f * u * u;
        d[1] = ( 9.0f * t2 - 12.0f * t ) * ( 1.0f / 6.0f );
        d[2] = ( -9.0f * t2 + 6.0f * t + 3.0f ) * ( 1.0f / 6.0f );
        d[3] = 0.5f * t2;
    }

    /*  上に 2階微分の重み e を足したもの。基底をもう1回 t で微分しただけ。
     *      Sum e = (1-t) + (3t-2) + (-3t+1) + t = 0 が恒等的に成り立つ */
    static inline void weights( const float t, float w[4], float d[4], float e[4] )
    {
        weights( t, w, d );
        e[0] = 1.0f - t;
        e[1] = 3.0f * t - 2.0f;
        e[2] = -3.0f * t + 1.0f;
        e[3] = t;
    }

    /*  点 I の格子添字と局所座標。三次は i-1 から i+2 を参照するので、
     *  参照が配列外に出ないようセル添字を [1, resolution-3] に収める。
     *  領域端の 1 セルぶんは外挿になるが、S1 の暫定処置とする（鏡像で埋めた制御点を使う）。
     *
     *  【節点値を厳密に再現できる範囲】
     *      添字 [1, resolution-2]。ここでは t=0 または t=1 となり、重みが
     *      (1/6, 4/6, 1/6) の内挿になるので前処理の解いた条件式そのものになる。
     *      両端の節点 0 と resolution-1 は上の丸め込みにより t=-1 / t=2 の
     *      外挿になるため、節点値には戻らない。selfTest はこの範囲で検査する。 */
    static inline void locate( const float p, const int res, int& i0, float& t )
    {
        int i = static_cast<int>( p );
        if ( i < 1 ) i = 1;
        if ( i > res - 3 ) i = res - 3;
        t = p - static_cast<float>( i );
        i0 = i - 1;                       // 参照の先頭
    }

    /*  1軸ぶんの再帰フィルタ。境界は鏡像。 */
    static void prefilterLine( float* c, const int n, const int stride );
};

/*===========================================================================*/
/**
 *  @brief  制御点を補間して値を求める。
 */
/*===========================================================================*/
inline void CubicBSplineInterpolator::scalar( float* values ) const
{
    const int rx = static_cast<int>( m_resolution.x() );
    const int ry = static_cast<int>( m_resolution.y() );
    const int rz = static_cast<int>( m_resolution.z() );

    for ( int I = 0; I < SIMDW; I++ )
    {
        int i0, j0, k0; float tx, ty, tz;
        locate( m_px[I], rx, i0, tx );
        locate( m_py[I], ry, j0, ty );
        locate( m_pz[I], rz, k0, tz );

        float wx[4], wy[4], wz[4], dd[4];
        weights( tx, wx, dd );
        weights( ty, wy, dd );
        weights( tz, wz, dd );

        float s = 0.0f;
        for ( int c = 0; c < 4; ++c )
        {
            const int bk = ( k0 + c ) * m_slice_size;
            float a = 0.0f;
            for ( int b = 0; b < 4; ++b )
            {
                const int bj = bk + ( j0 + b ) * m_line_size;
                float v = 0.0f;
                for ( int m = 0; m < 4; ++m ) v += wx[m] * m_coeff[ bj + i0 + m ];
                a += wy[b] * v;
            }
            s += wz[c] * a;
        }
        values[I] = s;
    }
}

/*===========================================================================*/
/**
 *  @brief  勾配を求める。**TrilinearInterpolator に合わせて -grad q を返す。**
 */
/*===========================================================================*/
inline void CubicBSplineInterpolator::gradient( float* g_x, float* g_y, float* g_z ) const
{
    const int rx = static_cast<int>( m_resolution.x() );
    const int ry = static_cast<int>( m_resolution.y() );
    const int rz = static_cast<int>( m_resolution.z() );
    const float inv_h = 1.0f / m_cell_length;

    for ( int I = 0; I < SIMDW; I++ )
    {
        int i0, j0, k0; float tx, ty, tz;
        locate( m_px[I], rx, i0, tx );
        locate( m_py[I], ry, j0, ty );
        locate( m_pz[I], rz, k0, tz );

        float wx[4], wy[4], wz[4], dx[4], dy[4], dz[4];
        weights( tx, wx, dx );
        weights( ty, wy, dy );
        weights( tz, wz, dz );

        float sx = 0.0f, sy = 0.0f, sz = 0.0f;
        for ( int c = 0; c < 4; ++c )
        {
            const int bk = ( k0 + c ) * m_slice_size;
            float a = 0.0f, ax = 0.0f, ay = 0.0f;
            for ( int b = 0; b < 4; ++b )
            {
                const int bj = bk + ( j0 + b ) * m_line_size;
                float v = 0.0f, vx = 0.0f;
                for ( int m = 0; m < 4; ++m )
                {
                    const float d = m_coeff[ bj + i0 + m ];
                    v  += wx[m] * d;
                    vx += dx[m] * d;
                }
                a  += wy[b] * v;
                ax += wy[b] * vx;
                ay += dy[b] * v;
            }
            sx += wz[c] * ax;
            sy += wz[c] * ay;
            sz += dz[c] * a;
        }
        // TrilinearInterpolator と同じく負の勾配を返す
        g_x[I] = -inv_h * sx;
        g_y[I] = -inv_h * sy;
        g_z[I] = -inv_h * sz;
    }
}

/*===========================================================================*/
/**
 *  @brief  2階微分（ヘッセ行列）の独立6成分を求める。**gradient() と同じく -H を返す。**
 *
 *  重みの組み合わせを変えるだけで、参照する制御点は scalar()/gradient() と同じ 64 点。
 *      対角成分   1軸だけ2階微分の重み e、他2軸は値の重み w
 *      交差成分   2軸を1階微分の重み d、残り1軸は w
 *
 *  1点あたりの内側ループで x 方向の3種類の和（w, d, e）を作り、それを y, z で
 *  組み合わせる。6成分をまとめて1パスで出す。
 */
/*===========================================================================*/
inline void CubicBSplineInterpolator::hessian(
    float* h_xx, float* h_yy, float* h_zz,
    float* h_xy, float* h_xz, float* h_yz ) const
{
    const int rx = static_cast<int>( m_resolution.x() );
    const int ry = static_cast<int>( m_resolution.y() );
    const int rz = static_cast<int>( m_resolution.z() );
    const float inv_h2 = 1.0f / ( m_cell_length * m_cell_length );

    for ( int I = 0; I < SIMDW; I++ )
    {
        int i0, j0, k0; float tx, ty, tz;
        locate( m_px[I], rx, i0, tx );
        locate( m_py[I], ry, j0, ty );
        locate( m_pz[I], rz, k0, tz );

        float wx[4], wy[4], wz[4], dx[4], dy[4], dz[4], ex[4], ey[4], ez[4];
        weights( tx, wx, dx, ex );
        weights( ty, wy, dy, ey );
        weights( tz, wz, dz, ez );

        float sxx = 0.0f, syy = 0.0f, szz = 0.0f, sxy = 0.0f, sxz = 0.0f, syz = 0.0f;
        for ( int c = 0; c < 4; ++c )
        {
            const int bk = ( k0 + c ) * m_slice_size;
            // a=値, axx=x2階, ayy=y2階, axy=xy, ax=x1階, ay=y1階（z との組み合わせ用）
            float a = 0.0f, axx = 0.0f, ayy = 0.0f, axy = 0.0f, ax = 0.0f, ay = 0.0f;
            for ( int b = 0; b < 4; ++b )
            {
                const int bj = bk + ( j0 + b ) * m_line_size;
                float v = 0.0f, vx = 0.0f, vxx = 0.0f;
                for ( int m = 0; m < 4; ++m )
                {
                    const float q = m_coeff[ bj + i0 + m ];
                    v   += wx[m] * q;
                    vx  += dx[m] * q;
                    vxx += ex[m] * q;
                }
                a   += wy[b] * v;
                axx += wy[b] * vxx;
                ayy += ey[b] * v;
                axy += dy[b] * vx;
                ax  += wy[b] * vx;
                ay  += dy[b] * v;
            }
            sxx += wz[c] * axx;
            syy += wz[c] * ayy;
            szz += ez[c] * a;
            sxy += wz[c] * axy;
            sxz += dz[c] * ax;
            syz += dz[c] * ay;
        }
        // gradient() と同じ規約で符号を反転して返す
        h_xx[I] = -inv_h2 * sxx;
        h_yy[I] = -inv_h2 * syy;
        h_zz[I] = -inv_h2 * szz;
        h_xy[I] = -inv_h2 * sxy;
        h_xz[I] = -inv_h2 * sxz;
        h_yz[I] = -inv_h2 * syz;
    }
}

/*===========================================================================*/
/**
 *  @brief  1軸ぶんの再帰フィルタ。境界は鏡像（q[-1] = q[1]）。
 *
 *  節点 i での補間値が q[i] になるよう制御点を逆算する。関係式は
 *      (1/6)c[i-1] + (4/6)c[i] + (1/6)c[i+1] = q[i]
 *  で、三重対角の連立方程式だが、極 z = sqrt(3)-2 の1次フィルタを
 *  順方向・逆方向に1回ずつ通すだけで解ける。
 */
/*===========================================================================*/
inline void CubicBSplineInterpolator::prefilterLine( float* c, const int n, const int stride )
{
    if ( n < 2 ) return;

    const float z = -0.2679491924311228f;              // sqrt(3) - 2
    const float lambda = ( 1.0f - z ) * ( 1.0f - 1.0f / z );   // = 6

    for ( int i = 0; i < n; ++i ) c[i*stride] *= lambda;

    // 因果側の初期値（鏡像対称を仮定した打ち切り和）
    int horizon = n;
    {
        const int hz = static_cast<int>( std::ceil( std::log( 1.0e-7 ) / std::log( 0.2679491924311228 ) ) );
        if ( hz < n ) horizon = hz;
    }
    float zn = z, sum = c[0*stride];
    for ( int i = 1; i < horizon; ++i ) { sum += zn * c[i*stride]; zn *= z; }
    c[0*stride] = sum;

    for ( int i = 1; i < n; ++i ) c[i*stride] += z * c[(i-1)*stride];

    // 反因果側の初期値
    c[(n-1)*stride] = ( z / ( z * z - 1.0f ) ) * ( z * c[(n-2)*stride] + c[(n-1)*stride] );
    for ( int i = n - 2; i >= 0; --i ) c[i*stride] = z * ( c[(i+1)*stride] - c[i*stride] );
}

/*===========================================================================*/
/**
 *  @brief  節点値から制御点を作る。x, y, z 軸に順に適用する。
 */
/*===========================================================================*/
template < typename T >
inline void CubicBSplineInterpolator::buildControlPoints(
    const T* value, const vismodule::Vector3ui resolution, std::vector<float>& coeff )
{
    const int nx = static_cast<int>( resolution.x() );
    const int ny = static_cast<int>( resolution.y() );
    const int nz = static_cast<int>( resolution.z() );
    const long long line  = nx;
    const long long slice = static_cast<long long>( nx ) * ny;

    coeff.assign( static_cast<size_t>( slice ) * nz, 0.0f );
    for ( size_t i = 0; i < coeff.size(); ++i ) coeff[i] = static_cast<float>( value[i] );

#pragma omp parallel for schedule( static )
    for ( int k = 0; k < nz; ++k )
        for ( int j = 0; j < ny; ++j )
            prefilterLine( &coeff[ static_cast<size_t>( k ) * slice + static_cast<size_t>( j ) * line ], nx, 1 );

#pragma omp parallel for schedule( static )
    for ( int k = 0; k < nz; ++k )
        for ( int i = 0; i < nx; ++i )
            prefilterLine( &coeff[ static_cast<size_t>( k ) * slice + i ], ny, static_cast<int>( line ) );

#pragma omp parallel for schedule( static )
    for ( int j = 0; j < ny; ++j )
        for ( int i = 0; i < nx; ++i )
            prefilterLine( &coeff[ static_cast<size_t>( j ) * line + i ], nz, static_cast<int>( slice ) );
}

/*===========================================================================*/
/**
 *  @brief  自己検査。前処理と重みの正しさを確かめる。
 *
 *   1. 重みの総和が 1、1階微分の重みの総和が 0、**2階微分の重みの総和が 0**
 *   2. 節点で評価すると節点値に戻る（添字 [1, N-2] の全節点）  ← 前処理の正しさ。決定的
 *   3. 定数場を補間すると同じ定数、勾配は 0
 *   4. 内部で三次までの多項式を厳密に再現する（値・勾配・**2階微分**とも）
 *      あわせて **2階微分の符号**を q = x² で確認する（-H を返す規約）
 *   5. 境界の影響が端から 1 セルにつき |z| 倍ずつ減衰する（鏡像の閉じ方の確認）
 *
 *  【誤差の測り方】
 *      場が符号を変える（真値が 0 を横切る）ので、各点ごとの相対誤差は使えない。
 *      分母が潰れて、丸め誤差しか無い点で相対誤差が跳ね上がる。
 *      ここでは **絶対誤差を場の振幅で割った量** で判定する。
 *
 *  【境界について】
 *      前処理は境界を鏡像 q[-1]=q[1] で閉じる。鏡像で延ばした場は元の場と
 *      一致しないので、端の数セルでは補間値が真値からずれる。これは実装の
 *      誤りではなく、袖領域を交換していないことの帰結（S4 で対処）。
 *      ずれは端から 1 セルにつき |z|=0.268 倍ずつ減る。検査 4 は端から
 *      8 セル以上離れた点で行い、検査 5 でこの減衰そのものを確かめる。
 */
/*===========================================================================*/
inline bool CubicBSplineInterpolator::selfTest( bool verbose )
{
    bool ok = true;
    const int N = 64;   // 2階微分の検査に端から 12 セルの余裕が要る（§17.9）
    const vismodule::Vector3ui res( N, N, N );
    const size_t sz = static_cast<size_t>( N ) * N * N;

    float px[SIMDW], py[SIMDW], pz[SIMDW], vv[SIMDW], gx[SIMDW], gy[SIMDW], gz[SIMDW];

    // --- 1: 重みの規格化 ---
    for ( int s = 0; s <= 10; ++s )
    {
        const float t = 0.1f * s;
        float w[4], d[4], e[4];
        weights( t, w, d, e );
        const float sw = w[0] + w[1] + w[2] + w[3];
        const float sd = d[0] + d[1] + d[2] + d[3];
        const float se = e[0] + e[1] + e[2] + e[3];
        if ( std::fabs( sw - 1.0f ) > 1.0e-5f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 重みの総和 t=%.1f sum=%.7f (1 のはず)\n", t, sw ); }
        if ( std::fabs( sd ) > 1.0e-5f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 微分の重みの総和 t=%.1f sum=%.7f (0 のはず)\n", t, sd ); }
        if ( std::fabs( se ) > 1.0e-5f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 2階微分の重みの総和 t=%.1f sum=%.7f (0 のはず)\n", t, se ); }
    }

    // 一次の場。検査 2 と 5 で使う
    const float A = 0.3f, B = -0.7f, C = 1.1f, D = 2.5f;
    const float amp = ( std::fabs(A) + std::fabs(B) + std::fabs(C) ) * ( N - 1 ) + std::fabs(D);
    std::vector<float> q( sz ), c;
    for ( int k = 0; k < N; ++k )
        for ( int j = 0; j < N; ++j )
            for ( int i = 0; i < N; ++i )
                q[ (size_t)k*N*N + (size_t)j*N + i ] = A*i + B*j + C*k + D;

    buildControlPoints( q.data(), res, c );
    CubicBSplineInterpolator ip( c.data(), res );
    ip.setCellLength( 1.0f );

    // --- 2: 添字 [1, N-2] の全節点で節点値に戻るか（前処理の正しさ。決定的）---
    {
        std::vector<int> pt;
        pt.reserve( 3 * (size_t)(N-2)*(N-2)*(N-2) );
        for ( int k = 1; k <= N-2; ++k )
            for ( int j = 1; j <= N-2; ++j )
                for ( int i = 1; i <= N-2; ++i )
                { pt.push_back(i); pt.push_back(j); pt.push_back(k); }
        const int npt = static_cast<int>( pt.size() / 3 );

        float worst = 0.0f; int wi = 0, wj = 0, wk = 0;
        for ( int base = 0; base < npt; base += SIMDW )
        {
            const int n = std::min( SIMDW, npt - base );
            for ( int m = 0; m < SIMDW; ++m )
            {
                const int s = ( m < n ) ? ( base + m ) : base;   // 余りは先頭で埋める
                px[m] = (float)pt[3*s]; py[m] = (float)pt[3*s+1]; pz[m] = (float)pt[3*s+2];
            }
            ip.attachPoint( px, py, pz );
            ip.scalar( vv );
            for ( int m = 0; m < n; ++m )
            {
                const int s = base + m;
                const int i = pt[3*s], j = pt[3*s+1], k = pt[3*s+2];
                const float e = std::fabs( vv[m] - q[ (size_t)k*N*N + (size_t)j*N + i ] ) / amp;
                if ( e > worst ) { worst = e; wi = i; wj = j; wk = k; }
            }
        }
        if ( worst > 1.0e-5f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 節点で節点値に戻らない 誤差/振幅 %.3e 節点(%d,%d,%d) (前処理の誤り)\n",
                         worst, wi, wj, wk ); }
        else if ( verbose )
            std::printf( "CubicBSpline selfTest OK: 節点値の再現 誤差/振幅 %.3e (内部 %d^3 節点すべて)\n", worst, N-2 );
    }

    // --- 5: 境界の影響が端から 1 セルにつき |z|=0.268 倍ずつ減衰するか ---
    {
        float err[6] = { 0, 0, 0, 0, 0, 0 };
        for ( int d = 1; d <= 5; ++d )
        {
            for ( int m = 0; m < SIMDW; ++m ) { px[m]=d+0.37f; py[m]=d+0.61f; pz[m]=d+0.13f; }
            ip.attachPoint( px, py, pz );
            ip.gradient( gx, gy, gz );
            err[d] = std::fabs( -gz[0] - C );          // gradient() は -grad を返す規約
        }
        for ( int d = 1; d <= 4; ++d )
        {
            const float r = ( err[d] > 0.0f ) ? ( err[d+1] / err[d] ) : 1.0f;
            if ( !( r > 0.20f && r < 0.34f ) ) { ok = false;
                std::printf( "CubicBSpline selfTest NG: 境界の影響の減衰が |z|=0.268 倍でない "
                             "端から %d セル %.3e -> %.3e 比 %.3f\n", d, err[d], err[d+1], r ); }
        }
        if ( verbose )
            std::printf( "CubicBSpline selfTest OK: 境界の影響 端から 1..5 セルで %.2e %.2e %.2e %.2e %.2e\n",
                         err[1], err[2], err[3], err[4], err[5] );
    }

    // --- 3: 定数場 → 同じ定数、勾配は 0 ---
    {
        std::vector<float> q2( sz, 3.75f ), c2;
        buildControlPoints( q2.data(), res, c2 );
        CubicBSplineInterpolator ip2( c2.data(), res );
        ip2.setCellLength( 1.0f );
        for ( int m = 0; m < SIMDW; ++m ) { px[m]=5.3f; py[m]=7.9f; pz[m]=11.2f; }
        ip2.attachPoint( px, py, pz );
        ip2.scalar( vv );
        ip2.gradient( gx, gy, gz );
        if ( std::fabs( vv[0] - 3.75f ) > 1.0e-4f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 定数場 %.7f (3.75 のはず)\n", vv[0] ); }
        if ( std::fabs( gx[0] ) > 1.0e-4f || std::fabs( gy[0] ) > 1.0e-4f || std::fabs( gz[0] ) > 1.0e-4f )
        { ok = false; std::printf( "CubicBSpline selfTest NG: 定数場の勾配 (%.3e %.3e %.3e) (0 のはず)\n",
            gx[0], gy[0], gz[0] ); }
    }

    // --- 4: 内部で三次までの多項式を厳密に再現するか（値と勾配）---
    {
        const double S = 1.0 / N;                       // 振幅を抑えるため正規化座標で作る
        #define BS_F(u,v,w)  ( 1.0 + 0.8*(u) - 0.5*(v) + 0.6*(w) + 1.2*(u)*(u) - 0.9*(v)*(v) \
                             + 0.7*(w)*(w) + 0.4*(u)*(v) - 0.3*(v)*(w) + 0.5*(u)*(w)          \
                             + 1.1*(u)*(u)*(u) - 0.6*(v)*(v)*(v) + 0.9*(w)*(w)*(w) + 0.35*(u)*(v)*(w) )
        #define BS_FX(u,v,w) ( ( 0.8 + 2.4*(u) + 0.4*(v) + 0.5*(w) + 3.3*(u)*(u) + 0.35*(v)*(w) ) * S )
        #define BS_FY(u,v,w) ( ( -0.5 - 1.8*(v) + 0.4*(u) - 0.3*(w) - 1.8*(v)*(v) + 0.35*(u)*(w) ) * S )
        #define BS_FZ(u,v,w) ( ( 0.6 + 1.4*(w) - 0.3*(v) + 0.5*(u) + 2.7*(w)*(w) + 0.35*(u)*(v) ) * S )
        // 2階微分。座標の正規化が二乗で効くので S*S が掛かる
        #define BS_FXX(u,v,w) ( (  2.4 + 6.6*(u) ) * S * S )
        #define BS_FYY(u,v,w) ( ( -1.8 - 3.6*(v) ) * S * S )
        #define BS_FZZ(u,v,w) ( (  1.4 + 5.4*(w) ) * S * S )
        #define BS_FXY(u,v,w) ( (  0.4 + 0.35*(w) ) * S * S )
        #define BS_FXZ(u,v,w) ( (  0.5 + 0.35*(v) ) * S * S )
        #define BS_FYZ(u,v,w) ( ( -0.3 + 0.35*(u) ) * S * S )
        const float amp3 = 8.0f;

        std::vector<float> q3( sz ), c3;
        for ( int k = 0; k < N; ++k )
            for ( int j = 0; j < N; ++j )
                for ( int i = 0; i < N; ++i )
                    q3[ (size_t)k*N*N + (size_t)j*N + i ] = (float)BS_F( i*S, j*S, k*S );

        buildControlPoints( q3.data(), res, c3 );
        CubicBSplineInterpolator ip3( c3.data(), res );
        ip3.setCellLength( 1.0f );

        // 端から 12 セル以上離れた内部だけを見る（境界の鏡像の影響を避ける）。
        // 2階微分は値・勾配より境界に敏感で、端から 4 セルでは 8 % ずれる（§17.9）
        const int MARGIN = 12;
        int n = 0;
        for ( int k = MARGIN; k <= N-1-MARGIN && n < SIMDW; ++k )
            for ( int j = MARGIN; j <= N-1-MARGIN && n < SIMDW; ++j )
                for ( int i = MARGIN; i <= N-1-MARGIN && n < SIMDW; ++i )
                { px[n]=i+0.37f; py[n]=j+0.61f; pz[n]=k+0.13f; ++n; }
        for ( int m = n; m < SIMDW; ++m ) { px[m]=px[0]; py[m]=py[0]; pz[m]=pz[0]; }
        float hxx[SIMDW], hyy[SIMDW], hzz[SIMDW], hxy[SIMDW], hxz[SIMDW], hyz[SIMDW];
        ip3.attachPoint( px, py, pz );
        ip3.scalar( vv );
        ip3.gradient( gx, gy, gz );
        ip3.hessian( hxx, hyy, hzz, hxy, hxz, hyz );

        // 2階微分の真値の大きさ。絶対誤差をこれで割って判定する
        // (成分ごとに 1e-3 〜 1e-4 と小さいので、絶対値のままでは尺度が合わない)
        const float amp_h = 6.6f * (float)( S * S );
        // 2階微分の許容値だけ緩い理由: この場では制御点が ~8、2階微分が ~6e-4 で、
        // 打ち消しが約 1.4 万倍ある。単精度の丸め (eps 6e-8) がそのまま
        // 相対誤差 ~1e-3 の床になる。端からの距離を 12 -> 20 セルに広げても
        // 下がらないことを実測で確認済み(§17.9)。重みや組み立ての誤りなら
        // 相対誤差は O(1) になるので、5e-3 でも十分に捕まえられる。
        const float tol_h = 5.0e-3f;

        float wv = 0.0f, wg = 0.0f, wh = 0.0f;
        for ( int m = 0; m < n; ++m )
        {
            const double u = px[m]*S, v = py[m]*S, w = pz[m]*S;
            wv = std::max( wv, (float)std::fabs( vv[m] - (float)BS_F( u, v, w ) ) / amp3 );
            wg = std::max( wg, (float)std::fabs( -gx[m] - (float)BS_FX( u, v, w ) ) );
            wg = std::max( wg, (float)std::fabs( -gy[m] - (float)BS_FY( u, v, w ) ) );
            wg = std::max( wg, (float)std::fabs( -gz[m] - (float)BS_FZ( u, v, w ) ) );
            // hessian() は -H を返す規約
            wh = std::max( wh, (float)std::fabs( -hxx[m] - (float)BS_FXX( u, v, w ) ) / amp_h );
            wh = std::max( wh, (float)std::fabs( -hyy[m] - (float)BS_FYY( u, v, w ) ) / amp_h );
            wh = std::max( wh, (float)std::fabs( -hzz[m] - (float)BS_FZZ( u, v, w ) ) / amp_h );
            wh = std::max( wh, (float)std::fabs( -hxy[m] - (float)BS_FXY( u, v, w ) ) / amp_h );
            wh = std::max( wh, (float)std::fabs( -hxz[m] - (float)BS_FXZ( u, v, w ) ) / amp_h );
            wh = std::max( wh, (float)std::fabs( -hyz[m] - (float)BS_FYZ( u, v, w ) ) / amp_h );
        }
        if ( wv > 1.0e-5f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 三次多項式の再現 誤差/振幅 %.3e\n", wv ); }
        if ( wg > 1.0e-4f ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 三次多項式の勾配 最大絶対誤差 %.3e\n", wg ); }
        if ( wh > tol_h ) { ok = false;
            std::printf( "CubicBSpline selfTest NG: 三次多項式の2階微分 誤差/大きさ %.3e (許容 %.1e)\n",
                         wh, tol_h ); }
        else if ( verbose )
            std::printf( "CubicBSpline selfTest OK: 三次多項式 値 %.3e 勾配 %.3e 2階微分 %.3e (内部 %d 点)\n",
                         wv, wg, wh, n );

        // 2階微分の符号。凸な場 q = x^2 では d2q/dx2 > 0 なので hessian() は負を返すはず
        {
            std::vector<float> q4( sz ), c4;
            for ( int k = 0; k < N; ++k )
                for ( int j = 0; j < N; ++j )
                    for ( int i = 0; i < N; ++i )
                        q4[ (size_t)k*N*N + (size_t)j*N + i ] = (float)( i * i ) * (float)( S * S );
            buildControlPoints( q4.data(), res, c4 );
            CubicBSplineInterpolator ip4( c4.data(), res );
            ip4.setCellLength( 1.0f );
            for ( int m = 0; m < SIMDW; ++m ) { px[m]=N*0.5f+0.4f; py[m]=N*0.5f+0.2f; pz[m]=N*0.5f+0.7f; }
            ip4.attachPoint( px, py, pz );
            ip4.hessian( hxx, hyy, hzz, hxy, hxz, hyz );
            const float want = -2.0f * (float)( S * S );        // -H なので負
            if ( std::fabs( hxx[0] - want ) > 1.0e-3f * std::fabs( want ) )
            { ok = false; std::printf( "CubicBSpline selfTest NG: 2階微分の符号か大きさ %.6e (%.6e のはず)\n",
                                       hxx[0], want ); }
            else if ( verbose )
                std::printf( "CubicBSpline selfTest OK: 2階微分の符号 q=x^2 で -H = %.6e\n", hxx[0] );
        }

        #undef BS_F
        #undef BS_FX
        #undef BS_FY
        #undef BS_FZ
        #undef BS_FXX
        #undef BS_FYY
        #undef BS_FZZ
        #undef BS_FXY
        #undef BS_FXZ
        #undef BS_FYZ
    }

    if ( verbose && ok ) std::printf( "CubicBSpline selfTest: 全項目 OK\n" );
    return ok;
}

} // end of namespace vismodule

#endif // PBVR__CUBIC_B_SPLINE_INTERPOLATOR_H_INCLUDE

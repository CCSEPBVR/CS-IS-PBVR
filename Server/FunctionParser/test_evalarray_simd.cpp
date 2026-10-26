// Unit test: evalArraySIMD (across-particle SIMD eval) vs scalar eval().
// Build (from Server/FunctionParser):
//   g++ -O2 -std=c++17 -fopenmp -I. test_evalarray_simd.cpp \
//       ExpressionTokenizer.cpp ExpressionConverter.cpp ReversePolishNotation.cpp -o test_evalarray_simd
// Run: ./test_evalarray_simd
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>

#include "Token.h"
#include "ExpressionTokenizer.h"
#include "ExpressionConverter.h"
#include "ReversePolishNotation.h"

using namespace FuncParser;

static bool buildRPN( const std::string& e, int* et, int* vn, float* va )
{
    ExpressionTokenizer tk;
    ExpressionConverter cv;
    tk.tokenizeString( e );
    cv.convertExpToken( tk.m_exp_token );
    int sz = (int)cv.token_array.size();
    if ( sz > 128 ) return false;
    for ( int i = 0; i < 128; i++ )
    {
        if ( i < sz ) { et[i]=cv.token_array[i]; vn[i]=cv.var_array[i]; va[i]=cv.value_array[i]; }
        else          { et[i]=0; vn[i]=0; va[i]=0.0f; }
    }
    return true;
}

bool TestEvalArraySIMD()
{
    struct C { const char* e; int nv; };
    const C cases[] = {
        {"q1",1}, {"q1+q2",2}, {"q1*q2",2}, {"sqrt(q1*q1+q2*q2)",2},
        {"sin(q1)+cos(q2)",2}, {"exp(q1)/log(q2)",2}, {"q1^2",1},
        {"abs(q1)",1}, {"min(q1,q2)",2}, {"q1*q2-q3",3}
    };
    const int nc = (int)( sizeof(cases)/sizeof(cases[0]) );
    const int N  = 128;   // particle block
    int fail = 0;

    for ( int c = 0; c < nc; c++ )
    {
        int   et[128], vn[128];
        float va[128];
        if ( !buildRPN( cases[c].e, et, vn, va ) ) { printf("[BUILD FAIL] %s\n", cases[c].e); ++fail; continue; }

        // per-variable particle arrays (varied per particle)
        std::vector< std::vector<float> > q( cases[c].nv, std::vector<float>(N) );
        for ( int v = 0; v < cases[c].nv; v++ )
            for ( int i = 0; i < N; i++ )
                q[v][i] = 0.3f + 0.1f*v + 0.01f*i;

        // SIMD across particles
        ReversePolishNotation rpn;
        rpn.setExpToken(et); rpn.setVariableName(vn); rpn.setNumber(va);
        float* varr[128];
        for ( int i = 0; i < 128; i++ ) varr[i] = 0;
        for ( int v = 0; v < cases[c].nv; v++ ) varr[Q1 + 4*v] = q[v].data();
        rpn.setVariableValueArray(varr);
        std::vector<float> out(N, 0.0f);
        rpn.evalArraySIMD( out.data(), N );

        // scalar reference, per particle
        ReversePolishNotation rpns;
        rpns.setExpToken(et); rpns.setVariableName(vn); rpns.setNumber(va);
        float vv[128];
        for ( int i = 0; i < 128; i++ ) vv[i] = 0.0f;
        rpns.setVariableValue(vv);

        float maxrel = 0.0f; int ndiff = 0;
        for ( int i = 0; i < N; i++ )
        {
            for ( int v = 0; v < cases[c].nv; v++ ) vv[Q1 + 4*v] = q[v][i];
            float ref = rpns.eval();
            float got = out[i];
            float den = std::fabs(ref) > 1e-6f ? std::fabs(ref) : 1.0f;
            float rel = std::fabs(got-ref)/den;
            if ( rel > maxrel ) maxrel = rel;
            if ( rel > 1e-5f ) ++ndiff;
        }
        const char* verdict = ( maxrel <= 1e-5f ) ? "OK" : "NG";
        if ( verdict[0] == 'N' ) ++fail;
        printf("[%s] %-22s N=%d max_rel=%.2e diff=%d\n", verdict, cases[c].e, N, maxrel, ndiff);
    }

    printf("\nTestEvalArraySIMD: %s (%d failures)\n", fail==0?"PASS":"FAIL", fail);
    return fail == 0;
}

int main() { return TestEvalArraySIMD() ? 0 : 1; }

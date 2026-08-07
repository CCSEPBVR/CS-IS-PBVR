// 変動係数(CoV)勾配の単体テスト実行用ハーネス。
// ライブラリの OBJS には含めない(Makefile は OBJS を明示列挙しているため影響しない)。
//
// ビルド例:
//   icpx -std=c++17 -w -I../../Utils -I../../App -I../shared \
//        -I../../VisModule -I../../FunctionParser -I../../../KVS/Install/include \
//        CoVNormalSelfTestMain.cpp ChainRuleNormal.cpp -o covtest && ./covtest

#include <cstdio>

#include <vismodule/ChainRuleNormal>

int main()
{
    const bool ok = vismodule::RunCoVNormalSelfTest();
    std::printf( "CoVNormalSelfTest: %s\n", ok ? "PASS" : "FAIL" );
    return ok ? 0 : 1;
}

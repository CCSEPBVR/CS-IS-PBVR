# pbvr_server の Windows MPI 設定

Visual Studio の `Server/pbvr_server.sln` で `Release | x64` を選択し、
`pbvr_server` をビルドします。依存する `VisModule` も同じモードでビルドされます。
この設定は `pbvr_server_vtk`、`VisModule_vtk`、他の構成には適用されません。

## MPI／逐次の切り替え

`Server/PBVR.Execution.props` の次の既定値を変更してください。
両プロジェクトがこのファイルを参照するため、切り替えはここだけで行います。
Visual Studio がプロジェクトの再読み込みを求めた場合は再読み込みしてください。

```xml
<PBVR_USE_MPI Condition="'$(PBVR_USE_MPI)' == ''">true</PBVR_USE_MPI>
```

- `true`（既定）：MPI版。`CPU_VER` は未定義、MS-MPIをリンクします。
- `false`：逐次版。`CPU_VER` を定義し、MS-MPIをリンクしません。

`CPU_VER=0` では逐次版になるので、プリプロセッサ定義を個別に変更しないでください。
`PBVR_USE_MPI` が環境変数やMSBuildの共通プロパティで指定されている場合は、
そちらが既定値より優先されます。プロジェクト単独の設定で上書きせず、両方に同じ値を渡してください。

## インストール先

MS-MPIランタイムとSDKが必要です。既定のSDK参照先は次のとおりです。

- ヘッダー：`C:\Program Files (x86)\Microsoft SDKs\MPI\Include`
- ライブラリ：`C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64\msmpi.lib`
- 起動コマンド：`C:\Program Files\Microsoft MPI\Bin\mpiexec.exe`

SDKを別の場所にインストールした場合は、共通設定の `PBVR_MSMPI_SDK_DIR` を変更してください。
MPI版では、SDKのヘッダーまたはライブラリが見つからないとビルド時にエラーを表示します。

## 出力先と起動

MPI版と逐次版は、出力と中間生成物を別のディレクトリに保存します。

- MPI版：`Server\x64\Release\MPI\pbvr_server.exe`
- 逐次版：`Server\x64\Release\CPU\pbvr_server.exe`
- 中間生成物：それぞれの出力先の `obj\プロジェクト名\`

`VisModule.lib` も同じモードの出力先に生成され、`pbvr_server` はそのファイルを直接リンクします。
`FunctionParser` と既存の外部ライブラリの出力・参照先は従来どおりです。

MPI版の出力ディレクトリで、次のように起動できます。

```powershell
mpiexec -n 5 .\pbvr_server.exe
```

`5` は任意のプロセス数に変更できます。rank 0がサーバー、残りがワーカーです。
`mpiexec` がPATHにない場合は次のように指定します。

```powershell
& 'C:\Program Files\Microsoft MPI\Bin\mpiexec.exe' -n 5 .\pbvr_server.exe
```

逐次版は出力ディレクトリで `.\pbvr_server.exe` を直接起動します。
実行時に相対パスで参照するデータや設定がある場合は、作業ディレクトリを合わせてください。

# コマンドライン

本章ではコンバータコマンドの使い方を示します.

## 実行方法

事前に各種動的ライブラリへのパスは通しているか, DLL を bin 以下などにコピーしてください.
インストール先の bin ディレクトリ以下のバイナリに対し, 以下実行します.

```bash
kvsml-converter <入力ファイル>
```

MPI を有効化してビルドしたならば, OpenMPI の場合以下実行してください.

```bash
mpirun -np <並列数> kvsml-converter-mpi <設定ファイル>
```

## 設定ファイル概要

設定ファイルの例を以下示します.

```xml
<kvsmlConverter version="0">
  <target>
    <input>
      <single>
        ../Example/Input/periodicPiece.vtu
      </single>
    </input>
  </target>
  <target>
    <input meshDeformation="0">
      <serial>
        <directoryPath>
          ../Example/Input/Hex
        </directoryPath>
        <prefix>hex</prefix>
        <extension>vtu</extension>
      </serial>
    </input>
    <output>
      <directoryPath>
        /tmp/hex
      </directoryPath>
      <prefix>
        output
      </prefix>
    </output>
  </target>
  <target>
    <input>
      <distributed stepId="0" lastStepId="1">
        <directoryPath>
          ../Example/Input/Pvtu/example_0
        </directoryPath>
        <wildcard>example*.vtu</wildcard>
      </distributed>
      <distributed stepId="1" lastStepId="1">
        <directoryPath>
          ../Example/Input/Pvtu/example_1
        </directoryPath>
        <wildcard>example*.vtu</wildcard>
      </distributed>
    </input>
    <output>
      <directoryPath>
        /tmp/pvtu
      </directoryPath>
      <prefix>
        output
      </prefix>
    </output>
  </target>
</kvsmlConverter>
```

kvsmlConterter 要素の version 属性は**必ず指定してください**.

kvsmlConterter/target 要素はファイルのまとまりです.

kvsmlConterter/target/input 要素は入力ファイルを設定します.
以下子要素を設定します.

- kvsmlConterter/target/input/single 要素は単一ファイルを設定します.
- kvsmlConterter/target/input/serial 要素は単一領域の時刻歴ファイルを設定します.
- kvsmlConterter/target/input/distributed 要素は複数領域の 1 ファイル, または時刻歴ファイルを設定します.
- kvsmlConterter/target/input/\*/directoryPath 要素は入力ファイルディレクトリです.
- kvsmlConterter/target/input/\*/wildcard 要素は入力ファイルのファイルパターンです. 後述の prefix, extension 要素より優先されます.
- kvsmlConterter/target/input/\*/prefix 要素は入力ファイルプレフィックスです. 未指定が可能です.
- kvsmlConterter/target/input/\*/extension 要素は入力ファイルのドットなし拡張子です. 未指定が可能です.

また, kvsmlConterter/target/input 要素の子要素に以下属性を設定可能です.

| `single`, `serial`と`distribution`属性 | 概要                                           |
| -------------------------------------- | ---------------------------------------------- |
| `stepId`                               | ステップ番号. 0 始まり. `distibuted`要素に必須 |
| `lastStepId`                           | 最終ステップ番号. `distibuted`要素に必須       |
| `gridType`                             | `Structured` か `unstructured`                 |
| `binary`                               | 1 のとき, EnsightGold バイナリ形式             |
| `q`                                    | Plot3D Q ファイルパス                          |
| `f`                                    | Plot3D F ファイルパス                          |

ファイルフォーマットごとに, 以下属性を設定する必要があります.

| ファイルフォーマット | 指定すべき属性       |
| -------------------- | -------------------- |
| \*.vtk               | `gridType`           |
| \*.vtm               | `gridType`           |
| \*.case              | `binary`             |
| \*.xyz               | `q`, `f`, `gridType` |

kvsmlConterter/target/output 要素は出力を設定します.
未指定も可能です.

- kvsmlConterter/target/output/directoryPath 要素は出力ディレクトリを設定します. 未指定の場合, 入力と同じディレクトリに出力されます.
- kvsmlConterter/target/output/prefix 要素は出力ファイルプレフィックスを設定します. 未指定の場合, 適当に設定されるため, 意図しない結果になる場合があります.

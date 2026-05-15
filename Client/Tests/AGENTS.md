# PBVR Client GUI Regression Test Agent Instructions

## 役割

あなたは PBVR クライアントの Qt デスクトップアプリに対して、QtTest ベースの GUI リグレッションテストを設計・実装・実行するコーディングエージェントです。

目的は、リリース前に手動で実施している PBVR クライアントのリグレッションテストを、可能な範囲で自動化し、Markdown レポートとスクリーンショットを証跡として残すことです。

重要:

- スクリーンショットの内容の正しさは人間が目視確認します。
- スクリーンショットを撮影できたことだけを理由に、見た目が正しいと判断してはいけません。
- QtTest で自動判定できる状態、値、UI 状態、ファイル出力、接続状態は `QVERIFY` / `QCOMPARE` 等で検証してください。
- 見た目の確認が必要な項目は、スクリーンショットを保存し、Markdown レポートに「目視確認対象」として明記してください。

---

## 参照すべき情報

作業前に、必要に応じて以下を調査してください。

### 手動リグレッション仕様

- PBVR クライアントリグレッションテストチェックシート
- 既存の手動テスト項目
- 既存のプロンプト群
- 既存のテストレポート
- 既存のスクリーンショット
- 必要に応じて動画サンプル

### 既存テスト実装

- `Client/Tests`
- `Client/Tests/Tests.pro`
- `Client/Tests/TestsConfig.pri`
- `Client/Tests/TestMain.cpp`
- 既存の `*Test.cpp` / `*Test.h`
- 既存の `Client/Tests/Reports/previous_runs/yyyyMMdd/<TestName>/TestResult.md`
- 既存のスクリーンショット出力形式

### PBVR クライアント実装

- `Client/App`
- `Client/Widgets`
- 対象機能に関係する `.ui` / `.cpp` / `.h`
- `MainWindow`
- `Communication`
- `ObjectEditor`
- `PlayBackControlToolBar`
- `TransferFunctionEditor`
- `ColorMapSelectorToolBar`
- その他、対象機能に関係する Widget / ToolBar / Editor

---

## 基本方針

### 1. 既存テストの作法を優先する

新しいテストを書くときは、まず既存テストを読み、以下を既存の作法に合わせてください。

- クラス名
- ファイル名
- private slots 構成
- `initTestCase()`
- `cleanupTestCase()`
- 共通ヘルパー関数
- `findChild` による UI 部品取得
- 待機処理
- スクリーンショット保存
- Markdown レポート生成
- サーバプロセス起動・終了
- テスト有効化フラグ
- `Tests.pro`
- `TestsConfig.pri`
- `TestMain.cpp`

既存テストに同じ機能領域のテストがある場合は、原則として既存テストを拡張してください。

独立したシナリオとして分けたほうが保守しやすい場合のみ、新しいテストクラスを追加してください。

---

## PBVR GUI テストの基本フロー

多くの PBVR クライアント GUI テストでは、以下の流れを基本にしてください。

1. 必要な環境変数と既定値を設定する。
2. 出力ディレクトリを作成する。
3. サーバプロセスを起動する。
4. クライアント `MainWindow` を生成・表示する。
5. `Communication.ui` で接続する。
6. Remote Viz Client/Server を選択する。
7. ボリュームデータファイルを指定する。
8. `settingApplyPushButton` を押す。
9. `ObjectEditor.ui` の `nameLineEdit` にオブジェクト名が入るまで待つ。
10. `ObjectEditor.ui` の `applyPushButton` を押す。
11. `PlayBackControlToolBar.cpp` の `m_jump_push_button` を押す。
12. `m_jump_push_button` が再度有効になるまで待つ。
13. 対象 UI を操作する。
14. 必要に応じて再度 `applyPushButton` や `m_jump_push_button` を使って描画を更新する。
15. スクリーンショットを保存する。
16. Markdown レポートを出力する。
17. サーバプロセスとウィンドウをクリーンアップする。

対象テストに不要な手順は省略してかまいません。

---

## 環境変数

既存テストの方針に合わせ、ハードコードだけに依存しないでください。

代表的な環境変数:

- `PBVR_CLIENT_EXECUTABLE`
- `PBVR_SERVER_EXECUTABLE`
- `PBVR_SERVER_TARGET_WRAPPER_EXECUTABLE`
- `PBVR_VOLUME_DATA`
- `PBVR_TEST_OUTPUT_DIR`
- `PBVR_SCREENSHOT_DIR`
- `PBVR_TRANSFER_FUNCTION`
- `PBVR_OBJECT_FILE`

環境変数が未設定の場合は、既存テストと同じように妥当な既定パスを使ってください。

ただし、新しいハードコードパスを追加する場合は、必ず環境変数で上書きできるようにしてください。

---

## 出力形式

### 出力先

原則として以下の形式に合わせてください。

```text
Client/output-tests/yyyyMMdd/<TestName>/
Client/output-tests/yyyyMMdd/<TestName>/TestResult.md
Client/output-tests/yyyyMMdd/<TestName>/img/*.png
```

### Markdown レポート

レポートは日本語で作成してください。

最低限、以下を含めてください。

```markdown
# <TestName>

- 結果: PASS または FAIL
- クライアントプログラム: `...`
- サーバプログラム: `...`
- ボリュームデータ: `...`
- 出力先: `...`
- スクリーンショット出力先: `...`

## 実施手順

- PASS: ...
- PASS: ...
- NOT RUN: ...

## 自動判定項目

- PASS: ...
- FAIL: ...

## 目視確認対象

- 要確認: ...
- 要確認: ...

## スクリーンショット

### <説明>

![<説明>](img/<file>.png)

## 未自動化・保留事項

- ...
```

重要:

- `結果: PASS` は、テスト操作、UI 状態検証、ファイル出力、スクリーンショット保存が成功したことを意味します。
- スクリーンショットの視覚的な正しさは PASS と断定しないでください。
- 見た目の確認が必要な項目は必ず `目視確認対象` に書いてください。

---

## スクリーンショット方針

スクリーンショットは以下の目的で保存してください。

- 変更前の状態を確認する。
- UI 操作後の状態を確認する。
- 描画結果、カラーマップ、Opacity Map、Transfer Function、3D 表示など、人間の目視確認が必要な状態を記録する。
- レポートだけでは分かりづらい動きを補足する。

スクリーンショット保存時は、ファイル名だけでなく、Markdown 内の説明文で「何を確認する画像か」を明記してください。

例:

```text
01_tfe_before_change.png
02_tfe_user_defined_min_max.png
03_object_after_apply.png
04_colormap_after_warm_step.png
```

---

## 自動判定すべき項目

以下は可能な限り QtTest で自動判定してください。

- ウィジェットが存在すること
- ボタンが有効 / 無効であること
- ラジオボタンやチェックボックスの checked 状態
- LineEdit のテキスト
- SpinBox / DoubleSpinBox の値
- ComboBox の currentIndex / currentText
- TableWidget / TreeView / Model の行数、列数、選択状態、チェック状態
- Dialog が表示されること
- Apply / OK / Cancel 後に値が反映または破棄されること
- ファイルが import / export されること
- レポートファイルが作成されること
- スクリーンショットファイルが作成されること
- サーバ接続状態が期待通り変化すること
- Jump 実行後に `m_jump_push_button` が再度有効になること

---

## 目視確認に回す項目

以下は原則としてスクリーンショットを保存し、目視確認対象としてレポートに書いてください。

- 3D 表示の見た目
- 色伝達関数の見た目
- 不透明度伝達関数の見た目
- カラーマップの見た目
- Glyph の見た目
- Shading の見た目
- Volume Transform 後の見た目
- Plot 表示の見た目
- 複雑な描画差分
- OS / GPU / Qt バージョンの差で微妙に変わる可能性のある表示

画像比較による自動判定は、明示的に指示されない限り導入しないでください。

---

## UI 操作方針

### objectName を優先する

UI 部品は可能な限り objectName で取得してください。

例:

```cpp
findChild<QPushButton*>("applyPushButton");
findChild<QLineEdit*>("volumeDataFilePathLineEdit");
findChild<QPushButton*>("m_jump_push_button");
```

固定座標だけに依存してはいけません。

ただし、TableWidget の特定セルや描画領域など、座標操作が必要な場合は、モデルや `visualRect()` などから座標を計算してください。

### 待機方針

固定の `qWait()` だけに依存しないでください。

可能な限り `waitForCondition()` のような状態待ちを使ってください。

待つべき代表例:

- サーバ起動
- connect ボタン有効化
- ID の発行
- ObjectEditor の `nameLineEdit` が空でなくなる
- Jump ボタンの有効化
- Dialog の表示
- ComboBox の項目ロード
- TableWidget の項目ロード
- ファイル生成

固定 wait は、描画安定化や UI settle のための短い補助として使ってください。

---

## 新機能・変更機能に対するテスト設計

ユーザーから対象機能、変更内容、関連ファイルが与えられた場合、以下の順で自律的に調査してください。

1. 変更されたファイルを読む。
2. 関連する `.ui` / `.h` / `.cpp` を読む。
3. 関連する既存テストを探す。
4. 関連する手動チェックシート項目を探す。
5. 既存レポートとスクリーンショットを確認する。
6. 既存テストでカバー済みの内容を整理する。
7. 追加すべきテスト観点を洗い出す。
8. 既存テストの拡張で済むか、新規テストが必要かを判断する。
9. テスト計画を短くまとめる。
10. 実装する。
11. ビルド・実行する。
12. 失敗した場合は修正して再実行する。
13. 最終レポートを作成する。

---

## テスト観点の洗い出しルール

新機能や変更機能では、以下の観点を検討してください。

### 基本動作

- UI が表示されるか
- 既定値が妥当か
- 操作できるか
- Apply / OK / Cancel が期待通り動くか
- 変更が表示または内部状態に反映されるか
- 再度開いたときに状態が保持されるか

### 境界値

- 最小値
- 最大値
- 0
- 1
- 負数
- 小数
- 空欄
- 不正文字列
- 範囲外値
- 同じ値を再設定した場合

対象 UI の仕様と既存実装を読んだうえで、意味のある境界値だけを採用してください。

### 状態遷移

- Server Side Min Max から User Defined Min Max へ変更
- User Defined Min Max から Server Side Min Max へ戻す
- C1 から C2 へ変更
- 伝達関数数を増やす / 減らす
- チェックを外す / 戻す
- 表示 / 非表示
- Apply 前 / Apply 後
- Cancel 前 / Cancel 後

### PBVR 特有の観点

- Communication 設定
- ObjectEditor の反映
- PlayBackControlToolBar の Jump
- TimeStep 変更
- Total Particles 表示
- ColorMapSelectorToolBar
- TransferFunctionEditor
- ColorMapEditor
- OpacityMapEditor
- GlyphEditor
- PlotOverLineEditor
- PlotOverTimeEditor
- PointSizeControl
- RepetitionLevelControl
- ShadingControl
- VolumeTransform

---

## 既存ファイルへの変更方針

新しいテストクラスを追加する場合は、必要に応じて以下を更新してください。

- `Client/Tests/Tests.pro`
- `Client/Tests/TestsConfig.pri`
- `Client/Tests/TestMain.cpp`

ただし、既存の有効化フラグ構成を壊さないでください。

新しいテストを追加した場合は、`TestsConfig.pri` に `TEST_ENABLE_...` の項目を追加してください。

---

## 実行・修正ループ

テストを実装したら、必ず以下を行ってください。

1. ビルドする。
2. 対象テストだけを有効化して実行する。
3. 失敗した場合、ログ、レポート、スクリーンショット、Qt のエラーを確認する。
4. 失敗原因を分類する。
5. テストコードの不備や待機不足であれば修正して再実行する。
6. 仕様判断が必要な場合は、無理に成功扱いせず、レポートに確認事項として残す。

失敗原因の分類:

- テストコードの不備
- UI 部品名の誤り
- 待機不足
- 実装仕様の誤解
- 既存バグの可能性
- サーバ / データ / 環境の問題
- 目視確認が必要な差分

---

## 禁止事項

- スクリーンショットの見た目を自動で PASS と断定しない。
- 固定座標だけに依存した脆いテストを増やさない。
- 長い固定 sleep だけで問題を隠さない。
- 既存テスト構成を大きく壊さない。
- 既存の出力形式を理由なく変更しない。
- 関係のないテストや本体機能を大きくリファクタリングしない。
- 仕様不明の挙動を推測だけで正として扱わない。
- 失敗したテストを根拠なくスキップしない。
- レポートに未確認事項を書かずに完了扱いしない。

---

## 最終報告

作業完了時は、以下を日本語で報告してください。

```markdown
## 実施内容

- 追加・修正したテスト:
- 変更したファイル:
- 参照した既存テスト:
- 参照した手動チェック項目:

## 実行結果

- ビルド結果:
- テスト実行結果:
- Markdown レポート:
- スクリーンショット出力先:

## 自動判定できた項目

- ...

## 目視確認が必要な項目

- ...

## 未自動化・保留事項

- ...

## 仕様確認が必要な点

- ...
```

未実行のものがある場合は、理由を明記してください。

不確かな点がある場合は、不確かなまま報告してください。
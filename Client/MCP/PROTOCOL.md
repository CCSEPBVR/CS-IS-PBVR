# PBVR file protocol version 1 / get_state

## 通信と寿命

`PBVR_CTRL_DIR`は双方必須、既存ディレクトリの絶対パスです。未指定時の代替先はありません。
Windowsローカルファイル、PBVRとMCPサーバー各1インスタンスを前提とします。
Pythonのみcommand.txtへ追記、PBVRのみstructured.logへ追記します。
UTF-8（BOMなし）JSON Linesで、LFまでが1レコードです。CRLFもJSONの末尾空白として許容します。
空行は不正JSONです。改行のない断片は保留します。

要求例（UUIDは毎回新しく作成）：

```json
{"version":1,"request_id":"c7909ee0-56e0-4de6-b07c-00f4777cabd3","command":"get_state"}
```

成功例（resultの完全仕様は後述）：

```json
{"version":1,"request_id":"c7909ee0-56e0-4de6-b07c-00f4777cabd3","session_id":"6b9b4189-f3eb-44b8-b783-fbc44c862443","ok":true,"result":{}}
```

失敗例：

```json
{"version":1,"request_id":"bad-command-1","session_id":"6b9b4189-f3eb-44b8-b783-fbc44c862443","ok":false,"error":{"code":"unsupported_command","message":"Only get_state is supported"}}
```

| キー | 型・意味 |
|---|---|
| version | 数値1（Pythonは整数1を送信、PBVRは数値として1との一致を確認） |
| request_id | 空でない文字列。PythonはUUID v4。照合にはこのIDだけを使う |
| command | 文字列。get_stateのみ。追加の要求フィールドは無視 |
| session_id | PBVRのMCPWorker生成ごとに一意なUUID文字列（通常PBVR起動1回に1つ） |
| ok | boolean |
| result / error | 成功時result object、失敗時error objectのいずれか |

PBVRがIDを取り出せない不正JSON/要求のエラーではrequest_idはnullです。
エラーコードはinvalid_json、invalid_request、unsupported_version、unsupported_command、
duplicate_requestです。同一PBVRセッション中のID再利用は処理せずエラーとします。
重複IDの集合はセッション終了まで保持します。再起動をまたぐ永続ID台帳は設けません。

Pythonのget_stateは上記の**応答エンベロープ全体**をdictとしてMCPへ返します。
PBVRからの失敗はok:false、Pythonの設定・I/O・不正応答・期限超過はMCPツールの例外として返ります。
タイムアウトは応答が期限までに観測できなかったことだけを意味します。
ロック取得後の送信から待機終了までを1要求として直列化し、待ち行列の待ち時間は期限に含みません。
キャンセル時も実行中のファイル処理の終了を待ってからロックを解放します。

Pythonの要求管理処理はキャンセル時にスレッド安全なEventを通知し、AnyIOのshield付き
CancelScope内でファイル処理タスクをasyncio.shieldで待ちます。繰り返しの直接キャンセルも
受け止め、実際のファイル処理が終了するまでロックを保持します。ファイル処理スレッドは
送信前・読み取りの区切り・応答待ちで通知を確認し、通常は期限を待たず終了します。
ただし、実行中のOSファイルI/Oを即時に中断する保証はありません。
送信直前の確認で通知を検知すれば要求は書き込みません。その確認とwrite/flushとの間に
通知が届く競合では送信され得ます。送信済み要求の削除・取り消し・再送はしません。
ClaudeのキャンセルはPBVRでの操作取り消しを意味せず、将来追加する色変更等も送信後は
キャンセル後にPBVRが実行し得ます。今回は取り消し要求も追加ツールも定義しません。
キャンセルした要求の遅延応答も、後続要求とはrequest_idで区別します。

PBVRは監視開始時のcommand.txt末尾を読み取り開始位置とします。
既存の最終行が未完なら、その続きも次のLFまで破棄し、開始前の要求を実行しません。
Pythonは要求前のstructured.log末尾から追尾し、IDの異なる応答を無視します。
自動再送はありません。PBVR再起動時に取り逃がした要求はタイムアウトします。
次に利用者が明示的に呼び出すget_stateは新しいIDになり、新セッションの応答を受け取れます。

## ファイル異常

- PBVR起動時にないファイルは作成。既存structured.logは維持。
- PBVRはcommand.txt読み取り時、およびstructured.log書き込み前にWindowsのファイルID、
  サイズ、消費済み全バイトのSHA-256を確認。観測した削除・置換・短縮・既読部分変更で
  MCP受付を停止し、ドックへエラーを保持。読み取り位置を巻き戻さず再実行しない。
- structured.logの別ライターによる追記もPBVR側でエラー。未完の末尾が起動時にある場合も停止。
- Pythonは送信前、command.txtに未完末尾があれば送信しない。
  追尾開始前からあるstructured.logの未完末尾は、続きのLFまで保留後に破棄する。
  待機中はログのファイルID・サイズ・既読部分を確認し、異常、不正UTF-8/JSONでその要求を失敗させる。
  次の要求は改めて現在のログ末尾から開始する。command.txt送信後の異常はPBVRが検出し、
  Python側は応答エラーまたはタイムアウトとなる。
- I/O/書き込み/flush失敗はドックでエラー。成功応答として表示しない。応答を保存できなければ
  Pythonはタイムアウトし得る。保存途中の行は両プロセス停止中に整理する。
- 要求行上限64 KiB（LF除く）。超過時はメモリ保護のため受付停止、応答なし。
  Pythonの応答行上限1 MiB。巨大なオブジェクト一覧で超えた場合はツールエラー。
- 監視で観測できる変更を対象とする。観測の間に完全に同一内容へ復元した変更の検出や、
  確認直後の敵対的な同時置換に対するトランザクション保証はない。
- 自動ローテーションなし。両プロセスを停止して整理する。詳細はREADME。

## GUIスナップショット

ファイルI/O、要求解析、コマンド判定、JSON応答化は専用QThreadです。
stateRequestedをGUIへキュー接続し、GUIで下記をまとめてQVariantMap/Listへコピーします。
GUIからcompleteをワーカーへキュー渡しします。GUI/シーンのポインターは渡しません。
GUIが重い処理中なら取得も待ちます。取得後の描画完了は待ちません。
GUI入力欄の未適用値は読みません。データ要求や設定変更を起こしません。

数値はJSON number、整数と記したものは整数値。存在しない値、非有限浮動小数点はnull。
ベクトルは[x,y,z]、行列は4×4の行配列です。行優先に列挙し、列ベクトルに左から掛けるKVSの行列です。
距離・位置はKVSのシーン/データ座標単位で、メートル等の物理単位へ換算しません。

| resultキー | 型・単位 | 取得元・意味 |
|---|---|---|
| captured_at | string、UTC ISO 8601ミリ秒付き | GUIコピー時の時刻（描画完了時刻ではない） |
| visualization_mode | string/null | MainWindow.m_viz_mode。local / local_client_server / remote_client_server / remote_in_situ。不明値null |
| viewport.width, height | integer、Qt論理pixel | 既存Screen.width()/height()。ドックを除く描画ウィジェット領域 |
| viewport.device_pixel_ratio | number | Screen.devicePixelRatioF()。物理pixelとの比率 |
| camera | object | scene.camera()。データ未読み込みでもカメラは存在 |
| object_manager_xform | matrix | scene.objectManager().xform()。全体の表示操作用変換。cameraとは別 |
| repetition_level | integer、無次元 | StochasticRenderingCompositor.repetitionLevel()の適用済み値。LODの一時描画回数ではない |
| current_time_step | integer/null、ステップ番号 | TimeStepControlToolBar.onDataRequestCompletedで記録した最後の全体完了値。Next入力/pendingではない。完了前・登録データなしではnull |
| objects | array of object | ObjectEditorのモデルに登録された全データ。データ登録なしは[]。内部Dummy、補助図形は含めない |

cameraのキー：

| キー | 型・単位 | 取得元 |
|---|---|---|
| position | vector、シーン座標 | Camera.position() = camera.xform().translation() |
| look_at | vector、シーン座標 | Camera.lookAt()（KVSのtransform center） |
| up | vector、無次元 | Camera.upVector()（xformのY軸の正規化） |
| xform | matrix | カメラ座標からシーン座標への変換 |
| view_matrix | matrix | Camera.viewingMatrix()、camera.xform().inverse() |
| projection_type | integer | Camera.projectionType()。0=Perspective、1=Orthogonal、2=Frustum |
| field_of_view_degrees | number、度 | Camera.fieldOfView() |
| near, far | number、シーン距離 | Camera.front()/back() |
| left, right, bottom, top | number、投影範囲のシーン座標 | Cameraの同名getter。投影種別により利用箇所が異なる |
| projection_matrix | matrix | Camera.projectionMatrix()の現在の内部値 |

objectsの各要素：

| キー | 型 | 取得元・意味 |
|---|---|---|
| uuid, name | string | ObjectInfo.uuid/name。空なら空文字列 |
| format | integer | ObjectInfo.format。Shared/ObjectInfoExtractor.hのFormat enum（0〜11） |
| format_name | string | ObjectInfoExtractor.formatToString(format)の既存表示名 |
| loaded | boolean | objectIDで参照できるシーンオブジェクトが存在する。登録前/未読み込みはfalse |
| visible | boolean | loadedかつ実際のObjectBase.isVisible()。レンダリング完了や画面内に映る保証ではない |
| display_enabled | boolean | ObjectInfo.isDisplay、適用済み表示設定（tmpIsDisplayではない）。時刻範囲外等でvisibleと異なり得る |
| object_id | integer/null | 読み込み済みシーンオブジェクトのobjectID.first。なければnull |
| current_time_step | integer/null | ObjectEditor.registerObject/replaceObjectで確定したcurrentImportedTimeStep。シーン未登録/値なしはnull |
| time_step_min, time_step_max | integer/null | ObjectInfo.timeStepのデータ範囲。-1はnull |
| xform | matrix/null | 登録済みObjectBase.xform()。未読み込みnull |
| min_object_coord, max_object_coord | vector/null | 登録済みObjectBaseのオブジェクト座標範囲。未読み込み/hasMinMaxObjectCoords()がfalseならnull |
| min_external_coord, max_external_coord | vector/null | 登録済みObjectBaseの外部座標範囲。未読み込み/hasMinMaxExternalCoords()がfalseならnull |

カメラ変換、ObjectManagerの変換、個別オブジェクトの変換は**別々**に返します。
KVSでは全体操作が個別xformにも反映されるため、object_manager_xformとobjects[].xformを
常に掛け合わせる仕様ではありません。描画時の正規化・センタリングも別です。
個別xformは取得時の実値であり、カメラ操作をオブジェクト変換として代用しません。
複数オブジェクトのステップはKeep Initial/Finalや途中の受信等で異なり得ます。
全体current_time_stepは同期完了の基準値、objects[].current_time_stepは実データの値です。

データ未読み込みでモデルも空ならobjects:[]、current_time_step:nullとなり成功です。
メタデータだけ登録されている場合は、その要素をloaded:false、visible:false、
object_id/current_time_step/xform/座標範囲:nullとして返します。
カメラ、描画領域、リピートレベルは空データでも返します。転送関数詳細は含みません。

最後の登録データの削除時、PBVRのGUI処理は保持中の全体ステップとCurrent Time Step表示を
クリアし、読み込み世代を無効化します。別データの登録だけではcurrent_time_stepはnullのままです。
有効な世代の読み込みが完了した時点で更新します。全削除前の読み込み結果は適用しません。
ローカル読み込みのGUIコールバックは要求通番も照合し、reset前後の同一番号を区別します。
ハイブリッドは両側の完了を回収してから次の読み込みへ進みます。既存サーバー応答はステップ
番号だけなので、無効化した要求も回収まで保持して直列化します。応答が来ない場合は次の
読み込みも待機し、接続resetで解除します。MCPの要求・応答形式や公開ツールは変わりません。

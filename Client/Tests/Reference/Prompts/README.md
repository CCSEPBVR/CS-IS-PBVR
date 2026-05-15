# Prompts

このディレクトリは、初期移行時の過去プロンプトを保存するためのアーカイブである。

通常のテスト作成では、このディレクトリを最初に参照しない。

通常は以下を優先する。

1. `Client/Tests/AGENTS.md`
2. `Client/Tests/REGRESSION_TEST_COVERAGE.md`
3. `Client/Tests/Reference/ManualChecklist/`
4. 既存テストコード
5. `Client/Tests/Reference/Reports/`

過去プロンプトは、既存テストの意図や、手動チェックシートに書かれていない操作手順を確認する必要がある場合のみ参照する。

今後の新しいプロンプトは原則として追加しない。
新しい知見は `AGENTS.md`、`REGRESSION_TEST_COVERAGE.md`、テストコードコメント、`TestResult.md`、または `Notes/` に反映する。
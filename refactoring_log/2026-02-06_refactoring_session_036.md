Date: 2026-02-06
Time: 23:46:30 JST

# Refactoring Session 036 (REF-009 continuation)

## 実施内容
- `ReadDefFileIdxPara` の `switch` ベース分岐を dispatcher へ置換。
- 追加要素:
  - `ReadIdxContext`
  - `ReadIdxHandler`
  - `ReadIdxDispatchEntry`
  - `DispatchReadIdxKeyword`
  - `IsGeneralSpinForbiddenKeyword`
- 既存の `ParseIdx*` helper を再利用する `HandleIdx*` ラッパーを追加し、`keyword -> handler` テーブルで統一。
- `ReadDefFileIdxPara` 本体から以下を削除:
  - 巨大 `switch(iKWidx)` 本体
  - 後段の `switch(iKWidx)` general-spin 制約判定
- 置換後は `dispatch` 実行後に `IsGeneralSpinForbiddenKeyword` で制約を一元判定。

## 挙動維持
- 既存 parser helper の処理内容、エラー時の戻り値、`cErrIncorrectFormatInter` の判定条件を維持。
- `KWInvTemp` など parser 未対応キーワードは no-op とし、従来 `default` 分岐相当の挙動を維持。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (10/10)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (70/70)

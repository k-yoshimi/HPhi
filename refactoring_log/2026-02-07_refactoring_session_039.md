Date: 2026-02-07
Time: 00:07:47 JST

# Refactoring Session 039 (Remaining task handling)

## 実施内容
- `src/readdef.c` の `ReadDefFileNInt` を追加分割し、可読性と error path の明確化を実施。
  - `CheckRequiredNameListFiles`
  - `ReadNIntKeywordFile`
  - `PostprocessReadDefNInt`
  - `GetReadNIntValidationContext`
- `ReadDefFileNInt` 本体は「初期化 -> 必須ファイル検証 -> 各def処理 -> 後処理」の流れへ整理。
- `ValidateReadNIntPositiveValues` のエラー表示対象を `ModPara` 名優先に統一（欠落時のみ `namelist` 名）。

## 追加テスト
- 新規: `test/unit/readdef_missing_ncond_error.sh`
  - `modpara` から `Ncond/Nup/Ndown/2Sz` を削除し、`NCond is not defined` の失敗を確認。
- `test/unit/CMakeLists.txt` に `unit_readdef_missing_ncond_error` を登録。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (15/15)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (75/75)

## 備考
- `unit -> smoke -> full` を逐次実行し、同一buildディレクトリ上の競合を回避。

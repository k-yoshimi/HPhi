Date: 2026-02-07
Time: 00:35:09 JST

# Refactoring Session 044 (diagonalcalc inter-process cleanup)

## 実施内容
- `src/diagonalcalc.c` の TEChemi/TETransfer inter process の重複ロジックを統合。
  - 新規: `ApplyTEOneBodyInterProcess`
- 置換対象:
  - `ApplyTEChemiInterProcess`
  - `ApplyTETransferInterProcess`
- 変更方針:
  - model別 occupation 判定（Hubbard/Kondo/tJ/Spin 系）と、
    `tmp_v0` 更新・`dam_pr` 加算・MPI集約を共通 helper へ集約。
  - 既存の2関数は wrapper 化して helper 呼び出しへ変更。

## 挙動維持
- inter process の one-body diagonal 更新式は不変。
- model分岐・general spin 判定ロジック・エラー処理（`cErrNoModel`）は維持。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (16/16)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (76/76)

## 補足
- `te_ac_hubbard_square` は初回実行で単発失敗するケースがあったが、
  単体・smoke・all の再実行では再現せず全PASSを確認した。

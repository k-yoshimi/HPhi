Date: 2026-02-06
Time: 22:53:32 JST

# Refactoring Session 026 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalInterAll` を段階分割。
- 追加 helper:
  - `ApplyInterAllBothInterProcess`
- 変更範囲:
  - `isite1 > X->Def.Nsite`（両サイト inter-process）分岐のみ helper 化。
  - `isite2 > X->Def.Nsite` 分岐と両サイト intra-process 分岐は保持。

## 挙動維持
- model別のビット抽出・`child_Spin*` 呼び出し・`BitCheckGeneral` 分岐をそのまま移設。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)

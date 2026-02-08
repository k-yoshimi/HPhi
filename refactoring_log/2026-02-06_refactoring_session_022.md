Date: 2026-02-06
Time: 22:40:16 JST

# Refactoring Session 022 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalCoulombInter` を段階分割。
- 追加 helper:
  - `ApplyCoulombInterBothInterProcess`
- 変更範囲:
  - 「`isite1 > X->Def.Nsite`（両サイト inter-process）」分岐のみ helper に移設。
  - 他分岐（片側 inter-process / 両側 intra-process）は今回未変更。

## 目的
- 大関数の先頭分岐を独立化し、次段分割の安全性を上げる。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)

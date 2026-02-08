Date: 2026-02-06
Time: 22:48:34 JST

# Refactoring Session 023 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalCoulombInter` を段階分割。
- 追加 helper:
  - `ApplyCoulombInterOneInterProcess`
  - `ApplyCoulombInterIntraProcess`
- `SetDiagonalCoulombInter` は以下3分岐の dispatcher へ整理。
  - 両サイト inter-process: `ApplyCoulombInterBothInterProcess`
  - 片側 inter-process: `ApplyCoulombInterOneInterProcess`
  - 両サイト intra-process: `ApplyCoulombInterIntraProcess`

## 挙動維持
- 各 model 別の式・ビット演算・`list_1` 利用有無を変更せずに移設。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)

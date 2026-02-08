Date: 2026-02-06
Time: 22:57:16 JST

# Refactoring Session 027 (REF-012 continuation)

## 実施内容
- `src/diagonalcalc.c` の `SetDiagonalInterAll` を段階分割。
- 追加 helper:
  - `ApplyInterAllOneInterProcess`
  - `ApplyInterAllIntraProcess`
- `SetDiagonalInterAll` は以下3分岐の dispatcher へ整理。
  - 両サイト inter-process: `ApplyInterAllBothInterProcess`
  - 片側 inter-process: `ApplyInterAllOneInterProcess`
  - 両サイト intra-process: `ApplyInterAllIntraProcess`

## 挙動維持
- model別の分岐、`child_Spin*` 呼び出し、`BitCheckGeneral` 条件を変更せず移設。
- `isite1/isite2` と `isigma1/isigma2` の swap 条件を維持。

## 追加整理
- `ApplyInterAllIntraProcess` の未使用ローカル変数を削除。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (9/9)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (69/69)

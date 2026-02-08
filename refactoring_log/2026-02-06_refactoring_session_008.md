Date: 2026-02-06
Time: 21:47:09 JST

# Refactoring Session 008 (REF-012 partial)

## 実施内容
- `src/diagonalcalc.c` の `diagonalcalc` 本体を処理単位で分割。
- 追加した helper:
  - `ResetDiagonalList`
  - `ApplyCoulombIntraTerms`
  - `ApplyChemiTerms`
  - `ApplyCoulombInterTerms`
  - `ApplyHundTerms`
  - `ApplyInterAllTerms`
- 既存ロジック（各相互作用項の計算式・ファイル出力内容）は変更せず、関数分割のみ実施。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)

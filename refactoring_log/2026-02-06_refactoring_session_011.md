Date: 2026-02-06
Time: 21:53:37 JST

# Refactoring Session 011 (REF-012 partial)

## 実施内容
- `src/sz.c` の `tJ` 系ケース分岐の重複を helper 化。
- 追加した helper:
  - `RecordOMPSzMid`
  - `ComputeSzCountForTJFamily`
- `sz()` の `switch (X->Def.iCalcModel)` で `tJ / tJNConserved / tJGC` を共通実装へ統合。
- 数値計算ロジック（`calculate_jb_*` と `omp_sz_tJ` の呼び出し順・回数）は維持。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)

Date: 2026-02-07
Time: 00:24:13 JST

# Refactoring Session 042 (sz tJ-family cleanup)

## 実施内容
- `src/sz.c` の tJ 系 `calculate_jb_*` 関数の重複ロジックを helper 化。
  - 新規: `GetTJHalfBitSiteCounts`
  - 新規: `CountTJHalfBitOccupations`
- 対象関数:
  - `calculate_jb_tJ`
  - `calculate_jb_tJNConserved`
  - `calculate_jb_tJGC`
- 変更方針:
  - doublon 判定・up/down カウント・`all_up/all_down` 算出を共通化。
  - 既存の組合せ計算式（`Binomial` 呼び出し）と更新順序は維持。

## 挙動維持
- tJ 系の `jb` 更新式は変更なし。
- helper 抽出のみで、入出力・分岐条件は維持。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (16/16)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (76/76)

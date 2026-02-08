Date: 2026-02-07
Time: 01:14:23 JST

# Refactoring Session 052 (follow-up on tasks 1 and 2)

## 実施内容
- `1` `GetDiagonalInterAll_simple` の役割整理:
  - public API は維持しつつ、実装を core 関数へ統合。
  - 新規 helper:
    - `SplitDiagonalAndOffDiagonalInterAll`
    - `InterAllTerm`
    - `LoadInterAllTermFromRow`
    - `AppendDiagonalInterAllTerm`
  - `GetDiagonalInterAll` は `apply_model_filter=TRUE`、`GetDiagonalInterAll_simple` は `FALSE` で同一 core を利用。
- `2` CI matrix 最適化（実行時間短縮 + 切り分け性向上）:
  - push/pr で `unit` と `smoke` を job 分離。
    - `ctest_unit`（ubuntu, single config）
    - `ctest_smoke`（matrix）
  - これにより `unit` の重複実行を削減し、失敗時の責務境界を明確化。

## 検証結果
- `ruby -e 'require "yaml"; YAML.load_file(".github/workflows/main.yml"); YAML.load_file(".github/actions/bootstrap-build/action.yml")'`: PASS
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (20/20)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (80/80)

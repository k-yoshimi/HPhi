Date: 2026-02-07
Time: 01:05:58 JST

# Refactoring Session 050 (CI isolation first, then readdef switch cleanup)

## 実施内容
- `2` を先行実施（CI の干渉回避）:
  - `.github/workflows/main.yml`
    - `ctest_unit_smoke`（push/pr）と `ctest_full_schedule`（schedule）へ job 分離。
    - workflow-level `concurrency` を追加し、同一 ref の CI 実行を排他化。
    - これにより `smoke` と `full` を同一 workflow/job で混在させない構成へ変更。
- `1` を続けて実施（`readdef.c` 大型 switch の段階整理）:
  - `src/readdef.c`
    - `GetDiagonalInterAll` の model 分岐を helper 化。
      - `BuildInterAllOffDiagonalTerm`
      - `IsInterAllFermionFamilyModel`
      - `IsInterAllSpinFamilyModel`
      - `IsInterAllStrictSzModel`
      - `CopyInterAllRow8`
      - `SetInterAllExchangeRow`
    - `ArrangeInterAllOffDiagonal` の model 分岐を helper 化。
      - `IsArrangeInterAllModel`
      - `NormalizeInterAllOffDiagonalRow`

## 検証結果
- `ruby -e 'require "yaml"; YAML.load_file(".github/workflows/main.yml")'`: PASS
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (20/20)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (80/80)

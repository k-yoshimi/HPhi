Date: 2026-02-07
Time: 00:59:50 JST

# Refactoring Session 049 (readdef readability + test-ops note)

## 実施内容
- `src/readdef.c`
  - `ApplyReadNIntModelRules` を分割して可読性を改善。
    - `IsCanonicalReadNIntModel`
    - `IsGrandCanonicalReadNIntModel`
    - `ApplyCanonicalRulesWithNCondAndSz`
    - `ApplyCanonicalRulesWithNCondNoSz`
    - `ApplyCanonicalReadNIntModelRules`
  - `CheckLocSpin` のモデル別分岐を helper 化して整理。
    - `IsItinerantOnlyCalcModel`
    - `IsKondoFamilyCalcModel`
    - `IsSpinFamilyCalcModel`
    - `ValidateLocSpinItinerantOnly`
    - `ValidateLocSpinKondoFamily`
    - `ValidateLocSpinSpinFamily`
- `doc/test_operations.md`
  - `te_ac_hubbard_square` の並列実行干渉に関する `Known Pitfall` を追記。
  - 再現時の isolation コマンドを明記。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (20/20)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (80/80)

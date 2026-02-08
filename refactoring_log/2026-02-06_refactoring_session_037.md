Date: 2026-02-06
Time: 23:53:54 JST

# Refactoring Session 037 (REF-009 continuation)

## 実施内容
- `readdef` の idx parser 群を `readdef.c` から分離。
  - 新規: `src/readdef_idx_parser.c`
  - 新規: `src/include/readdef_idx_parser.h`
- `readdef_idx_parser.c` に以下を移設。
  - `ParseIdx*` helper 一式（LocSpin/Trans/Coulomb*/Hund/PairHop/Exchange/Ising/PairLift/InterAll/OneBodyG/TwoBodyG/ThreeBodyG/FourBodyG/SixBodyG/Laser/TEOneBody/TETwoBody/Boost/SingleExcitation/PairExcitation）
  - `HandleIdx*` wrappers
  - `DispatchReadIdxKeyword`
  - `IsGeneralSpinForbiddenKeyword`
  - 公開API `ParseReadDefIdxKeyword`
- `src/readdef.c` は `ReadDefFileIdxPara` で parser 詳細を持たず、`ParseReadDefIdxKeyword` 呼び出しへ整理。
- `src/CMakeLists.txt` に `readdef_idx_parser.c` を追加。

## 追加テスト
- 新規: `test/unit/readdef_pair_excitation_count_mismatch.sh`
  - `pair.def` の件数不整合を意図的に作り、`ReadDefFileError` を確認。
- 新規: `test/unit/readdef_teonebody_count_mismatch.sh`
  - `teone.def` の timestep 件数不整合を意図的に作り、`ReadDefFileError` を確認。
- `test/unit/CMakeLists.txt` に2テストを追加。

## 挙動維持
- `ReadDefFileIdxPara` の分岐条件・エラー経路・general-spin 制約は維持。
- parser 移設後も既存 characterization と E2E が同一結果になることを確認。

## 検証結果
- `ctest --output-on-failure -L unit -j4`: PASS (12/12)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (72/72)

Date: 2026-02-07
Time: 00:14:44 JST

# Refactoring Session 040 (NInt parser split)

## 実施内容
- `ReadDefFileNInt` の parser API 境界を分離。
  - 新規: `src/readdef_nint_parser.c`
  - 新規: `src/include/readdef_nint_parser.h`
- `readdef.c` から以下を切り出し:
  - `ReadNIntContext`
  - `HandleNInt*` handler 群
  - `DispatchReadNIntKeyword`
  - `ReadSimpleKeywordCount`
- `readdef.c` は `ParseReadDefNIntKeyword` を使う呼び出し側に整理。
- `src/CMakeLists.txt` に `readdef_nint_parser.c` を追加してビルド連携。

## 挙動維持
- 既存の `ReadDefFileNInt` keyword 処理フローは維持。
- 既存 unit/smoke/full ctest で回帰がないことを確認。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit -j4`: PASS (15/15)
- `ctest --output-on-failure -L smoke -j4`: PASS (6/6)
- `ctest --output-on-failure -j4`: PASS (75/75)

Date: 2026-02-06
Time: 21:50:33 JST

# Refactoring Session 010 (REF-012 partial)

## 実施内容
- `src/sz.c` の終了処理を helper 化。
- 追加した helper:
  - `ValidateSzDimensionOrAbort`
  - `ConvertNConservedModelToNormalIfNeeded`
- `sz()` 本体から以下を置換:
  - 次元不一致時のエラー出力と終了処理
  - `NConserved -> Normal` へのモデル変換処理
- 目的はロジック変更なしの見通し改善。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)

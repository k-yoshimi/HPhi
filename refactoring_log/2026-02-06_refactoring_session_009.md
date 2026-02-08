Date: 2026-02-06
Time: 21:49:32 JST

# Refactoring Session 009 (REF-012 partial)

## 実施内容
- `src/sz.c` の前処理を helper 関数へ分割（挙動不変）。
- 追加した helper:
  - `InitializeGeneralSpinWorkArrays`
  - `AllocateAndClearListJb`
  - `SetupHilbertDimensionContext`
  - `SetupSplitBitContext`
- `sz()` 本体では以下を helper 呼び出しに置換:
  - General spin 向け作業配列の確保・0初期化
  - `list_jb` の確保・0初期化
  - `idim/N/N2` のモデル依存セットアップ
  - split bit (`irght/ilft/ihfbit`) セットアップ

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)

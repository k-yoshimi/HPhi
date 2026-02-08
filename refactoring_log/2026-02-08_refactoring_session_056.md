Date: 2026-02-08
Time: 11:07:50 JST

# Refactoring Session 056 (response plan for test-first + behavior-equivalence comments)

## 受領コメントサマリー
1. テスト先行の必要性
- 結合テスト中心だと未テスト機能の退行を見逃しやすい。
- リファクタ前に unit/characterization を先に整備し、ケースを網羅してから変更すべき。

2. リファクタ前後の挙動同値性検証
- リファクタは入出力不変であるべき。
- 変更箇所は旧版/新版を用意し、ランダム入力も含めた差分比較で同値性を確認すべき。

## 受領コメントへの対応方針
- 方針1（テスト先行）:
  - 以後のリファクタは「テスト追加コミット -> リファクタ本体コミット」の2段を必須化する。
  - `model x mode x conservation x input-format` の観点でテストギャップを先に埋める。
- 方針2（入出力不変性の厳密検証）:
  - リファクタ対象について旧版/新版バイナリを同時に作成し、同一入力の出力差分比較を自動化する。
  - 固定ケースに加えて制約付きランダム入力（fuzz）を投入し、差分があれば入力を保存して再現可能化する。

## 実装タスク分解
1. REF-GUARD-001: test-first運用ルールの明文化
- 成果物:
  - `doc/test_operations.md` に「テスト先行ルール」「2コミット運用」「PRチェック項目」を追記。
  - PRレビュー時の確認観点（正常/異常/境界）を明記。

2. REF-GUARD-002: テストギャップマトリクス作成
- 成果物:
  - `refactoring_log/` に対象マトリクス（`readdef/sz/diagonalcalc`）を追加。
  - 現在カバー済み/未カバーのセルを明示し、追加順序を定義。

3. REF-GUARD-003: characterization/unit testの先行拡張
- 成果物:
  - 未カバーセルに対する unit/characterization テスト追加。
  - 追加後に `ctest -L unit` / `ctest -L smoke` を通し、基準化。

4. REF-EQ-001: 旧版/新版比較ハーネスの作成
- 成果物:
  - 旧版（baseline）と新版（candidate）を分けてビルドするスクリプト（`test/tools/` 想定）。
  - 同一入力に対して主要出力ファイルと終了コードを比較する仕組みを追加。

5. REF-EQ-002: 差分比較ルールの実装
- 成果物:
  - 比較対象ファイル（例: `CHECK_Sdim.dat`, `CHECK_Memory.dat`, 物理量出力）を定義。
  - 浮動小数比較の許容誤差ルールを実装。
  - 差分検出時のレポート形式を統一。

6. REF-EQ-003: 制約付きランダム入力テスト
- 成果物:
  - `InterAll/readdef` 向けの制約付き入力生成器を追加（不正フォーマット除外、モデル制約順守）。
  - seed固定/保存機能を付け、失敗入力を再実行可能にする。

7. REF-EQ-004: CIへの段階導入
- 成果物:
  - PR必須は現行の `unit + smoke` を維持。
  - 差分テストは `workflow_dispatch` または schedule で実行するジョブとして追加。
  - 失敗時に入力seed/比較レポートをartifact化。

## 推奨実施順
1. `REF-GUARD-001`
2. `REF-GUARD-002`
3. `REF-GUARD-003`
4. `REF-EQ-001`
5. `REF-EQ-002`
6. `REF-EQ-003`
7. `REF-EQ-004`

## 完了条件
- すべてのリファクタPRで test-first の証跡（先行テスト追加）が確認できる。
- 主要対象（`readdef/sz/diagonalcalc`）で旧版/新版比較が自動実行される。
- 差分発生時に再現用入力（seed/fixture）を用いて追跡可能な状態になる。

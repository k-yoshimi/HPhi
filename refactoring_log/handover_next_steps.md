Date: 2026-02-07
Time: 01:16:54 JST

# Refactoring Handover Notes

## Current Status
- Main refactoring and unit-test expansion are complete.
- Local verification is green:
  - `ctest --output-on-failure -L unit -j4` (20/20)
  - `ctest --output-on-failure -L smoke -j4` (6/6)
  - `ctest --output-on-failure -j4` (80/80)
- CI workflow is split and shared setup/build is extracted to:
  - `.github/actions/bootstrap-build/action.yml`
  - `.github/workflows/main.yml`

## Related Document
- Test operation and guardrails:
  - `doc/test_operations.md`

## Remaining Work (Recommended Order)
1. Decide final policy for `GetDiagonalInterAll_simple`
- Keep as a lightweight path, or fully unify callers with `GetDiagonalInterAll`.
- If unifying, remove redundant API carefully and update declarations/usages.

2. Optimize schedule matrix cost
- Re-check coverage value of each `mpisize` entry in `ctest_full_schedule`.
- Reduce redundant combinations while preserving regression detection power.

3. Validate new CI design on GitHub Actions
- Confirm real CI run behavior (not only local YAML parse).
- Check runtime, flaky behavior, and failure diagnostics quality.

## Risks / Notes
- `te_ac_hubbard_square` has known intermittent interference if `smoke` and `full` are run concurrently in same build tree.
- Keep execution order for local validation:
  1. `unit`
  2. `smoke`
  3. `full`

## Useful Commands
```sh
cmake --build build -j4
ctest --output-on-failure -L unit -j4 --test-dir build
ctest --output-on-failure -L smoke -j4 --test-dir build
ctest --output-on-failure -j4 --test-dir build
ctest --output-on-failure -R '^te_ac_hubbard_square$' --test-dir build
```

## Update (2026-02-07 01:32:14 JST)
- CI failure follow-up was added for:
  - `CI / ctest_smoke (ubuntu-24.04, 1, 3)`
  - `CI / ctest_smoke (ubuntu-24.04, 1, 1)`
  - `CI / ctest_unit`
- Implemented hardening:
  - `.github/actions/bootstrap-build/action.yml`: apt install with retry (up to 3 attempts).
  - `.github/workflows/main.yml`: `unit/smoke` use `--parallel 1` and one retry on failure.
- Local verification after changes:
  - `ctest --output-on-failure -L unit --parallel 1`: PASS (20/20)
  - `ctest --output-on-failure -L smoke --parallel 1`: PASS (6/6)
  - smoke repeated 5 times: PASS
  - `OMP_NUM_THREADS=3` smoke: PASS
- Next restart point:
  1. Commit only CI files if needed first (`.github/workflows/main.yml`, `.github/actions/bootstrap-build/action.yml`).
  2. Re-run GitHub Actions and confirm whether failures are resolved.

## Update (2026-02-08 11:07:50 JST)
- Review comment対応として、以下の実施方針を追加:
  - test-first運用の必須化（テスト追加先行 -> リファクタ本体）。
  - 旧版/新版の入出力差分比較を自動化し、制約付きランダム入力で挙動不変を検証。
- 分解タスク（詳細は `refactoring_log/2026-02-08_refactoring_session_056.md`）:
  1. `REF-GUARD-001`: test-firstルールを `doc/test_operations.md` に明文化
  2. `REF-GUARD-002`: `readdef/sz/diagonalcalc` のテストギャップマトリクス作成
  3. `REF-GUARD-003`: 未カバーセルの unit/characterization 追加
  4. `REF-EQ-001`: 旧版/新版比較ハーネス作成
  5. `REF-EQ-002`: 比較対象/誤差許容ルール実装
  6. `REF-EQ-003`: 制約付きランダム入力 + seed保存
  7. `REF-EQ-004`: 差分テストを workflow_dispatch/schedule でCI導入

## Update (2026-02-08 11:17:10 JST)
- `readdef/InterAll` 向けに test-first の先行強化を実施:
  - 追加unit: `unit_readdef_interall_equivalence`
  - 追加ツール: `test/tools/compare_readdef_interall_versions.py`
  - 文書更新: `doc/test_operations.md` に test-first と差分比較手順を追記
- 実行結果:
  - `ctest -L unit --parallel 1`: PASS (21/21)
  - 差分比較ツール（baseline=candidate=build/src/HPhi, 3 trials）: PASS
- 次アクション候補:
  1. 差分比較ツールを `workflow_dispatch` で実行できるCIジョブ化（`REF-EQ-004`）
  2. `readdef/sz/diagonalcalc` のギャップマトリクス作成（`REF-GUARD-002`）

## Update (2026-02-08 12:50:44 JST)
- `sz` 側にも同様の test-first チェックを追加:
  - 追加unit: `unit_sz_hubbard_calchs_equivalence`（`CalcHS=0/1` 同値性）
  - 追加ツール: `test/tools/compare_sz_versions.py`（ランダム標準入力で baseline/candidate 比較）
- 併せて `compare_readdef_interall_versions.py` の失敗時 artifact 保持を修正。
- 実行結果:
  - `ctest -L unit --parallel 1`: PASS (22/22)
  - `ctest -L smoke --parallel 1`: PASS (6/6)
  - `compare_sz_versions.py`（10 trials）: PASS

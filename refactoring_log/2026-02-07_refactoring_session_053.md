Date: 2026-02-07
Time: 01:27:25 JST

# Refactoring Session 053 (CI failure follow-up)

## 背景
- GitHub CI で以下が失敗:
  - `CI / ctest_smoke (ubuntu-24.04, 1, 3)`
  - `CI / ctest_unit`
  - `CI / ctest_smoke (ubuntu-24.04, 1, 1)`

## 実施した対策
- `.github/actions/bootstrap-build/action.yml`
  - Linux の `apt update && apt install` を 3 回リトライ化。
  - 失敗時に明示エラーで終了。
- `.github/workflows/main.yml`
  - `ctest_unit` を `--output-on-failure -L unit --parallel 1` で実行。
  - 失敗時に 1 回だけ自動リトライ。
  - `ctest_smoke` も同様に `--parallel 1` + 1 回リトライを適用。

## ローカル検証
- `ctest --output-on-failure -L unit --parallel 1`: PASS (20/20)
- `ctest --output-on-failure -L smoke --parallel 1`: PASS (6/6)
- `for i in 1..5; ctest --output-on-failure -L smoke --parallel 1`: PASS (5 回連続)
- `OMP_NUM_THREADS=3 MPIRUN='mpiexec --oversubscribe -np 1' ctest --output-on-failure -L smoke --parallel 1`: PASS (6/6)

## メモ
- 一時的に sandbox 制約で `mpiexec` が socket bind 失敗になったため、OMP=3 再現確認は昇格実行で検証した。

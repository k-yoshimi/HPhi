Date: 2026-02-08
Time: 12:50:44 JST

# Refactoring Session 058 (sz-side test-first + equivalence checks)

## 目的
- `readdef/InterAll` 側と同様に、`sz` 系リファクタ領域にも
  先行テストと旧版/新版比較のチェックを追加する。

## 実施内容
1. 先行 unit test 追加（sz 等価性）
- 新規: `test/unit/sz_hubbard_calchs_equivalence.sh`
  - `FermionHubbard` 固定ケースで `CalcHS=0` と `CalcHS=1` を比較。
  - 比較項目:
    - `output/zvo_energy.dat` の `Energy`, `Doublon`, `Sz`
    - `output/CHECK_Memory.dat` の `idim_max`
  - `initial_iv=1` 固定で再現性を担保。

- `test/unit/CMakeLists.txt`
  - `unit_sz_hubbard_calchs_equivalence` を追加（label: `unit`）。

2. 旧版/新版差分比較ハーネス追加（sz）
- 新規: `test/tools/compare_sz_versions.py`
  - baseline/candidate バイナリで同一ランダム入力を実行し、
    `CHECK_Sdim.dat` の `sdim` と `CHECK_Memory.dat` の `idim_max` を比較。
  - 失敗時は trial artifact を保持し、再現調査可能。
  - ランダム入力は `FermionHubbard`/`Spin` の標準入力を生成。
    - `FermionHubbard` は `2Sz` を付与する場合、`Nup/Ndown` が有効範囲になる候補のみ採用。

3. 比較ハーネス運用改善
- `test/tools/compare_readdef_interall_versions.py`
  - 失敗時にも artifact が消えないよう cleanup 条件を修正。

4. ドキュメント更新
- `doc/test_operations.md`
  - `compare_sz_versions.py` の実行手順を追記。

## 検証結果
- `cmake -S . -B build`: PASS
- `ctest --output-on-failure -L unit --parallel 1 --test-dir build`: PASS (22/22)
- `ctest --output-on-failure -L smoke --parallel 1 --test-dir build`: PASS (6/6)
- `python3 test/tools/compare_sz_versions.py --baseline build/src/HPhi --candidate build/src/HPhi --trials 10 --seed 42`: PASS

## 補足
- 今回は `sz` の保護テスト・比較基盤を優先し、コア計算ロジックの挙動変更は実施していない。

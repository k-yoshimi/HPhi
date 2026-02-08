Date: 2026-02-08
Time: 11:17:10 JST

# Refactoring Session 057 (test-first hardening for readdef InterAll path)

## 実施方針
- コメント対応方針（test-first + 旧版/新版差分比較）に沿って、
  今回リファクタ対象だった `readdef/InterAll` 経路の検証を先行強化した。

## 実施内容
1. 先行テスト追加（characterization）
- 新規: `test/unit/readdef_interall_equivalence.sh`
  - `FermionHubbard` の expert-mode 入力で、`InterAll` の等価な2表現を比較。
    - case A: canonical off-diagonal表現
    - case B: exchange表現（正規化・符号反転が必要）
  - `GetDiagonalInterAll(..., INTERALL_SPLIT_SIMPLE)` +
    `CheckInterAllHermite_simple` + `ArrangeInterAllOffDiagonal`
    の経路が同一物理量（`Energy`, `Doublon`）を返すことを固定化。
- `test/unit/CMakeLists.txt` に `unit_readdef_interall_equivalence` を登録。

2. 旧版/新版差分比較ハーネス追加
- 新規: `test/tools/compare_readdef_interall_versions.py`
  - baseline/candidate の2バイナリを受け取り、
    ランダム生成 `InterAll` fixture で挙動差分を比較。
  - 比較対象: `output/zvo_energy.dat` の `Energy`, `Doublon`, `Sz`
  - seed固定・trial数指定・許容誤差指定をサポート。
  - 差分時は作業ディレクトリを保持して再現調査可能。

3. 運用ドキュメント更新
- `doc/test_operations.md`
  - test-first ガードレールを追加。
  - 旧版/新版差分比較コマンド例を追記。

## 検証結果
- `cmake --build build -j4`: PASS
- `ctest --output-on-failure -L unit --parallel 1 --test-dir build`: PASS (21/21)
- `python3 test/tools/compare_readdef_interall_versions.py --baseline build/src/HPhi --candidate build/src/HPhi --trials 3 --seed 42 --tol 1e-12`: PASS

## 補足
- 今回は「先行テスト + 比較基盤」の追加に集中し、既存計算ロジックの挙動変更は入れていない。

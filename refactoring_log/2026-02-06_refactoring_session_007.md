Date: 2026-02-06
Time: 21:45:19 JST

# Refactoring Session 007 (REF-011)

## 実施内容
- `test/unit/diagonalcalc_characterization.sh` を新規追加。
- `test/unit/CMakeLists.txt` に `unit_diagonalcalc_characterization` を追加（`unit` ラベル）。
- 小規模 `FullDiag` ケース (`FermionHubbard`, `L=4`) で以下を固定:
  - `output/zvo_phys_Nup2_Ndown2.dat` が生成されること。
  - 行数が `37`（ヘッダ1行 + 状態36行）であること。
  - 先頭3固有値が既知値（`-2.102748`, `-1.806424`, `-1.068140`）と許容誤差 `1e-6` 以内で一致すること。

## 検証結果
- `ctest --output-on-failure -L unit` : PASS (5/5)
- `ctest --output-on-failure -L smoke -j4` : PASS (6/6)
- `ctest --output-on-failure -j4` : PASS (65/65)
